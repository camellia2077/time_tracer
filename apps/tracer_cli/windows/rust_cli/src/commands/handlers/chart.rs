mod assets;
mod config;
mod render;
mod request;
mod visuals;

// Orchestration-only handler: validation + flow control, with heavy logic moved into chart/* modules.
use std::env;
use std::fs;
use std::path::{Path, PathBuf};

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

impl CommandHandler<ChartArgs> for ChartHandler {
    fn handle(&self, args: ChartArgs, ctx: &CommandContext) -> Result<(), AppError> {
        let exe_path = env::current_exe()
            .map_err(|e| AppError::Io(format!("Resolve current exe failed: {e}")))?;

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

        let api = CoreApi::load()?;
        let (runtime, _cli_config) = api.bootstrap("chart", ctx)?;

        let request = build_chart_query_request(&args);
        let semantic_payload = runtime.run_query(&request)?;
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
