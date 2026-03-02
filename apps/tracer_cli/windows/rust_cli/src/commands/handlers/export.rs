use serde_json::json;

use crate::cli::{ExportArgs, ExportFormat, ExportType};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::core::runtime::CoreApi;
use crate::error::AppError;

pub struct ExportHandler;

impl CommandHandler<ExportArgs> for ExportHandler {
    fn handle(&self, args: ExportArgs, ctx: &CommandContext) -> Result<(), AppError> {
        let api = CoreApi::load()?;
        let (runtime, cli_config) = api.bootstrap("export", ctx)?;
        let formats = resolve_export_formats(
            &args,
            &cli_config.command_defaults.export_format,
            &cli_config.defaults.default_format,
        );
        let export_type = export_type_token(&args.export_type);

        for format in formats {
            let mut request = json!({
                "type": export_type,
                "format": export_format_token(&format),
            });

            if let Some(argument) = args.argument.as_ref() {
                if matches!(args.export_type, ExportType::AllRecent) {
                    request["recent_days_list"] = json!(parse_int_list(argument)?);
                } else {
                    request["argument"] = json!(argument);
                }
            }

            runtime.run_export(&request)?;
        }
        Ok(())
    }
}

fn resolve_export_formats(
    args: &ExportArgs,
    command_default: &Option<String>,
    global_default: &Option<String>,
) -> Vec<ExportFormat> {
    if !args.format.is_empty() {
        return args.format.clone();
    }
    if let Some(value) = command_default {
        let formats = parse_format_tokens(value);
        if !formats.is_empty() {
            return formats;
        }
    }
    if let Some(value) = global_default {
        let formats = parse_format_tokens(value);
        if !formats.is_empty() {
            return formats;
        }
    }
    vec![ExportFormat::Md]
}

fn parse_format_tokens(value: &str) -> Vec<ExportFormat> {
    value
        .split(',')
        .filter_map(|token| match token.trim().to_ascii_lowercase().as_str() {
            "md" => Some(ExportFormat::Md),
            "tex" => Some(ExportFormat::Tex),
            "typ" => Some(ExportFormat::Typ),
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

fn export_type_token(value: &ExportType) -> &'static str {
    match value {
        ExportType::Day => "day",
        ExportType::Month => "month",
        ExportType::Week => "week",
        ExportType::Year => "year",
        ExportType::Recent => "recent",
        ExportType::AllDay => "all-day",
        ExportType::AllMonth => "all-month",
        ExportType::AllWeek => "all-week",
        ExportType::AllYear => "all-year",
        ExportType::AllRecent => "all-recent",
    }
}

fn export_format_token(value: &ExportFormat) -> &'static str {
    match value {
        ExportFormat::Md => "md",
        ExportFormat::Tex => "tex",
        ExportFormat::Typ => "typ",
    }
}
