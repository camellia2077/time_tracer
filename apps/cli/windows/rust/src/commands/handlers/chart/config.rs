use std::collections::BTreeMap;
use std::env;
use std::fs;
use std::path::{Path, PathBuf};

use serde::Deserialize;

use crate::error::AppError;

#[derive(Debug, Deserialize)]
pub(crate) struct HeatmapConfig {
    pub(crate) thresholds: HeatmapThresholds,
    pub(crate) defaults: HeatmapDefaults,
    pub(crate) palettes: BTreeMap<String, Vec<String>>,
}

#[derive(Debug, Deserialize)]
pub(crate) struct HeatmapThresholds {
    pub(crate) positive_hours: Vec<f64>,
}

#[derive(Debug, Deserialize)]
pub(crate) struct HeatmapDefaults {
    pub(crate) light_palette: String,
    pub(crate) dark_palette: String,
}

pub(crate) fn load_heatmap_config(exe_path: &Path) -> Result<HeatmapConfig, AppError> {
    let path = resolve_heatmap_config_path(exe_path)?;
    let content = fs::read_to_string(&path).map_err(|e| {
        AppError::Io(format!(
            "Read heatmap config failed ({}): {e}",
            path.display()
        ))
    })?;
    toml::from_str::<HeatmapConfig>(&content).map_err(|e| {
        AppError::Config(format!(
            "Parse heatmap config failed ({}): {e}",
            path.display()
        ))
    })
}

fn resolve_heatmap_config_path(exe_path: &Path) -> Result<PathBuf, AppError> {
    let mut candidates = Vec::new();
    if let Some(exe_dir) = exe_path.parent() {
        candidates.push(exe_dir.join("config/charts/heatmap.toml"));
        candidates.push(exe_dir.join("config/heatmap.toml"));
    }

    if let Ok(cwd) = env::current_dir() {
        candidates.push(cwd.join("config/charts/heatmap.toml"));
        candidates.push(cwd.join("config/heatmap.toml"));
        let mut cursor = Some(cwd.as_path());
        while let Some(dir) = cursor {
            candidates.push(dir.join("apps/cli/windows/rust/runtime/config/charts/heatmap.toml"));
            cursor = dir.parent();
        }
    }

    candidates
        .into_iter()
        .find(|path| path.exists())
        .ok_or_else(|| AppError::Config("Unable to locate heatmap.toml".to_string()))
}
