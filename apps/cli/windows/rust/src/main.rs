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
            match err.kind() {
                ErrorKind::DisplayHelp | ErrorKind::DisplayVersion => {
                    print!("{err}");
                    Ok(None)
                }
                ErrorKind::InvalidSubcommand if is_unknown_top_level_command(raw_args) => {
                    Err(AppError::Plain {
                        message: format!(
                            "Unknown command '{}'.\nRun with --help to see available commands.",
                            raw_args.get(1).map_or("", String::as_str)
                        ),
                        code: AppExitCode::CommandNotFound,
                    })
                }
                _ => Err(AppError::InvalidArguments(err.to_string())),
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

fn is_unknown_top_level_command(raw_args: &[String]) -> bool {
    let Some(command) = raw_args.get(1).map(String::as_str) else {
        return false;
    };
    !matches!(
        command,
        "query"
            | "chart"
            | "pipeline"
            | "report"
            | "exchange"
            | "doctor"
            | "licenses"
            | "tracer"
            | "motto"
    )
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
