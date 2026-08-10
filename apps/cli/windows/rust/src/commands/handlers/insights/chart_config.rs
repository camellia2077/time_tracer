use std::collections::BTreeMap;
use std::env;
use std::fs;
use std::path::{Path, PathBuf};

use serde::Deserialize;

use crate::error::AppError;

#[derive(Debug, Deserialize)]
pub(crate) struct HeatmapConfig {
    pub(crate) thresholds: HeatmapThresholds,
    pub(crate) palettes: BTreeMap<String, Vec<String>>,
}

#[derive(Debug, Deserialize)]
pub(crate) struct HeatmapThresholds {
    pub(crate) positive_hours: Vec<f64>,
}

#[derive(Debug, Deserialize)]
struct HeatmapProgramConfig {
    palettes: BTreeMap<String, Vec<String>>,
}

#[derive(Debug, Deserialize)]
struct HeatmapUserConfig {
    thresholds: HeatmapThresholds,
}

pub(crate) fn load_heatmap_config(exe_path: &Path) -> Result<HeatmapConfig, AppError> {
    let (program_path, user_path) = resolve_heatmap_config_paths(exe_path)?;
    let program_content = fs::read_to_string(&program_path).map_err(|e| {
        AppError::Io(format!(
            "Read heatmap program config failed ({}): {e}",
            program_path.display()
        ))
    })?;
    let user_content = fs::read_to_string(&user_path).map_err(|e| {
        AppError::Config(format!(
            "Read heatmap user config failed ({}): {e}",
            user_path.display()
        ))
    })?;
    let program = toml::from_str::<HeatmapProgramConfig>(&program_content).map_err(|e| {
        AppError::Config(format!(
            "Parse heatmap program config failed ({}): {e}",
            program_path.display()
        ))
    })?;
    let user = toml::from_str::<HeatmapUserConfig>(&user_content).map_err(|e| {
        AppError::Config(format!(
            "Parse heatmap user config failed ({}): {e}",
            user_path.display()
        ))
    })?;
    Ok(HeatmapConfig {
        thresholds: user.thresholds,
        palettes: program.palettes,
    })
}

fn resolve_heatmap_config_paths(exe_path: &Path) -> Result<(PathBuf, PathBuf), AppError> {
    let mut roots = Vec::new();
    if let Some(exe_dir) = exe_path.parent() {
        roots.push(exe_dir.to_path_buf());
    }

    if let Ok(cwd) = env::current_dir() {
        roots.push(cwd.clone());
        let mut cursor = Some(cwd.as_path());
        while let Some(dir) = cursor {
            roots.push(dir.join("apps/cli/windows/rust/runtime"));
            cursor = dir.parent();
        }
    }

    roots
        .into_iter()
        .map(|root| {
            (
                root.join("config/program/charts/heatmap.toml"),
                root.join("config/user/heatmap.toml"),
            )
        })
        .find(|(program_path, user_path)| program_path.exists() && user_path.exists())
        .ok_or_else(|| {
            AppError::Config(
                "Unable to locate program and user heatmap configuration files".to_string(),
            )
        })
}
