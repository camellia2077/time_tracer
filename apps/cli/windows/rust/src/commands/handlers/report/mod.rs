pub mod export;
pub mod render;
pub mod support;

use std::path::Path;

use serde_json::Value;

use crate::cli::{ReportArgs, ReportCommand};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::core::runtime::{CliConfig, CoreApi, RuntimeSession};
use crate::error::AppError;

use self::export::ExportHandler;
use self::render::RenderHandler;

pub struct ReportHandler;

pub(crate) trait ReportSession {
    fn cli_config(&self) -> &CliConfig;
    fn runtime_output_root(&self) -> &Path;
    fn render(&self, request: &Value) -> Result<String, AppError>;
    fn list_targets(&self, target_type: &str) -> Result<Vec<String>, AppError>;
}

pub(crate) trait ReportSessionPort {
    fn open(
        &self,
        command_name: &str,
        ctx: &CommandContext,
    ) -> Result<Box<dyn ReportSession>, AppError>;
}

pub(crate) struct RuntimeReportSessionPort;

struct RuntimeBoundReportSession {
    session: RuntimeSession,
}

impl ReportSession for RuntimeBoundReportSession {
    fn cli_config(&self) -> &CliConfig {
        self.session.cli_config()
    }

    fn runtime_output_root(&self) -> &Path {
        Path::new(&self.session.paths().runtime_output_root)
    }

    fn render(&self, request: &Value) -> Result<String, AppError> {
        self.session.report().render(request)
    }

    fn list_targets(&self, target_type: &str) -> Result<Vec<String>, AppError> {
        self.session.report().list_targets(target_type)
    }
}

impl ReportSessionPort for RuntimeReportSessionPort {
    fn open(
        &self,
        command_name: &str,
        ctx: &CommandContext,
    ) -> Result<Box<dyn ReportSession>, AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, ctx)?;
        Ok(Box::new(RuntimeBoundReportSession { session }))
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
    use std::fs;
    use std::path::{Path, PathBuf};
    use std::rc::Rc;

    use serde_json::Value;

    use crate::cli::{
        ReportExportArgs, ReportExportPeriod, ReportFormat, ReportRenderArgs, ReportRenderPeriod,
    };
    use crate::commands::testing::{
        RecordedReportSession, default_context, sample_cli_config, temp_output_path,
    };
    use crate::core::runtime::CliConfig;

    use super::{ReportSession, ReportSessionPort};
    use super::export::run_export_with_port;
    use super::render::run_render_with_port;

    struct TestReportPort {
        recorded: Rc<RecordedReportSession>,
    }

    struct TestReportSession {
        recorded: Rc<RecordedReportSession>,
        command_name: String,
        cli_config: CliConfig,
        runtime_output_root: PathBuf,
    }

    impl ReportSession for TestReportSession {
        fn cli_config(&self) -> &CliConfig {
            &self.cli_config
        }

        fn runtime_output_root(&self) -> &Path {
            &self.runtime_output_root
        }

        fn render(&self, request: &Value) -> Result<String, crate::error::AppError> {
            self.recorded.record_render(&self.command_name, request)
        }

        fn list_targets(&self, target_type: &str) -> Result<Vec<String>, crate::error::AppError> {
            self.recorded
                .record_list_targets(&self.command_name, target_type)
        }
    }

    impl ReportSessionPort for TestReportPort {
        fn open(
            &self,
            command_name: &str,
            _ctx: &crate::commands::handler::CommandContext,
        ) -> Result<Box<dyn ReportSession>, crate::error::AppError> {
            Ok(Box::new(TestReportSession {
                recorded: Rc::clone(&self.recorded),
                command_name: command_name.to_string(),
                cli_config: self.recorded.load_cli_config(),
                runtime_output_root: PathBuf::from(self.recorded.runtime_output_root()),
            }))
        }
    }

    #[test]
    fn report_render_recent_request_uses_query_bootstrap_token() {
        let recorded = Rc::new(RecordedReportSession::new(sample_cli_config(), "ok"));
        let port = TestReportPort {
            recorded: Rc::clone(&recorded),
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
        let recorded = Rc::new(RecordedReportSession::new(sample_cli_config(), "ok"));
        let port = TestReportPort {
            recorded: Rc::clone(&recorded),
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
        let recorded = Rc::new(RecordedReportSession::new(sample_cli_config(), "ok"));
        let port = TestReportPort {
            recorded: Rc::clone(&recorded),
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
    fn report_export_month_compact_input_writes_dashed_filename() {
        let export_root = temp_output_path("report_export_month_compact", "root");
        let recorded = Rc::new(
            RecordedReportSession::new(sample_cli_config(), "# month\n")
                .with_runtime_output_root(export_root.to_string_lossy().to_string()),
        );
        let port = TestReportPort {
            recorded: Rc::clone(&recorded),
        };

        run_export_with_port(
            ReportExportArgs {
                period: ReportExportPeriod::Month,
                argument: Some("202603".to_string()),
                all: false,
                format: vec![ReportFormat::Md],
            },
            &default_context(),
            &port,
        )
        .expect("month export should succeed");

        assert_eq!(recorded.command_names(), vec!["export".to_string()]);
        let request = recorded.requests().remove(0);
        assert_eq!(request["type"], "month");
        assert_eq!(request["argument"], "202603");
        assert!(export_root.join("markdown").join("month").join("2026-03.md").exists());
        let _ = fs::remove_dir_all(&export_root);
    }

    #[test]
    fn report_export_all_month_uses_canonical_dashed_target_names() {
        let export_root = temp_output_path("report_export_all_month", "root");
        let recorded = Rc::new(
            RecordedReportSession::new(sample_cli_config(), "# month\n")
                .with_runtime_output_root(export_root.to_string_lossy().to_string())
                .with_targets("month", vec!["2026-03", "2026-04"]),
        );
        let port = TestReportPort {
            recorded: Rc::clone(&recorded),
        };

        run_export_with_port(
            ReportExportArgs {
                period: ReportExportPeriod::Month,
                argument: None,
                all: true,
                format: vec![ReportFormat::Md],
            },
            &default_context(),
            &port,
        )
        .expect("month export all should succeed");

        assert!(export_root.join("markdown").join("month").join("2026-03.md").exists());
        assert!(export_root.join("markdown").join("month").join("2026-04.md").exists());
        let _ = fs::remove_dir_all(&export_root);
    }

    #[test]
    fn report_export_day_and_recent_keep_existing_path_templates() {
        let export_root = temp_output_path("report_export_day_recent", "root");
        let recorded = Rc::new(
            RecordedReportSession::new(sample_cli_config(), "content\n")
                .with_runtime_output_root(export_root.to_string_lossy().to_string()),
        );
        let port = TestReportPort {
            recorded: Rc::clone(&recorded),
        };

        run_export_with_port(
            ReportExportArgs {
                period: ReportExportPeriod::Day,
                argument: Some("20260103".to_string()),
                all: false,
                format: vec![ReportFormat::Md],
            },
            &default_context(),
            &port,
        )
        .expect("day export should succeed");
        run_export_with_port(
            ReportExportArgs {
                period: ReportExportPeriod::Recent,
                argument: Some("7".to_string()),
                all: false,
                format: vec![ReportFormat::Md],
            },
            &default_context(),
            &port,
        )
        .expect("recent export should succeed");

        assert!(
            export_root
                .join("markdown")
                .join("day")
                .join("2026")
                .join("01")
                .join("2026-01-03.md")
                .exists()
        );
        assert!(
            export_root
                .join("markdown")
                .join("recent")
                .join("last_7_days_report.md")
                .exists()
        );
        let _ = fs::remove_dir_all(&export_root);
    }

    #[test]
    fn report_export_output_override_stays_directory_root_even_if_it_looks_like_file() {
        let export_root = temp_output_path("report_export_file_like_root", "md");
        let recorded = Rc::new(
            RecordedReportSession::new(sample_cli_config(), "content\n")
                .with_runtime_output_root(export_root.to_string_lossy().to_string()),
        );
        let port = TestReportPort {
            recorded: Rc::clone(&recorded),
        };

        run_export_with_port(
            ReportExportArgs {
                period: ReportExportPeriod::Month,
                argument: Some("2026-03".to_string()),
                all: false,
                format: vec![ReportFormat::Md],
            },
            &default_context(),
            &port,
        )
        .expect("month export with file-like root should succeed");

        assert!(
            export_root
                .join("markdown")
                .join("month")
                .join("2026-03.md")
                .exists()
        );
        let _ = fs::remove_dir_all(&export_root);
    }
}
