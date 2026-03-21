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
        return Ok(json!({
            "days_list": parse_int_list(argument)?,
            "format": format_token(format),
        }));
    }

    Ok(json!({
        "type": render_period_token(period),
        "argument": argument,
        "format": format_token(format),
    }))
}

pub fn build_export_request(
    period: ReportExportPeriod,
    argument: Option<&str>,
    all: bool,
    format: &ReportFormat,
) -> Result<Value, AppError> {
    if all {
        return build_all_export_request(period, argument, format);
    }

    let argument = require_argument(period, argument)?;
    Ok(json!({
        "type": export_period_token(period),
        "format": format_token(format),
        "argument": argument,
    }))
}

fn build_all_export_request(
    period: ReportExportPeriod,
    argument: Option<&str>,
    format: &ReportFormat,
) -> Result<Value, AppError> {
    match period {
        ReportExportPeriod::Day
        | ReportExportPeriod::Month
        | ReportExportPeriod::Week
        | ReportExportPeriod::Year => {
            if let Some(argument) = argument {
                if !argument.trim().is_empty() {
                    return Err(AppError::InvalidArguments(format!(
                        "`report export {}` does not accept <argument> when --all is set.",
                        export_period_token(period)
                    )));
                }
            }
            Ok(json!({
                "type": all_export_period_token(period),
                "format": format_token(format),
            }))
        }
        ReportExportPeriod::Recent => Ok(json!({
            "type": "all-recent",
            "format": format_token(format),
            "recent_days_list": parse_int_list(require_argument(period, argument)?)?,
        })),
    }
}

fn require_argument(period: ReportExportPeriod, argument: Option<&str>) -> Result<&str, AppError> {
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

fn parse_int_list(value: &str) -> Result<Vec<i32>, AppError> {
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
    }
}

fn all_export_period_token(value: ReportExportPeriod) -> &'static str {
    match value {
        ReportExportPeriod::Day => "all-day",
        ReportExportPeriod::Month => "all-month",
        ReportExportPeriod::Week => "all-week",
        ReportExportPeriod::Year => "all-year",
        ReportExportPeriod::Recent => "all-recent",
    }
}

fn format_token(value: &ReportFormat) -> &'static str {
    match value {
        ReportFormat::Md => "md",
        ReportFormat::Tex => "tex",
        ReportFormat::Typ => "typ",
    }
}

#[cfg(test)]
mod tests {
    use serde_json::json;

    use super::{ReportExportPeriod, ReportFormat, build_export_request};

    #[test]
    fn report_export_all_day_still_maps_to_legacy_runtime_type() {
        let request = build_export_request(ReportExportPeriod::Day, None, true, &ReportFormat::Md)
            .expect("all-day request");
        assert_eq!(
            request,
            json!({
                "type": "all-day",
                "format": "md",
            })
        );
    }

    #[test]
    fn report_export_all_recent_still_maps_days_list_to_legacy_runtime_shape() {
        let request = build_export_request(
            ReportExportPeriod::Recent,
            Some("7,10"),
            true,
            &ReportFormat::Tex,
        )
        .expect("all-recent request");
        assert_eq!(
            request,
            json!({
                "type": "all-recent",
                "format": "tex",
                "recent_days_list": [7, 10],
            })
        );
    }
}
