use std::env;
use std::fs;
use std::path::PathBuf;

use crate::cli::{ChartArgs, ChartType};
use crate::error::AppError;

pub(crate) struct ChartAssets {
    pub(crate) base_template: String,
    pub(crate) option_script: String,
    pub(crate) echarts_script: String,
}

pub(crate) fn load_chart_assets(args: &ChartArgs) -> Result<ChartAssets, AppError> {
    let assets_root = resolve_assets_root()?;
    let base_template =
        fs::read_to_string(assets_root.join("templates/report_chart_base.html.tpl"))
            .map_err(|e| AppError::Io(format!("Read chart base template failed: {e}")))?;
    let option_script = fs::read_to_string(assets_root.join(chart_option_template(args)))
        .map_err(|e| AppError::Io(format!("Read chart option template failed: {e}")))?;
    let echarts_script = fs::read_to_string(assets_root.join("vendor/echarts/echarts.min.js"))
        .map_err(|e| AppError::Io(format!("Read echarts script failed: {e}")))?;

    Ok(ChartAssets {
        base_template,
        option_script,
        echarts_script,
    })
}

fn chart_option_template(args: &ChartArgs) -> &'static str {
    match args.chart_type {
        ChartType::Line => "templates/report_chart_option_line.js.tpl",
        ChartType::Bar => "templates/report_chart_option_bar.js.tpl",
        ChartType::Pie => "templates/report_chart_option_pie.js.tpl",
        ChartType::HeatmapYear => "templates/report_chart_option_heatmap_year.js.tpl",
        ChartType::HeatmapMonth => "templates/report_chart_option_heatmap_month.js.tpl",
    }
}

fn resolve_assets_root() -> Result<PathBuf, AppError> {
    let exe =
        env::current_exe().map_err(|e| AppError::Io(format!("Resolve current exe failed: {e}")))?;
    let mut candidates = Vec::new();
    if let Some(exe_dir) = exe.parent() {
        candidates.push(exe_dir.join("assets"));
    }
    if let Ok(cwd) = env::current_dir() {
        candidates.push(cwd.join("assets"));
        let mut cursor = Some(cwd.as_path());
        while let Some(dir) = cursor {
            candidates.push(dir.join("apps/tracer_cli/windows/rust_cli/runtime/assets"));
            cursor = dir.parent();
        }
    }

    candidates
        .into_iter()
        .find(|dir| dir.join("templates/report_chart_base.html.tpl").exists())
        .ok_or_else(|| AppError::Config("Unable to locate chart assets directory".to_string()))
}
