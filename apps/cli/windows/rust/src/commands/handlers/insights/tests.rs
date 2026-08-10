use std::fs;
use std::path::{Path, PathBuf};
use std::rc::Rc;

use serde_json::Value;

use crate::cli::{
    InsightsExportArgs, InsightsExportPeriod, InsightsFormat, InsightsRenderArgs, InsightsRenderPeriod,
};
use crate::commands::testing::{
    default_context, sample_cli_config, temp_output_path, RecordedInsightsSession,
};
use crate::core::runtime::CliConfig;

use super::export::run_export_with_port;
use super::render::run_render_with_port;
use super::{RenderedInsights, InsightsSession, InsightsSessionPort, InsightsWindowMetadata};

struct TestInsightsPort {
    recorded: Rc<RecordedInsightsSession>,
}

struct TestInsightsSession {
    recorded: Rc<RecordedInsightsSession>,
    command_name: String,
    cli_config: CliConfig,
    runtime_output_root: PathBuf,
}

impl InsightsSession for TestInsightsSession {
    fn cli_config(&self) -> &CliConfig {
        &self.cli_config
    }

    fn runtime_output_root(&self) -> &Path {
        &self.runtime_output_root
    }

    fn render(&self, request: &Value) -> Result<RenderedInsights, crate::error::AppError> {
        self.recorded.record_render(&self.command_name, request)
    }

    fn list_targets(&self, target_type: &str) -> Result<Vec<String>, crate::error::AppError> {
        self.recorded
            .record_list_targets(&self.command_name, target_type)
    }

    fn export(&self, request: &Value) -> Result<(), crate::error::AppError> {
        self.recorded.record_export(&self.command_name, request)
    }
}

impl InsightsSessionPort for TestInsightsPort {
    fn open(
        &self,
        command_name: &str,
        _ctx: &crate::commands::handler::CommandContext,
    ) -> Result<Box<dyn InsightsSession>, crate::error::AppError> {
        Ok(Box::new(TestInsightsSession {
            recorded: Rc::clone(&self.recorded),
            command_name: command_name.to_string(),
            cli_config: self.recorded.load_cli_config(),
            runtime_output_root: PathBuf::from(self.recorded.runtime_output_root()),
        }))
    }
}

#[test]
fn insights_render_recent_request_uses_query_bootstrap_token() {
    let recorded = Rc::new(RecordedInsightsSession::new(sample_cli_config(), "ok"));
    let port = TestInsightsPort {
        recorded: Rc::clone(&recorded),
    };

    run_render_with_port(
        InsightsRenderArgs {
            period: InsightsRenderPeriod::Recent,
            argument: "7,10".to_string(),
            as_of: None,
            format: vec![InsightsFormat::Md],
        },
        &default_context(),
        &port,
    )
    .expect("insights render recent should succeed");

    assert_eq!(recorded.command_names(), vec!["query".to_string()]);
    let request = recorded.requests().remove(0);
    assert_eq!(request["days_list"], serde_json::json!([7, 10]));
    assert_eq!(request["format"], "md");
}

#[test]
fn insights_render_single_recent_request_uses_temporal_recent_shape() {
    let recorded = Rc::new(RecordedInsightsSession::new(sample_cli_config(), "ok"));
    let port = TestInsightsPort {
        recorded: Rc::clone(&recorded),
    };

    run_render_with_port(
        InsightsRenderArgs {
            period: InsightsRenderPeriod::Recent,
            argument: "7".to_string(),
            as_of: None,
            format: vec![InsightsFormat::Md],
        },
        &default_context(),
        &port,
    )
    .expect("insights render single recent should succeed");

    let request = recorded.requests().remove(0);
    assert_eq!(request["operation_kind"], "query");
    assert_eq!(request["display_mode"], "recent");
    assert_eq!(request["selection_kind"], "recent_days");
    assert_eq!(request["days"], 7);
    assert_eq!(request["format"], "md");
}

#[test]
fn insights_render_recent_as_of_uses_temporal_anchor_date_shape() {
    let recorded = Rc::new(RecordedInsightsSession::new(sample_cli_config(), "ok"));
    let port = TestInsightsPort {
        recorded: Rc::clone(&recorded),
    };

    run_render_with_port(
        InsightsRenderArgs {
            period: InsightsRenderPeriod::Recent,
            argument: "7".to_string(),
            as_of: Some("2026-03-07".to_string()),
            format: vec![InsightsFormat::Md],
        },
        &default_context(),
        &port,
    )
    .expect("insights render recent as-of should succeed");

    let request = recorded.requests().remove(0);
    assert_eq!(request["operation_kind"], "query");
    assert_eq!(request["display_mode"], "recent");
    assert_eq!(request["selection_kind"], "recent_days");
    assert_eq!(request["days"], 7);
    assert_eq!(request["anchor_date"], "2026-03-07");
    assert_eq!(request["format"], "md");
}

#[test]
fn insights_render_day_request_uses_period_and_argument() {
    let recorded = Rc::new(RecordedInsightsSession::new(sample_cli_config(), "ok"));
    let port = TestInsightsPort {
        recorded: Rc::clone(&recorded),
    };

    run_render_with_port(
        InsightsRenderArgs {
            period: InsightsRenderPeriod::Day,
            argument: "20260103".to_string(),
            as_of: None,
            format: vec![InsightsFormat::Tex],
        },
        &default_context(),
        &port,
    )
    .expect("insights render day should succeed");

    let request = recorded.requests().remove(0);
    assert_eq!(request["operation_kind"], "query");
    assert_eq!(request["display_mode"], "day");
    assert_eq!(request["selection_kind"], "single_day");
    assert_eq!(request["date"], "2026-01-03");
    assert_eq!(request["format"], "tex");
}

#[test]
fn insights_render_range_request_uses_period_and_argument() {
    let recorded = Rc::new(RecordedInsightsSession::new(sample_cli_config(), "ok"));
    let port = TestInsightsPort {
        recorded: Rc::clone(&recorded),
    };

    run_render_with_port(
        InsightsRenderArgs {
            period: InsightsRenderPeriod::Range,
            argument: "20260101|20260131".to_string(),
            as_of: None,
            format: vec![InsightsFormat::Typ],
        },
        &default_context(),
        &port,
    )
    .expect("insights render range should succeed");

    let request = recorded.requests().remove(0);
    assert_eq!(request["operation_kind"], "query");
    assert_eq!(request["display_mode"], "range");
    assert_eq!(request["selection_kind"], "date_range");
    assert_eq!(request["start_date"], "2026-01-01");
    assert_eq!(request["end_date"], "2026-01-31");
    assert_eq!(request["format"], "typ");
}

#[test]
fn insights_session_preserves_window_metadata() {
    let recorded = Rc::new(
        RecordedInsightsSession::new(sample_cli_config(), "ok").with_window_metadata(
            InsightsWindowMetadata {
                has_records: false,
                matched_day_count: 0,
                matched_record_count: 0,
                start_date: "2024-12-01".to_string(),
                end_date: "2024-12-31".to_string(),
                requested_days: 31,
            },
        ),
    );
    let port = TestInsightsPort {
        recorded: Rc::clone(&recorded),
    };
    let session = port
        .open("query", &default_context())
        .expect("open insights session");

    let rendered = session
        .render(&serde_json::json!({
            "operation_kind": "query",
            "display_mode": "range",
            "selection_kind": "date_range",
            "start_date": "2024-12-01",
            "end_date": "2024-12-31",
            "format": "md"
        }))
        .expect("render insights");

    assert_eq!(
        rendered.insights_window_metadata,
        Some(InsightsWindowMetadata {
            has_records: false,
            matched_day_count: 0,
            matched_record_count: 0,
            start_date: "2024-12-01".to_string(),
            end_date: "2024-12-31".to_string(),
            requested_days: 31,
        })
    );
}

#[test]
fn insights_export_month_compact_input_writes_dashed_filename() {
    let export_root = temp_output_path("insights_export_month_compact", "root");
    let recorded = Rc::new(
        RecordedInsightsSession::new(sample_cli_config(), "# month\n")
            .with_runtime_output_root(export_root.to_string_lossy().to_string()),
    );
    let port = TestInsightsPort {
        recorded: Rc::clone(&recorded),
    };

    run_export_with_port(
        InsightsExportArgs {
            period: InsightsExportPeriod::Month,
            argument: Some("202603".to_string()),
            all: false,
            as_of: None,
            format: vec![InsightsFormat::Md],
        },
        &default_context(),
        &port,
    )
    .expect("month export should succeed");

    assert_eq!(recorded.command_names(), vec!["export".to_string()]);
    let request = recorded.requests().remove(0);
    assert_eq!(request["operation_kind"], "export");
    assert_eq!(request["display_mode"], "month");
    assert_eq!(request["export_scope"], "single");
    assert_eq!(request["selection_kind"], "date_range");
    assert_eq!(request["start_date"], "2026-03-01");
    assert_eq!(request["end_date"], "2026-03-31");
    let _ = fs::remove_dir_all(&export_root);
}

#[test]
fn insights_export_all_month_uses_canonical_dashed_target_names() {
    let export_root = temp_output_path("insights_export_all_month", "root");
    let recorded = Rc::new(
        RecordedInsightsSession::new(sample_cli_config(), "# month\n")
            .with_runtime_output_root(export_root.to_string_lossy().to_string())
            .with_targets("month", vec!["2026-03", "2026-04"]),
    );
    let port = TestInsightsPort {
        recorded: Rc::clone(&recorded),
    };

    run_export_with_port(
        InsightsExportArgs {
            period: InsightsExportPeriod::Month,
            argument: None,
            all: true,
            as_of: None,
            format: vec![InsightsFormat::Md],
        },
        &default_context(),
        &port,
    )
    .expect("month export all should succeed");

    let requests = recorded.requests();
    assert_eq!(requests.len(), 1);
    assert_eq!(requests[0]["operation_kind"], "export");
    assert_eq!(requests[0]["display_mode"], "month");
    assert_eq!(requests[0]["export_scope"], "all_matching");
    let _ = fs::remove_dir_all(&export_root);
}

#[test]
fn insights_export_day_and_recent_keep_existing_path_templates() {
    let export_root = temp_output_path("insights_export_day_recent", "root");
    let recorded = Rc::new(
        RecordedInsightsSession::new(sample_cli_config(), "content\n")
            .with_runtime_output_root(export_root.to_string_lossy().to_string()),
    );
    let port = TestInsightsPort {
        recorded: Rc::clone(&recorded),
    };

    run_export_with_port(
        InsightsExportArgs {
            period: InsightsExportPeriod::Day,
            argument: Some("20260103".to_string()),
            all: false,
            as_of: None,
            format: vec![InsightsFormat::Md],
        },
        &default_context(),
        &port,
    )
    .expect("day export should succeed");
    run_export_with_port(
        InsightsExportArgs {
            period: InsightsExportPeriod::Recent,
            argument: Some("7".to_string()),
            all: false,
            as_of: None,
            format: vec![InsightsFormat::Md],
        },
        &default_context(),
        &port,
    )
    .expect("recent export should succeed");

    let requests = recorded.requests();
    assert_eq!(requests.len(), 2);
    assert_eq!(requests[0]["display_mode"], "day");
    assert_eq!(requests[0]["selection_kind"], "single_day");
    assert_eq!(requests[0]["date"], "2026-01-03");
    assert_eq!(requests[1]["display_mode"], "recent");
    assert_eq!(requests[1]["selection_kind"], "recent_days");
    assert_eq!(requests[1]["days"], 7);
    let _ = fs::remove_dir_all(&export_root);
}

#[test]
fn insights_export_recent_as_of_uses_anchor_date_request_and_keeps_recent_path() {
    let export_root = temp_output_path("insights_export_recent_as_of", "root");
    let recorded = Rc::new(
        RecordedInsightsSession::new(sample_cli_config(), "content\n")
            .with_runtime_output_root(export_root.to_string_lossy().to_string()),
    );
    let port = TestInsightsPort {
        recorded: Rc::clone(&recorded),
    };

    run_export_with_port(
        InsightsExportArgs {
            period: InsightsExportPeriod::Recent,
            argument: Some("7".to_string()),
            all: false,
            as_of: Some("2026-03-07".to_string()),
            format: vec![InsightsFormat::Md],
        },
        &default_context(),
        &port,
    )
    .expect("recent as-of export should succeed");

    let request = recorded.requests().remove(0);
    assert_eq!(request["operation_kind"], "export");
    assert_eq!(request["display_mode"], "recent");
    assert_eq!(request["export_scope"], "single");
    assert_eq!(request["selection_kind"], "recent_days");
    assert_eq!(request["days"], 7);
    assert_eq!(request["anchor_date"], "2026-03-07");
    let _ = fs::remove_dir_all(&export_root);
}

#[test]
fn insights_export_output_override_stays_directory_root_even_if_it_looks_like_file() {
    let export_root = temp_output_path("insights_export_file_like_root", "md");
    let recorded = Rc::new(
        RecordedInsightsSession::new(sample_cli_config(), "content\n")
            .with_runtime_output_root(export_root.to_string_lossy().to_string()),
    );
    let port = TestInsightsPort {
        recorded: Rc::clone(&recorded),
    };

    run_export_with_port(
        InsightsExportArgs {
            period: InsightsExportPeriod::Month,
            argument: Some("2026-03".to_string()),
            all: false,
            as_of: None,
            format: vec![InsightsFormat::Md],
        },
        &default_context(),
        &port,
    )
    .expect("month export with file-like root should succeed");

    let request = recorded.requests().remove(0);
    assert_eq!(request["operation_kind"], "export");
    assert_eq!(request["display_mode"], "month");
    assert_eq!(request["selection_kind"], "date_range");
    assert_eq!(request["start_date"], "2026-03-01");
    assert_eq!(request["end_date"], "2026-03-31");
    let _ = fs::remove_dir_all(&export_root);
}
