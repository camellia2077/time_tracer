pub mod export;
pub mod import;
pub mod inspect;
pub mod support;

use serde_json::Value;

use crate::cli::{ExchangeArgs, ExchangeCommand};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::core::runtime::{CliConfig, CoreApi};
use crate::error::AppError;

use self::export::ExportHandler;
use self::import::ImportHandler;
use self::inspect::InspectHandler;

pub struct ExchangeHandler;

pub(crate) trait ExchangeSessionPort {
    fn load_cli_config(
        &self,
        command_name: &str,
        ctx: &CommandContext,
    ) -> Result<CliConfig, AppError>;
    fn export_package(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<String, AppError>;
    fn import_package(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<String, AppError>;
    fn inspect_package(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<String, AppError>;
}

pub(crate) trait ExchangePromptPort {
    fn prompt_export_passphrase(&self) -> Result<String, AppError>;
    fn prompt_package_passphrase(&self) -> Result<String, AppError>;
}

pub(crate) struct RuntimeExchangeSessionPort;

impl ExchangeSessionPort for RuntimeExchangeSessionPort {
    fn load_cli_config(
        &self,
        command_name: &str,
        ctx: &CommandContext,
    ) -> Result<CliConfig, AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, &ctx.without_output())?;
        Ok(session.cli_config().clone())
    }

    fn export_package(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<String, AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, &ctx.without_output())?;
        session.exchange().export_package(request)
    }

    fn import_package(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<String, AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, &ctx.without_output())?;
        session.exchange().import_package(request)
    }

    fn inspect_package(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<String, AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, &ctx.without_output())?;
        session.exchange().inspect_package(request)
    }
}

pub(crate) struct InteractiveExchangePromptPort;

impl ExchangePromptPort for InteractiveExchangePromptPort {
    fn prompt_export_passphrase(&self) -> Result<String, AppError> {
        support::prompt_passphrase_for_export()
    }

    fn prompt_package_passphrase(&self) -> Result<String, AppError> {
        support::prompt_passphrase_for_package()
    }
}

struct ExchangeProgressLineGuard;

impl Drop for ExchangeProgressLineGuard {
    fn drop(&mut self) {
        crate::core::runtime::finalize_tracer_exchange_progress_line();
    }
}

impl CommandHandler<ExchangeArgs> for ExchangeHandler {
    fn handle(&self, args: ExchangeArgs, ctx: &CommandContext) -> Result<(), AppError> {
        let _progress_guard = ExchangeProgressLineGuard;
        match args.command {
            ExchangeCommand::Export(args) => ExportHandler.handle(args, ctx),
            ExchangeCommand::Import(args) => ImportHandler.handle(args, ctx),
            ExchangeCommand::Inspect(args) => InspectHandler.handle(args, ctx),
        }
    }
}

#[cfg(test)]
mod tests {
    use serde_json::Value;

    use crate::cli::{ExchangeExportArgs, ExchangeImportArgs, ExchangeInspectArgs, SecurityLevel};
    use crate::commands::testing::{
        RecordedExchangeSession, default_context, sample_cli_config, temp_output_path,
    };

    use super::export::run_export_with_port;
    use super::import::run_import_with_port;
    use super::inspect::run_inspect_with_port;
    use super::{ExchangePromptPort, ExchangeSessionPort};

    struct TestExchangePort<'a> {
        recorded: &'a RecordedExchangeSession,
    }

    impl ExchangeSessionPort for TestExchangePort<'_> {
        fn load_cli_config(
            &self,
            _command_name: &str,
            _ctx: &crate::commands::handler::CommandContext,
        ) -> Result<crate::core::runtime::CliConfig, crate::error::AppError> {
            Ok(self.recorded.load_cli_config())
        }

        fn export_package(
            &self,
            command_name: &str,
            _ctx: &crate::commands::handler::CommandContext,
            request: &Value,
        ) -> Result<String, crate::error::AppError> {
            self.recorded.record_text(command_name, request)
        }

        fn import_package(
            &self,
            command_name: &str,
            _ctx: &crate::commands::handler::CommandContext,
            request: &Value,
        ) -> Result<String, crate::error::AppError> {
            self.recorded.record_text(command_name, request)
        }

        fn inspect_package(
            &self,
            command_name: &str,
            _ctx: &crate::commands::handler::CommandContext,
            request: &Value,
        ) -> Result<String, crate::error::AppError> {
            self.recorded.record_text(command_name, request)
        }
    }

    struct TestPromptPort;

    impl ExchangePromptPort for TestPromptPort {
        fn prompt_export_passphrase(&self) -> Result<String, crate::error::AppError> {
            Ok("secret-export".to_string())
        }

        fn prompt_package_passphrase(&self) -> Result<String, crate::error::AppError> {
            Ok("secret-import".to_string())
        }
    }

    #[test]
    fn exchange_export_still_requires_output_before_session_use() {
        let recorded = RecordedExchangeSession::new(sample_cli_config(), "ok");
        let port = TestExchangePort {
            recorded: &recorded,
        };
        let error = run_export_with_port(
            ExchangeExportArgs {
                input: ".".to_string(),
                security_level: None,
            },
            &default_context(),
            &port,
            &TestPromptPort,
        )
        .expect_err("exchange export should require output");

        assert!(
            error
                .render_for_stderr()
                .contains("-o/--output is required for exchange export")
        );
        assert!(recorded.requests().is_empty());
    }

    #[test]
    fn exchange_export_uses_crypto_bootstrap_token_and_request_shape() {
        let recorded = RecordedExchangeSession::new(sample_cli_config(), "ok");
        let port = TestExchangePort {
            recorded: &recorded,
        };
        let output_path = temp_output_path("exchange_export", "tracer");
        let mut ctx = default_context();
        ctx.output_path = Some(output_path.to_string_lossy().to_string());

        run_export_with_port(
            ExchangeExportArgs {
                input: ".".to_string(),
                security_level: Some(SecurityLevel::High),
            },
            &ctx,
            &port,
            &TestPromptPort,
        )
        .expect("exchange export should succeed");

        assert_eq!(recorded.command_names(), vec!["crypto".to_string()]);
        let request = recorded.requests().remove(0);
        assert_eq!(request["passphrase"], "secret-export");
        assert_eq!(request["security_level"], "high");
        assert_eq!(request["date_check_mode"], "none");
        assert!(
            request["output_path"]
                .as_str()
                .unwrap_or_default()
                .ends_with(".tracer")
        );
    }

    #[test]
    fn exchange_import_uses_crypto_bootstrap_token() {
        let recorded = RecordedExchangeSession::new(sample_cli_config(), "ok");
        let port = TestExchangePort {
            recorded: &recorded,
        };

        run_import_with_port(
            ExchangeImportArgs {
                input: "Cargo.toml".to_string(),
            },
            &default_context(),
            &port,
            &TestPromptPort,
        )
        .expect("exchange import should succeed");

        assert_eq!(recorded.command_names(), vec!["crypto".to_string()]);
        let request = recorded.requests().remove(0);
        assert_eq!(request["passphrase"], "secret-import");
        assert!(
            request["input_path"]
                .as_str()
                .unwrap_or_default()
                .contains("Cargo.toml")
        );
    }

    #[test]
    fn exchange_inspect_rejects_output() {
        let recorded = RecordedExchangeSession::new(sample_cli_config(), "ok");
        let port = TestExchangePort {
            recorded: &recorded,
        };
        let mut ctx = default_context();
        ctx.output_path = Some("inspect-output".to_string());

        let error = run_inspect_with_port(
            ExchangeInspectArgs {
                input: "Cargo.toml".to_string(),
            },
            &ctx,
            &port,
            &TestPromptPort,
        )
        .expect_err("inspect should reject output");

        assert!(
            error
                .render_for_stderr()
                .contains("-o/--output is not valid for exchange inspect")
        );
        assert!(recorded.requests().is_empty());
    }

    #[test]
    fn exchange_inspect_uses_crypto_bootstrap_token() {
        let recorded = RecordedExchangeSession::new(sample_cli_config(), "ok");
        let port = TestExchangePort {
            recorded: &recorded,
        };

        run_inspect_with_port(
            ExchangeInspectArgs {
                input: "Cargo.toml".to_string(),
            },
            &default_context(),
            &port,
            &TestPromptPort,
        )
        .expect("inspect should succeed");

        assert_eq!(recorded.command_names(), vec!["crypto".to_string()]);
        let request = recorded.requests().remove(0);
        assert_eq!(request["passphrase"], "secret-import");
        assert!(
            request["input_path"]
                .as_str()
                .unwrap_or_default()
                .contains("Cargo.toml")
        );
    }
}
