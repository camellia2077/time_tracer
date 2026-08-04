use serde_json::Value;

use crate::cli::{
    ExchangeExportArgs, ExchangeImportArgs, ExchangeInspectArgs, ExchangeUnpackArgs, SecurityLevel,
};
use crate::commands::testing::{
    default_context, sample_cli_config, temp_output_path, RecordedExchangeSession,
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

    fn unpack_package(
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
            date_check: None,
            no_date_check: false,
        },
        &default_context(),
        &port,
        &TestPromptPort,
    )
    .expect_err("exchange export should require output");

    assert!(error
        .render_for_stderr()
        .contains("-o/--output is required for exchange export"));
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
            date_check: None,
            no_date_check: false,
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
    assert!(request["output_path"]
        .as_str()
        .unwrap_or_default()
        .ends_with(".tracer"));
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
    assert!(request["input_path"]
        .as_str()
        .unwrap_or_default()
        .contains("Cargo.toml"));
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

    assert!(error
        .render_for_stderr()
        .contains("-o/--output is not valid for exchange inspect"));
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
    assert!(request["input_path"]
        .as_str()
        .unwrap_or_default()
        .contains("Cargo.toml"));
}

#[test]
fn exchange_unpack_uses_crypto_bootstrap_token_and_requires_output() {
    let recorded = RecordedExchangeSession::new(sample_cli_config(), "ok");
    let port = TestExchangePort {
        recorded: &recorded,
    };
    let output_path = temp_output_path("exchange_unpack", "dir");
    let mut ctx = default_context();
    ctx.output_path = Some(output_path.to_string_lossy().to_string());

    crate::commands::handlers::exchange::unpack::run_unpack_with_port(
        ExchangeUnpackArgs {
            input: "Cargo.toml".to_string(),
        },
        &ctx,
        &port,
        &TestPromptPort,
    )
    .expect("exchange unpack should succeed");

    assert_eq!(recorded.command_names(), vec!["crypto".to_string()]);
    let request = recorded.requests().remove(0);
    assert_eq!(request["passphrase"], "secret-import");
    assert!(request["output_path"]
        .as_str()
        .unwrap_or_default()
        .contains("exchange_unpack"));
}
