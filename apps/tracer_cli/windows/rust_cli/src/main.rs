mod cli;
mod commands;
mod core;
mod error;
mod licenses;

use std::env;

use clap::Parser;
use cli::Cli;
use error::{AppError, AppExitCode};

fn parse_cli(raw_args: &[String]) -> Result<Option<Cli>, AppError> {
    match Cli::try_parse_from(raw_args) {
        Ok(cli) => Ok(Some(cli)),
        Err(err) => {
            use clap::error::ErrorKind;
            let kind = err.kind();
            match kind {
                ErrorKind::DisplayHelp | ErrorKind::DisplayVersion => {
                    print!("{err}");
                    Ok(None)
                }
                ErrorKind::InvalidSubcommand => Err(AppError::Plain {
                    message: format!(
                        "Unknown command '{}'.\nRun with --help to see available commands.",
                        raw_args.get(1).map_or("", String::as_str)
                    ),
                    code: AppExitCode::CommandNotFound,
                }),
                _ => {
                    if let Some(mapped) = map_parse_error_to_contract(raw_args, kind) {
                        return Err(mapped);
                    }
                    Err(AppError::InvalidArguments(err.to_string()))
                }
            }
        }
    }
}

fn run() -> Result<(), AppError> {
    let raw_args = env::args().collect::<Vec<_>>();

    if try_handle_project_license(&raw_args)? {
        return Ok(());
    }

    if let Some(cli) = parse_cli(&raw_args)? {
        commands::execute(cli)?;
    }
    Ok(())
}

fn try_handle_project_license(raw_args: &[String]) -> Result<bool, AppError> {
    if !matches!(raw_args.get(1), Some(arg) if arg == "--license") {
        return Ok(false);
    }

    if raw_args.len() != 2 {
        return Err(AppError::InvalidArguments(
            "`--license` does not accept extra arguments.".to_string(),
        ));
    }

    println!("{}", licenses::render_project_license());
    Ok(true)
}

fn is_query_missing_required_case(raw_args: &[String]) -> bool {
    matches!(raw_args.get(1), Some(cmd) if cmd == "query") && raw_args.len() == 2
}

fn map_parse_error_to_contract(
    raw_args: &[String],
    kind: clap::error::ErrorKind,
) -> Option<AppError> {
    use clap::error::ErrorKind;

    if is_query_missing_required_case(raw_args) {
        return Some(generic_command_error(
            "query",
            "Missing required argument: <type>",
        ));
    }

    let command = raw_args.get(1).map(String::as_str)?;

    match command {
        "blink" | "ingest" => {
            if matches!(kind, ErrorKind::MissingRequiredArgument) {
                return Some(generic_command_error(
                    "ingest",
                    "Missing required argument: <path>",
                ));
            }
        }
        "query" => {
            if matches!(kind, ErrorKind::InvalidValue) {
                let query_type = raw_args.get(2).map(String::as_str).unwrap_or_default();
                if !query_type.is_empty() && !is_known_query_type(query_type) {
                    return Some(generic_command_error(
                        "query",
                        &format!(
                            "Unknown query type '{}'. Supported: day, month, week, year, recent, range, data.",
                            query_type
                        ),
                    ));
                }
            }
        }
        "tree" => {
            if matches!(kind, ErrorKind::ValueValidation | ErrorKind::InvalidValue) {
                if let Some(level) = extract_tree_level_value(raw_args) {
                    return Some(generic_command_error(
                        "tree",
                        &format!("Argument 'level' is not a valid integer: {level}"),
                    ));
                }
            }
        }
        "export" => {
            if matches!(kind, ErrorKind::MissingRequiredArgument) {
                let export_type = raw_args.get(2).map(String::as_str).unwrap_or_default();
                if requires_export_argument(export_type) && is_export_argument_missing(raw_args) {
                    return Some(generic_command_error(
                        "export",
                        &format!("Argument required for export type '{export_type}'."),
                    ));
                }
            }
        }
        _ => {}
    }

    None
}

fn generic_command_error(command: &str, detail: &str) -> AppError {
    AppError::Plain {
        message: format_contract_message(
            "cli.invalid_arguments",
            "cli",
            &format!("Error executing command '{command}': {detail}"),
            &["Run command --help to inspect required arguments."],
        ),
        code: AppExitCode::GenericError,
    }
}

fn format_contract_message(
    error_code: &str,
    error_category: &str,
    detail: &str,
    hints: &[&str],
) -> String {
    let hint_text = hints
        .iter()
        .map(|item| item.trim())
        .filter(|item| !item.is_empty())
        .collect::<Vec<_>>()
        .join(" | ");
    if hint_text.is_empty() {
        return format!("[{error_code}] {detail} (category={error_category})");
    }
    format!("[{error_code}] {detail} (category={error_category}, hints={hint_text})")
}

fn is_known_query_type(value: &str) -> bool {
    matches!(
        value,
        "day" | "month" | "week" | "year" | "recent" | "range" | "data"
    )
}

fn extract_tree_level_value(raw_args: &[String]) -> Option<String> {
    for (idx, token) in raw_args.iter().enumerate() {
        if token == "-l" || token == "--level" {
            return raw_args.get(idx + 1).cloned();
        }
        if let Some(value) = token.strip_prefix("--level=") {
            return Some(value.to_string());
        }
        if let Some(value) = token.strip_prefix("-l") {
            if !value.is_empty() {
                return Some(value.to_string());
            }
        }
    }
    None
}

fn requires_export_argument(export_type: &str) -> bool {
    matches!(
        export_type,
        "day" | "month" | "week" | "year" | "recent" | "all-recent"
    )
}

fn is_export_argument_missing(raw_args: &[String]) -> bool {
    raw_args
        .get(3)
        .map(|token| token.starts_with('-'))
        .unwrap_or(true)
}

fn main() -> std::process::ExitCode {
    init_console_encoding();

    match run() {
        Ok(()) => std::process::ExitCode::from(AppExitCode::Success as u8),
        Err(err) => {
            eprintln!("{}", err.render_for_stderr());
            std::process::ExitCode::from(err.exit_code() as u8)
        }
    }
}

#[cfg(windows)]
fn init_console_encoding() {
    windows_console::set_utf8_codepage();
}

#[cfg(not(windows))]
fn init_console_encoding() {}

#[cfg(windows)]
mod windows_console {
    const CP_UTF8: u32 = 65001;

    #[link(name = "kernel32")]
    unsafe extern "system" {
        fn SetConsoleOutputCP(code_page: u32) -> i32;
        fn SetConsoleCP(code_page: u32) -> i32;
    }

    pub fn set_utf8_codepage() {
        unsafe {
            let _ = SetConsoleOutputCP(CP_UTF8);
            let _ = SetConsoleCP(CP_UTF8);
        }
    }
}
