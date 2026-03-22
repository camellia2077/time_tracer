pub mod export;
pub mod render;
pub mod support;

use serde_json::Value;

use crate::cli::{ReportArgs, ReportCommand};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::core::runtime::{CliConfig, CoreApi};
use crate::error::AppError;

use self::export::ExportHandler;
use self::render::RenderHandler;

pub struct ReportHandler;

pub(crate) trait ReportSessionPort {
    fn load_cli_config(
        &self,
        command_name: &str,
        ctx: &CommandContext,
    ) -> Result<CliConfig, AppError>;
    fn render(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<String, AppError>;
    fn export(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<(), AppError>;
}

pub(crate) struct RuntimeReportSessionPort;

impl ReportSessionPort for RuntimeReportSessionPort {
    fn load_cli_config(
        &self,
        command_name: &str,
        ctx: &CommandContext,
    ) -> Result<CliConfig, AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, ctx)?;
        Ok(session.cli_config().clone())
    }

    fn render(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<String, AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, ctx)?;
        session.report().render(request)
    }

    fn export(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<(), AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, ctx)?;
        session.report().export(request)
    }
}

impl CommandHandler<ReportArgs> for ReportHandler {
    fn handle(&self, args: ReportArgs, ctx: &CommandContext) -> Result<(), AppError> {
        match args.command {
            ReportCommand::Render(args) => RenderHandler.handle(args, ctx),
            ReportCommand::Export(args) => ExportHandler.handle(args, ctx),
        }
    }
}

#[cfg(test)]
mod tests {
    use serde_json::Value;

    use crate::cli::{
        ReportExportArgs, ReportExportPeriod, ReportFormat, ReportRenderArgs, ReportRenderPeriod,
    };
    use crate::commands::testing::{RecordedReportSession, default_context, sample_cli_config};

    use super::ReportSessionPort;
    use super::export::run_export_with_port;
    use super::render::run_render_with_port;

    struct TestReportPort<'a> {
        recorded: &'a RecordedReportSession,
    }

    impl ReportSessionPort for TestReportPort<'_> {
        fn load_cli_config(
            &self,
            _command_name: &str,
            _ctx: &crate::commands::handler::CommandContext,
        ) -> Result<crate::core::runtime::CliConfig, crate::error::AppError> {
            Ok(self.recorded.load_cli_config())
        }

        fn render(
            &self,
            command_name: &str,
            _ctx: &crate::commands::handler::CommandContext,
            request: &Value,
        ) -> Result<String, crate::error::AppError> {
            self.recorded.record_render(command_name, request)
        }

        fn export(
            &self,
            command_name: &str,
            _ctx: &crate::commands::handler::CommandContext,
            request: &Value,
        ) -> Result<(), crate::error::AppError> {
            self.recorded.record_export(command_name, request)
        }
    }

    #[test]
    fn report_render_recent_request_uses_query_bootstrap_token() {
        let recorded = RecordedReportSession::new(sample_cli_config(), "ok");
        let port = TestReportPort {
            recorded: &recorded,
        };

        run_render_with_port(
            ReportRenderArgs {
                period: ReportRenderPeriod::Recent,
                argument: "7,10".to_string(),
                format: vec![ReportFormat::Md],
            },
            &default_context(),
            &port,
        )
        .expect("report render recent should succeed");

        assert_eq!(recorded.command_names(), vec!["query".to_string()]);
        let request = recorded.requests().remove(0);
        assert_eq!(request["days_list"], serde_json::json!([7, 10]));
        assert_eq!(request["format"], "md");
    }

    #[test]
    fn report_render_day_request_uses_period_and_argument() {
        let recorded = RecordedReportSession::new(sample_cli_config(), "ok");
        let port = TestReportPort {
            recorded: &recorded,
        };

        run_render_with_port(
            ReportRenderArgs {
                period: ReportRenderPeriod::Day,
                argument: "20260103".to_string(),
                format: vec![ReportFormat::Tex],
            },
            &default_context(),
            &port,
        )
        .expect("report render day should succeed");

        let request = recorded.requests().remove(0);
        assert_eq!(request["type"], "day");
        assert_eq!(request["argument"], "20260103");
        assert_eq!(request["format"], "tex");
    }

    #[test]
    fn report_render_range_request_uses_period_and_argument() {
        let recorded = RecordedReportSession::new(sample_cli_config(), "ok");
        let port = TestReportPort {
            recorded: &recorded,
        };

        run_render_with_port(
            ReportRenderArgs {
                period: ReportRenderPeriod::Range,
                argument: "20260101|20260131".to_string(),
                format: vec![ReportFormat::Typ],
            },
            &default_context(),
            &port,
        )
        .expect("report render range should succeed");

        let request = recorded.requests().remove(0);
        assert_eq!(request["type"], "range");
        assert_eq!(request["argument"], "20260101|20260131");
        assert_eq!(request["format"], "typ");
    }

    #[test]
    fn report_export_all_maps_to_legacy_runtime_shape_with_export_token() {
        let recorded = RecordedReportSession::new(sample_cli_config(), "ok");
        let port = TestReportPort {
            recorded: &recorded,
        };

        run_export_with_port(
            ReportExportArgs {
                period: ReportExportPeriod::Day,
                argument: None,
                all: true,
                format: vec![ReportFormat::Md],
            },
            &default_context(),
            &port,
        )
        .expect("report export all should succeed");

        assert_eq!(recorded.command_names(), vec!["export".to_string()]);
        let request = recorded.requests().remove(0);
        assert_eq!(request["type"], "all-day");
        assert_eq!(request["format"], "md");
    }
}
