use serde_json::Value;

use crate::cli::{ChartArgs, ChartTheme, ChartType};

use super::assets::ChartAssets;

pub(crate) fn build_chart_html(
    args: &ChartArgs,
    payload_json: &Value,
    semantic_payload: &str,
    chart_assets: &ChartAssets,
    heatmap_palette_json: &str,
    heatmap_thresholds_json: &str,
) -> String {
    let root_label = payload_json
        .get("selected_root")
        .and_then(|value| value.as_str())
        .filter(|value| !value.is_empty())
        .unwrap_or("all roots");

    let range_label = if let (Some(from), Some(to)) = (
        payload_json
            .get("from_date")
            .and_then(|value| value.as_str()),
        payload_json.get("to_date").and_then(|value| value.as_str()),
    ) {
        format!("{from} ~ {to}")
    } else if let Some(days) = payload_json
        .get("lookback_days")
        .and_then(|value| value.as_i64())
    {
        format!("lookback {days} days")
    } else {
        "unspecified".to_string()
    };

    let mut html = chart_assets.base_template.clone();
    let chart_kind = chart_type_token(&args.chart_type);
    let chart_title_label = chart_type_title_label(&args.chart_type);
    html = html.replace(
        "{{PAGE_TITLE}}",
        &format!("time_tracer report-chart {chart_kind}"),
    );
    html = html.replace(
        "{{CHART_TITLE}}",
        &format!("Report Chart ({chart_title_label})"),
    );
    html = html.replace("{{CHART_KIND}}", chart_kind);
    html = html.replace(
        "{{CHART_THEME}}",
        match args.theme {
            ChartTheme::Default => "default",
            ChartTheme::Github => "github",
        },
    );
    html = html.replace("{{ROOT_LABEL}}", root_label);
    html = html.replace("{{RANGE_LABEL}}", &range_label);
    html = html.replace(
        "{{REPORT_CHART_JSON}}",
        &escape_inline_json(semantic_payload),
    );
    html = html.replace("{{HEATMAP_PALETTE_COLORS_JSON}}", heatmap_palette_json);
    html = html.replace("{{HEATMAP_POSITIVE_HOURS_JSON}}", heatmap_thresholds_json);
    html = html.replace(
        "{{CHART_OPTION_SCRIPT}}",
        &escape_inline_script(&chart_assets.option_script),
    );
    html = html.replace(
        "{{ECHARTS_SCRIPT}}",
        &escape_inline_script(&chart_assets.echarts_script),
    );
    html
}

pub(crate) fn chart_type_token(value: &ChartType) -> &'static str {
    match value {
        ChartType::Line => "line",
        ChartType::Bar => "bar",
        ChartType::Pie => "pie",
        ChartType::HeatmapYear => "heatmap-year",
        ChartType::HeatmapMonth => "heatmap-month",
    }
}

fn chart_type_title_label(value: &ChartType) -> &'static str {
    match value {
        ChartType::Line => "Line",
        ChartType::Bar => "Bar",
        ChartType::Pie => "Pie",
        ChartType::HeatmapYear => "Heatmap Year",
        ChartType::HeatmapMonth => "Heatmap Month",
    }
}

fn escape_inline_json(value: &str) -> String {
    value.replace("</", "<\\/")
}

fn escape_inline_script(value: &str) -> String {
    value.replace("</script", "<\\/script")
}
