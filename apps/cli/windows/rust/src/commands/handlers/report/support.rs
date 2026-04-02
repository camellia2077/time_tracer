use std::path::{Path, PathBuf};

use serde_json::{Value, json};

use crate::cli::{
    ReportExportArgs, ReportExportPeriod, ReportFormat, ReportRenderArgs, ReportRenderPeriod,
};
use crate::core::runtime::CliConfig;
use crate::error::AppError;

pub fn resolve_render_formats(
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

pub fn resolve_export_formats(
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

pub fn build_render_request(
    period: ReportRenderPeriod,
    argument: &str,
    format: &ReportFormat,
) -> Result<Value, AppError> {
    if matches!(period, ReportRenderPeriod::Recent) {
        let days = parse_positive_int_list("`report render recent`", argument)?;
        if days.len() == 1 {
            return Ok(json!({
                "type": "recent",
                "argument": days[0].to_string(),
                "format": format_token(format),
            }));
        }
        return Ok(json!({
            "days_list": days,
            "format": format_token(format),
        }));
    }

    let normalized_argument = normalize_render_argument(period, argument)?;

    Ok(json!({
        "type": render_period_token(period),
        "argument": normalized_argument,
        "format": format_token(format),
    }))
}

pub fn build_export_render_request(
    period: ReportExportPeriod,
    argument: &str,
    format: &ReportFormat,
) -> Result<Value, AppError> {
    let normalized_argument = normalize_export_argument(period, argument)?;
    Ok(json!({
        "type": export_period_token(period),
        "argument": normalized_argument,
        "format": format_token(format),
    }))
}

pub fn require_export_argument(
    period: ReportExportPeriod,
    argument: Option<&str>,
) -> Result<&str, AppError> {
    let Some(argument) = argument else {
        return Err(AppError::InvalidArguments(format!(
            "`report export {}` requires <argument>.",
            export_period_token(period)
        )));
    };
    if argument.trim().is_empty() {
        return Err(AppError::InvalidArguments(format!(
            "`report export {}` requires a non-empty <argument>.",
            export_period_token(period)
        )));
    }
    Ok(argument)
}

pub fn reject_argument_when_all(
    period: ReportExportPeriod,
    argument: Option<&str>,
) -> Result<(), AppError> {
    if let Some(argument) = argument {
        if !argument.trim().is_empty() {
            return Err(AppError::InvalidArguments(format!(
                "`report export {}` does not accept <argument> when --all is set.",
                export_period_token(period)
            )));
        }
    }
    Ok(())
}

pub fn parse_int_list(value: &str) -> Result<Vec<i32>, AppError> {
    let mut out = Vec::new();
    for token in value.split(',') {
        let t = token.trim();
        if t.is_empty() {
            continue;
        }
        let n = t.parse::<i32>().map_err(|e| {
            AppError::InvalidArguments(format!("Invalid integer in list `{value}`: {e}"))
        })?;
        out.push(n);
    }
    Ok(out)
}

fn parse_positive_int_list(command_label: &str, value: &str) -> Result<Vec<i32>, AppError> {
    let out = parse_int_list(value)?;
    if out.is_empty() {
        return Err(AppError::InvalidArguments(format!(
            "{command_label} expects a positive integer."
        )));
    }
    if let Some(invalid) = out.iter().find(|days| **days <= 0) {
        return Err(AppError::InvalidArguments(format!(
            "{command_label} expects positive integers, got `{invalid}`."
        )));
    }
    Ok(out)
}

pub fn list_targets_type(period: ReportExportPeriod) -> Result<&'static str, AppError> {
    match period {
        ReportExportPeriod::Day => Ok("day"),
        ReportExportPeriod::Month => Ok("month"),
        ReportExportPeriod::Week => Ok("week"),
        ReportExportPeriod::Year => Ok("year"),
        ReportExportPeriod::Recent => Err(AppError::InvalidArguments(
            "`report export recent --all` must enumerate days locally.".to_string(),
        )),
        ReportExportPeriod::Range => Err(AppError::InvalidArguments(
            "`report export range --all` is not supported; specify an explicit range.".to_string(),
        )),
    }
}

pub fn normalize_export_name(
    period: ReportExportPeriod,
    argument: &str,
) -> Result<String, AppError> {
    normalize_export_argument(period, argument)
}

pub fn build_export_output_path(
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
            .join(format!("last_{normalized_id}_days_report.{extension}"))),
        ReportExportPeriod::Range => {
            let fs_safe_id = normalized_id.replace('|', "_");
            Ok(base_dir
                .join("range")
                .join(format!("{fs_safe_id}.{extension}")))
        }
    }
}

pub fn format_token(value: &ReportFormat) -> &'static str {
    match value {
        ReportFormat::Md => "md",
        ReportFormat::Tex => "tex",
        ReportFormat::Typ => "typ",
    }
}

pub fn report_format_dir(value: &ReportFormat) -> &'static str {
    match value {
        ReportFormat::Md => "markdown",
        ReportFormat::Tex => "latex",
        ReportFormat::Typ => "typ",
    }
}

pub fn report_format_extension(value: &ReportFormat) -> &'static str {
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

fn render_period_token(value: ReportRenderPeriod) -> &'static str {
    match value {
        ReportRenderPeriod::Day => "day",
        ReportRenderPeriod::Month => "month",
        ReportRenderPeriod::Week => "week",
        ReportRenderPeriod::Year => "year",
        ReportRenderPeriod::Recent => "recent",
        ReportRenderPeriod::Range => "range",
    }
}

fn export_period_token(value: ReportExportPeriod) -> &'static str {
    match value {
        ReportExportPeriod::Day => "day",
        ReportExportPeriod::Month => "month",
        ReportExportPeriod::Week => "week",
        ReportExportPeriod::Year => "year",
        ReportExportPeriod::Recent => "recent",
        ReportExportPeriod::Range => "range",
    }
}

fn normalize_render_argument(
    period: ReportRenderPeriod,
    argument: &str,
) -> Result<String, AppError> {
    match period {
        ReportRenderPeriod::Day => normalize_day_argument("`report render day`", argument),
        ReportRenderPeriod::Month => normalize_month_argument("`report render month`", argument),
        ReportRenderPeriod::Week => normalize_week_argument("`report render week`", argument),
        ReportRenderPeriod::Year => normalize_year_argument("`report render year`", argument),
        ReportRenderPeriod::Recent => unreachable!("recent arguments are parsed as days_list"),
        ReportRenderPeriod::Range => normalize_range_argument("`report render range`", argument),
    }
}

fn normalize_export_argument(
    period: ReportExportPeriod,
    argument: &str,
) -> Result<String, AppError> {
    match period {
        ReportExportPeriod::Day => normalize_day_argument("`report export day`", argument),
        ReportExportPeriod::Month => normalize_month_argument("`report export month`", argument),
        ReportExportPeriod::Week => normalize_week_argument("`report export week`", argument),
        ReportExportPeriod::Year => normalize_year_argument("`report export year`", argument),
        ReportExportPeriod::Recent => normalize_recent_argument("`report export recent`", argument),
        ReportExportPeriod::Range => normalize_range_argument("`report export range`", argument),
    }
}

fn normalize_day_argument(command_label: &str, value: &str) -> Result<String, AppError> {
    let normalized = if value.len() == 8 && value.chars().all(|ch| ch.is_ascii_digit()) {
        format!("{}-{}-{}", &value[..4], &value[4..6], &value[6..8])
    } else if value.len() == 10
        && value.as_bytes()[4] == b'-'
        && value.as_bytes()[7] == b'-'
        && value
            .chars()
            .enumerate()
            .all(|(index, ch)| matches!(index, 4 | 7) || ch.is_ascii_digit())
    {
        value.to_string()
    } else {
        return Err(AppError::InvalidArguments(format!(
            "{command_label} expects YYYYMMDD or YYYY-MM-DD, got `{value}`."
        )));
    };

    // Semantic validation: reject impossible dates like 2026-02-30.
    let year: u32 = normalized[..4].parse().unwrap_or(0);
    let month: u32 = normalized[5..7].parse().unwrap_or(0);
    let day: u32 = normalized[8..10].parse().unwrap_or(0);
    let max_day = match month {
        1 | 3 | 5 | 7 | 8 | 10 | 12 => 31,
        4 | 6 | 9 | 11 => 30,
        2 => {
            if (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0) {
                29
            } else {
                28
            }
        }
        _ => 0,
    };
    if month < 1 || month > 12 || day < 1 || day > max_day {
        return Err(AppError::InvalidArguments(format!(
            "{command_label} expects YYYYMMDD or YYYY-MM-DD, got `{value}`."
        )));
    }

    Ok(normalized)
}

fn normalize_month_argument(command_label: &str, value: &str) -> Result<String, AppError> {
    if value.len() == 6 && value.chars().all(|ch| ch.is_ascii_digit()) {
        return Ok(format!("{}-{}", &value[..4], &value[4..6]));
    }
    if value.len() == 7
        && value.as_bytes()[4] == b'-'
        && value
            .chars()
            .enumerate()
            .all(|(index, ch)| index == 4 || ch.is_ascii_digit())
    {
        return Ok(value.to_string());
    }
    Err(AppError::InvalidArguments(format!(
        "{command_label} expects YYYYMM or YYYY-MM, got `{value}`."
    )))
}

fn normalize_week_argument(command_label: &str, value: &str) -> Result<String, AppError> {
    if value.len() == 8
        && value.as_bytes()[4] == b'-'
        && value.as_bytes()[5] == b'W'
        && value[..4].chars().all(|ch| ch.is_ascii_digit())
        && value[6..].chars().all(|ch| ch.is_ascii_digit())
    {
        return Ok(value.to_string());
    }
    Err(AppError::InvalidArguments(format!(
        "{command_label} expects YYYY-Www, got `{value}`."
    )))
}

fn normalize_year_argument(command_label: &str, value: &str) -> Result<String, AppError> {
    if value.len() == 4 && value.chars().all(|ch| ch.is_ascii_digit()) {
        return Ok(value.to_string());
    }
    Err(AppError::InvalidArguments(format!(
        "{command_label} expects a 4-digit year, got `{value}`."
    )))
}

fn normalize_recent_argument(command_label: &str, value: &str) -> Result<String, AppError> {
    let days = value.parse::<i32>().map_err(|error| {
        AppError::InvalidArguments(format!(
            "{command_label} expects a positive integer, got `{value}`: {error}"
        ))
    })?;
    if days <= 0 {
        return Err(AppError::InvalidArguments(format!(
            "{command_label} expects a positive integer, got `{value}`."
        )));
    }
    Ok(days.to_string())
}

fn normalize_range_argument(command_label: &str, value: &str) -> Result<String, AppError> {
    let Some((start, end)) = value.split_once('|') else {
        return Err(AppError::InvalidArguments(
            format!("{command_label} expects `start|end`.")
        ));
    };
    if end.contains('|') {
        return Err(AppError::InvalidArguments(
            format!("{command_label} expects exactly one `|` separator.")
        ));
    }

    let normalized_start = normalize_day_argument(command_label, start.trim())?;
    let normalized_end = normalize_day_argument(command_label, end.trim())?;
    if normalized_start > normalized_end {
        return Err(AppError::InvalidArguments(
            format!("{command_label} expects start <= end.")
        ));
    }

    Ok(format!("{normalized_start}|{normalized_end}"))
}

#[cfg(test)]
mod tests {
    use std::path::Path;

    use super::{
        ReportExportPeriod, ReportFormat, ReportRenderPeriod, build_export_output_path,
        build_export_render_request, build_render_request, normalize_export_name,
    };

    #[test]
    fn normalize_month_accepts_compact_and_dashed_input() {
        assert_eq!(
            normalize_export_name(ReportExportPeriod::Month, "202603").expect("compact month"),
            "2026-03"
        );
        assert_eq!(
            normalize_export_name(ReportExportPeriod::Month, "2026-03").expect("dashed month"),
            "2026-03"
        );
    }

    #[test]
    fn build_month_export_output_path_uses_dashed_filename() {
        let path = build_export_output_path(
            Path::new("C:/tmp/out"),
            &ReportFormat::Md,
            ReportExportPeriod::Month,
            "2026-03",
        )
        .expect("month path");
        assert_eq!(
            path,
            Path::new("C:/tmp/out")
                .join("markdown")
                .join("month")
                .join("2026-03.md")
        );
    }

    #[test]
    fn build_render_request_normalizes_compact_day_and_month_input() {
        let day_request =
            build_render_request(ReportRenderPeriod::Day, "20260103", &ReportFormat::Md)
                .expect("day request");
        assert_eq!(day_request["argument"], "2026-01-03");

        let month_request =
            build_render_request(ReportRenderPeriod::Month, "202603", &ReportFormat::Md)
                .expect("month request");
        assert_eq!(month_request["argument"], "2026-03");
    }

    #[test]
    fn build_render_request_normalizes_range_and_single_recent_input() {
        let range_request = build_render_request(
            ReportRenderPeriod::Range,
            "20260101|20260131",
            &ReportFormat::Md,
        )
        .expect("range request");
        assert_eq!(range_request["argument"], "2026-01-01|2026-01-31");

        let recent_request =
            build_render_request(ReportRenderPeriod::Recent, "7", &ReportFormat::Md)
                .expect("recent request");
        assert_eq!(recent_request["type"], "recent");
        assert_eq!(recent_request["argument"], "7");
    }

    #[test]
    fn build_export_render_request_normalizes_compact_day_and_month_input() {
        let day_request =
            build_export_render_request(ReportExportPeriod::Day, "20260103", &ReportFormat::Md)
                .expect("day request");
        assert_eq!(day_request["argument"], "2026-01-03");

        let month_request =
            build_export_render_request(ReportExportPeriod::Month, "202603", &ReportFormat::Md)
                .expect("month request");
        assert_eq!(month_request["argument"], "2026-03");
    }
}
