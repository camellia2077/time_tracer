use std::path::{Path, PathBuf};

use crate::cli::{InsightsExportArgs, InsightsExportPeriod, InsightsFormat, InsightsRenderArgs};
use crate::core::runtime::CliConfig;
use crate::error::AppError;

use super::dates::normalize_export_argument;

pub(crate) fn resolve_render_formats(
    args: &InsightsRenderArgs,
    cli_config: &CliConfig,
) -> Vec<InsightsFormat> {
    if !args.format.is_empty() {
        return args.format.clone();
    }
    if let Some(value) = &cli_config.command_defaults.query_format {
        let formats = parse_format_tokens(value);
        if !formats.is_empty() {
            return formats;
        }
    }
    if let Some(value) = &cli_config.defaults.default_format {
        let formats = parse_format_tokens(value);
        if !formats.is_empty() {
            return formats;
        }
    }
    vec![InsightsFormat::Md]
}

pub(crate) fn resolve_export_formats(
    args: &InsightsExportArgs,
    cli_config: &CliConfig,
) -> Vec<InsightsFormat> {
    if !args.format.is_empty() {
        return args.format.clone();
    }
    if let Some(value) = &cli_config.command_defaults.export_format {
        let formats = parse_format_tokens(value);
        if !formats.is_empty() {
            return formats;
        }
    }
    if let Some(value) = &cli_config.defaults.default_format {
        let formats = parse_format_tokens(value);
        if !formats.is_empty() {
            return formats;
        }
    }
    vec![InsightsFormat::Md]
}

pub(crate) fn normalize_export_name(
    period: InsightsExportPeriod,
    argument: &str,
) -> Result<String, AppError> {
    normalize_export_argument(period, argument)
}

pub(crate) fn build_export_output_path(
    export_root: &Path,
    format: &InsightsFormat,
    period: InsightsExportPeriod,
    normalized_id: &str,
) -> Result<PathBuf, AppError> {
    let base_dir = export_root.join(insights_format_dir(format));
    let extension = insights_format_extension(format);
    match period {
        InsightsExportPeriod::Day => {
            if normalized_id.len() != 10 {
                return Err(AppError::InvalidArguments(format!(
                    "Normalized day id must be YYYY-MM-DD, got `{normalized_id}`."
                )));
            }
            Ok(base_dir
                .join("day")
                .join(&normalized_id[..4])
                .join(&normalized_id[5..7])
                .join(format!("{normalized_id}.{extension}")))
        }
        InsightsExportPeriod::Month => Ok(base_dir
            .join("month")
            .join(format!("{normalized_id}.{extension}"))),
        InsightsExportPeriod::Week => Ok(base_dir
            .join("week")
            .join(format!("{normalized_id}.{extension}"))),
        InsightsExportPeriod::Year => Ok(base_dir
            .join("year")
            .join(format!("{normalized_id}.{extension}"))),
        InsightsExportPeriod::Recent => Ok(base_dir
            .join("recent")
            // Preserve legacy recent file naming even when request resolution uses range.
            .join(format!("last_{normalized_id}_days_insights.{extension}"))),
        InsightsExportPeriod::Range => {
            let fs_safe_id = normalized_id.replace('|', "_");
            Ok(base_dir
                .join("range")
                .join(format!("{fs_safe_id}.{extension}")))
        }
    }
}

pub(crate) fn format_token(value: &InsightsFormat) -> &'static str {
    match value {
        InsightsFormat::Md => "md",
        InsightsFormat::Tex => "tex",
        InsightsFormat::Typ => "typ",
    }
}

pub(crate) fn insights_format_dir(value: &InsightsFormat) -> &'static str {
    match value {
        InsightsFormat::Md => "markdown",
        InsightsFormat::Tex => "latex",
        InsightsFormat::Typ => "typ",
    }
}

pub(crate) fn insights_format_extension(value: &InsightsFormat) -> &'static str {
    match value {
        InsightsFormat::Md => "md",
        InsightsFormat::Tex => "tex",
        InsightsFormat::Typ => "typ",
    }
}

fn parse_format_tokens(value: &str) -> Vec<InsightsFormat> {
    value
        .split(',')
        .filter_map(|token| match token.trim().to_ascii_lowercase().as_str() {
            "md" => Some(InsightsFormat::Md),
            "tex" => Some(InsightsFormat::Tex),
            "typ" => Some(InsightsFormat::Typ),
            _ => None,
        })
        .collect()
}

pub(crate) fn export_period_token(value: InsightsExportPeriod) -> &'static str {
    match value {
        InsightsExportPeriod::Day => "day",
        InsightsExportPeriod::Month => "month",
        InsightsExportPeriod::Week => "week",
        InsightsExportPeriod::Year => "year",
        InsightsExportPeriod::Recent => "recent",
        InsightsExportPeriod::Range => "range",
    }
}
