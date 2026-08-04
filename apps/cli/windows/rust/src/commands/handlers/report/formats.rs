use std::path::{Path, PathBuf};

use crate::cli::{ReportExportArgs, ReportExportPeriod, ReportFormat, ReportRenderArgs};
use crate::core::runtime::CliConfig;
use crate::error::AppError;

use super::dates::normalize_export_argument;

pub(crate) fn resolve_render_formats(
    args: &ReportRenderArgs,
    cli_config: &CliConfig,
) -> Vec<ReportFormat> {
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
    vec![ReportFormat::Md]
}

pub(crate) fn resolve_export_formats(
    args: &ReportExportArgs,
    cli_config: &CliConfig,
) -> Vec<ReportFormat> {
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
    vec![ReportFormat::Md]
}

pub(crate) fn normalize_export_name(
    period: ReportExportPeriod,
    argument: &str,
) -> Result<String, AppError> {
    normalize_export_argument(period, argument)
}

pub(crate) fn build_export_output_path(
    export_root: &Path,
    format: &ReportFormat,
    period: ReportExportPeriod,
    normalized_id: &str,
) -> Result<PathBuf, AppError> {
    let base_dir = export_root.join(report_format_dir(format));
    let extension = report_format_extension(format);
    match period {
        ReportExportPeriod::Day => {
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
        ReportExportPeriod::Month => Ok(base_dir
            .join("month")
            .join(format!("{normalized_id}.{extension}"))),
        ReportExportPeriod::Week => Ok(base_dir
            .join("week")
            .join(format!("{normalized_id}.{extension}"))),
        ReportExportPeriod::Year => Ok(base_dir
            .join("year")
            .join(format!("{normalized_id}.{extension}"))),
        ReportExportPeriod::Recent => Ok(base_dir
            .join("recent")
            // Preserve legacy recent file naming even when request resolution uses range.
            .join(format!("last_{normalized_id}_days_report.{extension}"))),
        ReportExportPeriod::Range => {
            let fs_safe_id = normalized_id.replace('|', "_");
            Ok(base_dir
                .join("range")
                .join(format!("{fs_safe_id}.{extension}")))
        }
    }
}

pub(crate) fn format_token(value: &ReportFormat) -> &'static str {
    match value {
        ReportFormat::Md => "md",
        ReportFormat::Tex => "tex",
        ReportFormat::Typ => "typ",
    }
}

pub(crate) fn report_format_dir(value: &ReportFormat) -> &'static str {
    match value {
        ReportFormat::Md => "markdown",
        ReportFormat::Tex => "latex",
        ReportFormat::Typ => "typ",
    }
}

pub(crate) fn report_format_extension(value: &ReportFormat) -> &'static str {
    match value {
        ReportFormat::Md => "md",
        ReportFormat::Tex => "tex",
        ReportFormat::Typ => "typ",
    }
}

fn parse_format_tokens(value: &str) -> Vec<ReportFormat> {
    value
        .split(',')
        .filter_map(|token| match token.trim().to_ascii_lowercase().as_str() {
            "md" => Some(ReportFormat::Md),
            "tex" => Some(ReportFormat::Tex),
            "typ" => Some(ReportFormat::Typ),
            _ => None,
        })
        .collect()
}

pub(crate) fn export_period_token(value: ReportExportPeriod) -> &'static str {
    match value {
        ReportExportPeriod::Day => "day",
        ReportExportPeriod::Month => "month",
        ReportExportPeriod::Week => "week",
        ReportExportPeriod::Year => "year",
        ReportExportPeriod::Recent => "recent",
        ReportExportPeriod::Range => "range",
    }
}
