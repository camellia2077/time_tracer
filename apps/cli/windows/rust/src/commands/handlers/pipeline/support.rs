use serde_json::{Value, json};

use crate::cli::{DateCheckMode, PipelineConvertArgs, PipelineImportArgs, PipelineIngestArgs};
use crate::core::runtime::CliConfig;

pub fn build_convert_request(args: &PipelineConvertArgs, cli_config: &CliConfig) -> Value {
    json!({
        "input_path": args.path,
        "date_check_mode": cli_config
            .command_defaults
            .convert_date_check_mode
            .clone()
            .or(cli_config.default_date_check_mode.clone())
            .unwrap_or_else(|| "none".to_string()),
        "save_processed_output": cli_config
            .command_defaults
            .convert_save_processed_output
            .unwrap_or(cli_config.default_save_processed_output),
        "validate_logic": cli_config
            .command_defaults
            .convert_validate_logic
            .unwrap_or(true),
        "validate_structure": cli_config
            .command_defaults
            .convert_validate_structure
            .unwrap_or(true),
    })
}

pub fn build_import_request(args: &PipelineImportArgs) -> Value {
    json!({ "processed_path": args.path })
}

pub fn build_ingest_request(args: &PipelineIngestArgs, cli_config: &CliConfig) -> Value {
    json!({
        "input_path": args.path,
        "date_check_mode": resolve_date_check_mode(
            cli_config.command_defaults.ingest_date_check_mode.clone(),
            cli_config.default_date_check_mode.clone(),
            args.date_check,
            args.no_date_check,
        ),
        "save_processed_output": resolve_save_processed_output(
            cli_config.command_defaults.ingest_save_processed_output,
            cli_config.default_save_processed_output,
            args.save_processed,
            args.no_save_processed,
        ),
    })
}

pub fn build_validate_structure_request(path: &str) -> Value {
    json!({ "input_path": path })
}

pub fn build_validate_logic_request(
    path: &str,
    cli_config: &CliConfig,
    date_check: Option<DateCheckMode>,
    no_date_check: bool,
) -> Value {
    json!({
        "input_path": path,
        "date_check_mode": resolve_date_check_mode(
            cli_config.command_defaults.validate_logic_date_check_mode.clone(),
            cli_config.default_date_check_mode.clone(),
            date_check,
            no_date_check,
        ),
    })
}

fn resolve_date_check_mode(
    command_default: Option<String>,
    global_default: Option<String>,
    explicit: Option<DateCheckMode>,
    no_date_check: bool,
) -> String {
    if let Some(mode) = explicit {
        return date_check_mode_token(mode).to_string();
    }
    if no_date_check {
        return "none".to_string();
    }
    command_default
        .or(global_default)
        .unwrap_or_else(|| "none".to_string())
}

fn resolve_save_processed_output(
    command_default: Option<bool>,
    global_default: bool,
    enable_flag: bool,
    disable_flag: bool,
) -> bool {
    if disable_flag {
        return false;
    }
    if enable_flag {
        return true;
    }
    command_default.unwrap_or(global_default)
}

fn date_check_mode_token(value: DateCheckMode) -> &'static str {
    match value {
        DateCheckMode::None => "none",
        DateCheckMode::Continuity => "continuity",
        DateCheckMode::Full => "full",
    }
}
