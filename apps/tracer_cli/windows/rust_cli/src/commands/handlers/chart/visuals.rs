use crate::cli::{ChartArgs, ChartType};
use crate::error::AppError;

use super::config::HeatmapConfig;

pub(crate) fn resolve_heatmap_visuals(
    args: &ChartArgs,
    config: &HeatmapConfig,
) -> Result<(String, String), AppError> {
    let thresholds = serde_json::to_string(&config.thresholds.positive_hours)
        .map_err(|e| AppError::Generic(format!("Serialize heatmap thresholds failed: {e}")))?;

    if !matches!(
        args.chart_type,
        ChartType::HeatmapYear | ChartType::HeatmapMonth
    ) {
        return Ok(("[]".to_string(), thresholds));
    }

    let palette_name = args
        .heatmap_palette
        .clone()
        .unwrap_or_else(|| config.defaults.light_palette.clone());
    let palette = config.palettes.get(&palette_name).ok_or_else(|| {
        AppError::InvalidArguments(format!("Unknown heatmap palette `{palette_name}`"))
    })?;

    let colors = serde_json::to_string(palette)
        .map_err(|e| AppError::Generic(format!("Serialize heatmap palette failed: {e}")))?;
    Ok((colors, thresholds))
}
