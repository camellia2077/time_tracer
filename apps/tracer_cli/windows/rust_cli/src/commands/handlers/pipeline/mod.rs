pub mod convert;
pub mod import;
pub mod ingest;
pub mod support;
pub mod validate;

use serde_json::Value;

use crate::cli::{PipelineArgs, PipelineCommand};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::core::runtime::{CliConfig, CoreApi};
use crate::error::AppError;

use self::convert::ConvertHandler;
use self::import::ImportHandler;
use self::ingest::IngestHandler;
use self::validate::ValidateHandler;

pub struct PipelineHandler;

pub(crate) trait PipelineSessionPort {
    fn load_cli_config(
        &self,
        command_name: &str,
        ctx: &CommandContext,
    ) -> Result<CliConfig, AppError>;
    fn convert(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<(), AppError>;
    fn import_processed(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<(), AppError>;
    fn ingest(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<(), AppError>;
    fn validate_structure(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<(), AppError>;
    fn validate_logic(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<(), AppError>;
}

pub(crate) struct RuntimePipelineSessionPort;

impl PipelineSessionPort for RuntimePipelineSessionPort {
    fn load_cli_config(
        &self,
        command_name: &str,
        ctx: &CommandContext,
    ) -> Result<CliConfig, AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, ctx)?;
        Ok(session.cli_config().clone())
    }

    fn convert(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<(), AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, ctx)?;
        session.pipeline().convert(request)
    }

    fn import_processed(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<(), AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, ctx)?;
        session.pipeline().import_processed(request)
    }

    fn ingest(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<(), AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, ctx)?;
        session.pipeline().ingest(request)
    }

    fn validate_structure(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<(), AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, ctx)?;
        session.pipeline().validate_structure(request)
    }

    fn validate_logic(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<(), AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, ctx)?;
        session.pipeline().validate_logic(request)
    }
}

impl CommandHandler<PipelineArgs> for PipelineHandler {
    fn handle(&self, args: PipelineArgs, ctx: &CommandContext) -> Result<(), AppError> {
        match args.command {
            PipelineCommand::Convert(args) => ConvertHandler.handle(args, ctx),
            PipelineCommand::Import(args) => ImportHandler.handle(args, ctx),
            PipelineCommand::Ingest(args) => IngestHandler.handle(args, ctx),
            PipelineCommand::Validate(args) => ValidateHandler.handle(args, ctx),
        }
    }
}

#[cfg(test)]
mod tests {
    use serde_json::Value;

    use crate::cli::{PipelineConvertArgs, PipelineIngestArgs, PipelineValidateAllArgs};
    use crate::commands::testing::{RecordedPipelineSession, default_context, sample_cli_config};

    use super::PipelineSessionPort;
    use super::convert::run_convert_with_port;
    use super::ingest::run_ingest_with_port;
    use super::validate::run_all_with_port;

    struct TestPipelinePort<'a> {
        recorded: &'a RecordedPipelineSession,
    }

    impl PipelineSessionPort for TestPipelinePort<'_> {
        fn load_cli_config(
            &self,
            _command_name: &str,
            _ctx: &crate::commands::handler::CommandContext,
        ) -> Result<crate::core::runtime::CliConfig, crate::error::AppError> {
            Ok(self.recorded.load_cli_config())
        }

        fn convert(
            &self,
            command_name: &str,
            _ctx: &crate::commands::handler::CommandContext,
            request: &Value,
        ) -> Result<(), crate::error::AppError> {
            self.recorded.record_ack(command_name, request)
        }

        fn import_processed(
            &self,
            command_name: &str,
            _ctx: &crate::commands::handler::CommandContext,
            request: &Value,
        ) -> Result<(), crate::error::AppError> {
            self.recorded.record_ack(command_name, request)
        }

        fn ingest(
            &self,
            command_name: &str,
            _ctx: &crate::commands::handler::CommandContext,
            request: &Value,
        ) -> Result<(), crate::error::AppError> {
            self.recorded.record_ack(command_name, request)
        }

        fn validate_structure(
            &self,
            command_name: &str,
            _ctx: &crate::commands::handler::CommandContext,
            request: &Value,
        ) -> Result<(), crate::error::AppError> {
            self.recorded.record_ack(command_name, request)
        }

        fn validate_logic(
            &self,
            command_name: &str,
            _ctx: &crate::commands::handler::CommandContext,
            request: &Value,
        ) -> Result<(), crate::error::AppError> {
            self.recorded.record_ack(command_name, request)
        }
    }

    #[test]
    fn pipeline_convert_uses_convert_bootstrap_token() {
        let recorded = RecordedPipelineSession::new(sample_cli_config());
        let port = TestPipelinePort {
            recorded: &recorded,
        };

        run_convert_with_port(
            PipelineConvertArgs {
                path: "input.txt".to_string(),
            },
            &default_context(),
            &port,
        )
        .expect("convert should succeed");

        assert_eq!(recorded.command_names(), vec!["convert".to_string()]);
        let request = recorded.requests().remove(0);
        assert_eq!(request["input_path"], "input.txt");
        assert_eq!(request["date_check_mode"], "continuity");
        assert_eq!(request["save_processed_output"], true);
        assert_eq!(request["validate_logic"], true);
        assert_eq!(request["validate_structure"], true);
    }

    #[test]
    fn pipeline_ingest_uses_defaults_in_request() {
        let recorded = RecordedPipelineSession::new(sample_cli_config());
        let port = TestPipelinePort {
            recorded: &recorded,
        };

        run_ingest_with_port(
            PipelineIngestArgs {
                path: "source_dir".to_string(),
                date_check: None,
                no_date_check: false,
                save_processed: false,
                no_save_processed: false,
            },
            &default_context(),
            &port,
        )
        .expect("ingest should succeed");

        assert_eq!(recorded.command_names(), vec!["ingest".to_string()]);
        let request = recorded.requests().remove(0);
        assert_eq!(request["input_path"], "source_dir");
        assert_eq!(request["date_check_mode"], "full");
        assert_eq!(request["save_processed_output"], false);
    }

    #[test]
    fn pipeline_validate_all_short_circuits_after_structure_failure() {
        let recorded =
            RecordedPipelineSession::new(sample_cli_config()).with_failure_on("validate-structure");
        let port = TestPipelinePort {
            recorded: &recorded,
        };

        let error = run_all_with_port(
            PipelineValidateAllArgs {
                path: "input.txt".to_string(),
                date_check: None,
                no_date_check: false,
            },
            &default_context(),
            &port,
        )
        .expect_err("structure failure should stop validate all");

        assert!(error.render_for_stderr().contains("forced failure"));
        assert_eq!(
            recorded.command_names(),
            vec!["validate-structure".to_string()]
        );
        assert_eq!(recorded.requests().len(), 1);
    }
}
