mod assets;
mod config;
mod render;
mod request;
mod visuals;

// Orchestration-only handler: validation + flow control, with heavy logic moved into chart/* modules.
use std::env;
use std::fs;
use std::path::{Path, PathBuf};

use serde_json::Value;

use crate::cli::ChartArgs;
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::core::runtime::CoreApi;
use crate::error::AppError;

use self::assets::load_chart_assets;
use self::config::load_heatmap_config;
use self::render::{build_chart_html, chart_type_token};
use self::request::{build_chart_query_request, parse_chart_payload};
use self::visuals::resolve_heatmap_visuals;

pub struct ChartHandler;

pub(crate) trait ChartQuerySessionPort {
    fn query_data(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<String, AppError>;
}

struct RuntimeChartQuerySessionPort;

impl ChartQuerySessionPort for RuntimeChartQuerySessionPort {
    fn query_data(
        &self,
        command_name: &str,
        ctx: &CommandContext,
        request: &Value,
    ) -> Result<String, AppError> {
        let api = CoreApi::load()?;
        let session = api.bootstrap(command_name, ctx)?;
        session.query().data(request)
    }
}

impl CommandHandler<ChartArgs> for ChartHandler {
    fn handle(&self, args: ChartArgs, ctx: &CommandContext) -> Result<(), AppError> {
        run_chart_with_query_port(args, ctx, &RuntimeChartQuerySessionPort)
    }
}

fn run_chart_with_query_port(
    args: ChartArgs,
    ctx: &CommandContext,
    query_port: &dyn ChartQuerySessionPort,
) -> Result<(), AppError> {
    let exe_path =
        env::current_exe().map_err(|e| AppError::Io(format!("Resolve current exe failed: {e}")))?;

    if args.list_heatmap_palettes {
        let config = load_heatmap_config(&exe_path)?;
        println!("Heatmap palettes:");
        for (name, _colors) in &config.palettes {
            let mut suffix = String::new();
            if name == &config.defaults.light_palette {
                suffix.push_str(" (default-light)");
            }
            if name == &config.defaults.dark_palette {
                suffix.push_str(" (default-dark)");
            }
            println!("- {name}{suffix}");
        }
        return Ok(());
    }

    let output_path = validate_chart_output_path(ctx)?;
    validate_lookback_days(args.lookback_days)?;

    let request = build_chart_query_request(&args);
    let semantic_payload = query_port.query_data("chart", ctx, &request)?;
    let payload_json = parse_chart_payload(&semantic_payload)?;

    let heatmap_config = load_heatmap_config(&exe_path)?;
    let (palette_json, thresholds_json) = resolve_heatmap_visuals(&args, &heatmap_config)?;
    let chart_assets = load_chart_assets(&args)?;
    let html = build_chart_html(
        &args,
        &payload_json,
        &semantic_payload,
        &chart_assets,
        &palette_json,
        &thresholds_json,
    );

    let output_file = PathBuf::from(output_path);
    write_chart_html(&output_file, &html)?;

    println!(
        "Saved chart {} HTML: {}",
        chart_type_token(&args.chart_type),
        output_file.display()
    );
    Ok(())
}

fn validate_chart_output_path(ctx: &CommandContext) -> Result<String, AppError> {
    let output_path = ctx.output_path.clone().ok_or_else(|| {
        AppError::InvalidArguments("chart requires output path via -o/--output".to_string())
    })?;
    if output_path.trim().is_empty() {
        return Err(AppError::InvalidArguments(
            "chart output path must not be empty".to_string(),
        ));
    }
    Ok(output_path)
}

fn validate_lookback_days(lookback_days: Option<i32>) -> Result<(), AppError> {
    if let Some(days) = lookback_days {
        if days <= 0 {
            return Err(AppError::InvalidArguments(
                "--lookback-days must be greater than 0".to_string(),
            ));
        }
    }
    Ok(())
}

fn write_chart_html(output_file: &Path, html: &str) -> Result<(), AppError> {
    if let Some(parent) = output_file.parent() {
        if !parent.as_os_str().is_empty() {
            fs::create_dir_all(parent)
                .map_err(|e| AppError::Io(format!("Create chart output directory failed: {e}")))?;
        }
    }
    fs::write(output_file, html)
        .map_err(|e| AppError::Io(format!("Write chart output failed: {e}")))
}

#[cfg(test)]
mod tests {
    use crate::cli::{ChartArgs, ChartTheme, ChartType};
    use crate::commands::testing::{RecordedQuerySession, context_with_output, temp_output_path};

    use super::{ChartQuerySessionPort, run_chart_with_query_port};

    struct StubChartQueryPort<'a> {
        recorder: &'a RecordedQuerySession,
    }

    impl ChartQuerySessionPort for StubChartQueryPort<'_> {
        fn query_data(
            &self,
            command_name: &str,
            _ctx: &crate::commands::handler::CommandContext,
            request: &serde_json::Value,
        ) -> Result<String, crate::error::AppError> {
            self.recorder.record(command_name, request)
        }
    }

    fn sample_chart_args(
        output_path: &str,
    ) -> (ChartArgs, crate::commands::handler::CommandContext) {
        (
            ChartArgs {
                chart_type: ChartType::Line,
                theme: ChartTheme::Default,
                heatmap_palette: None,
                list_heatmap_palettes: false,
                root: Some("study".to_string()),
                year: None,
                month: None,
                from: Some("20260101".to_string()),
                to: Some("20260107".to_string()),
                lookback_days: None,
            },
            context_with_output(output_path),
        )
    }

    #[test]
    fn chart_handler_uses_chart_session_and_writes_html_output() {
        let output_path = temp_output_path("chart_handler", "html");
        let (args, ctx) = sample_chart_args(&output_path.to_string_lossy());
        let recorder = RecordedQuerySession::new_success(
            r#"{"selected_root":"study","from_date":"20260101","to_date":"20260107","series":[]}"#,
        );
        let port = StubChartQueryPort {
            recorder: &recorder,
        };

        run_chart_with_query_port(args, &ctx, &port).expect("chart handler should succeed");

        let recorded_commands = recorder.command_names();
        let recorded_requests = recorder.requests();
        assert_eq!(recorded_commands, vec!["chart".to_string()]);
        assert_eq!(recorded_requests.len(), 1);
        assert_eq!(recorded_requests[0]["action"], "report-chart");
        assert_eq!(recorded_requests[0]["output_mode"], "semantic_json");
        assert_eq!(recorded_requests[0]["root"], "study");
        assert_eq!(recorded_requests[0]["from_date"], "20260101");
        assert_eq!(recorded_requests[0]["to_date"], "20260107");

        let html = std::fs::read_to_string(&output_path).expect("chart html should exist");
        assert!(html.contains("Report Chart (Line)"));
        assert!(html.contains("\"selected_root\":\"study\""));
    }

    #[test]
    fn chart_handler_rejects_non_positive_lookback_before_query_session() {
        let output_path = temp_output_path("chart_handler_invalid", "html");
        let (mut args, ctx) = sample_chart_args(&output_path.to_string_lossy());
        args.lookback_days = Some(0);
        let recorder = RecordedQuerySession::new_success("{}");
        let port = StubChartQueryPort {
            recorder: &recorder,
        };

        let error = run_chart_with_query_port(args, &ctx, &port)
            .expect_err("lookback validation should fail");
        assert!(
            error
                .render_for_stderr()
                .contains("--lookback-days must be greater than 0")
        );
        assert!(recorder.requests().is_empty());
    }
}
