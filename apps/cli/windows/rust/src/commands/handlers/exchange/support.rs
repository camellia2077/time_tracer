use std::fs;
use std::io::{self, IsTerminal, Write};
use std::path::PathBuf;

use rpassword::prompt_password;

use crate::cli::SecurityLevel;
use crate::error::AppError;

pub fn absolute_existing_path(raw: &str) -> Result<PathBuf, AppError> {
    if raw.trim().is_empty() {
        return Err(AppError::InvalidArguments("--in is required.".to_string()));
    }
    let path = PathBuf::from(raw);
    let absolute = if path.is_absolute() {
        path
    } else {
        std::env::current_dir()
            .map_err(|e| AppError::Io(format!("Resolve current directory failed: {e}")))?
            .join(path)
    };
    if !absolute.exists() {
        return Err(AppError::InvalidArguments(format!(
            "Input path does not exist: {}",
            absolute.display()
        )));
    }
    Ok(absolute)
}

pub fn validate_exchange_export_input(input: &PathBuf) -> Result<(), AppError> {
    if input.is_dir() {
        return Ok(());
    }
    Err(AppError::InvalidArguments(
        "--in must point to a directory containing .txt files for exchange export.".to_string(),
    ))
}

pub fn validate_exchange_package_input(input: &PathBuf) -> Result<(), AppError> {
    if input.is_file() {
        return Ok(());
    }
    Err(AppError::InvalidArguments(
        "--in must point to a .tracer file.".to_string(),
    ))
}

pub fn require_output_path_for(raw: Option<&str>, context: &str) -> Result<PathBuf, AppError> {
    let value = raw.ok_or_else(|| {
        AppError::InvalidArguments(format!("-o/--output is required for {context}."))
    })?;
    if value.trim().is_empty() {
        return Err(AppError::InvalidArguments(
            format!("-o/--output is required for {context}."),
        ));
    }
    resolve_output_path(Some(value))?.ok_or_else(|| {
        AppError::InvalidArguments(format!("-o/--output is required for {context}."))
    })
}

pub fn reject_output_path(raw: Option<&str>, context: &str) -> Result<(), AppError> {
    if raw.is_none() {
        return Ok(());
    }
    Err(AppError::InvalidArguments(format!(
        "-o/--output is not valid for {context}."
    )))
}

pub fn resolve_output_path(raw: Option<&str>) -> Result<Option<PathBuf>, AppError> {
    let Some(value) = raw else {
        return Ok(None);
    };
    if value.trim().is_empty() {
        return Err(AppError::InvalidArguments(
            "-o/--output must not be empty.".to_string(),
        ));
    }
    let path = PathBuf::from(value);
    let absolute = if path.is_absolute() {
        path
    } else {
        std::env::current_dir()
            .map_err(|e| AppError::Io(format!("Resolve current directory failed: {e}")))?
            .join(path)
    };

    if let Some(parent) = absolute.parent() {
        if !parent.as_os_str().is_empty() && !parent.exists() {
            fs::create_dir_all(parent).map_err(|e| {
                AppError::Io(format!(
                    "Create output parent directory failed ({}): {e}",
                    parent.display()
                ))
            })?;
        }
    }
    Ok(Some(absolute))
}

pub fn prompt_passphrase_for_export() -> Result<String, AppError> {
    let first = read_secret_prompt("Enter passphrase (required): ")?;
    if first.is_empty() {
        return Err(AppError::InvalidArguments(
            "Passphrase is required.".to_string(),
        ));
    }
    let second = read_secret_prompt("Confirm passphrase: ")?;
    if first != second {
        return Err(AppError::InvalidArguments(
            "Passphrase confirmation does not match.".to_string(),
        ));
    }
    Ok(first)
}

pub fn prompt_passphrase_for_package() -> Result<String, AppError> {
    let passphrase = read_secret_prompt("Enter passphrase (required): ")?;
    if passphrase.is_empty() {
        return Err(AppError::InvalidArguments(
            "Passphrase is required.".to_string(),
        ));
    }
    Ok(passphrase)
}

pub fn security_level_token(level: Option<SecurityLevel>) -> &'static str {
    match level.unwrap_or(SecurityLevel::Interactive) {
        SecurityLevel::Min => "min",
        SecurityLevel::Interactive => "interactive",
        SecurityLevel::Moderate => "moderate",
        SecurityLevel::High => "high",
        SecurityLevel::Max => "max",
    }
}

pub fn print_content(content: &str) {
    if content.is_empty() {
        return;
    }
    if content.ends_with('\n') {
        print!("{content}");
        return;
    }
    println!("{content}");
}

fn read_secret_prompt(prompt: &str) -> Result<String, AppError> {
    if io::stdin().is_terminal() {
        return prompt_password(prompt)
            .map(|secret| secret.trim_end_matches(['\r', '\n']).to_string())
            .map_err(|e| AppError::Io(format!("Read hidden passphrase failed: {e}")));
    }
    read_line_prompt(prompt)
}

fn read_line_prompt(prompt: &str) -> Result<String, AppError> {
    print!("{prompt}");
    io::stdout()
        .flush()
        .map_err(|e| AppError::Io(format!("Flush stdout failed: {e}")))?;
    let mut input = String::new();
    io::stdin()
        .read_line(&mut input)
        .map_err(|e| AppError::Io(format!("Read stdin failed: {e}")))?;
    while input.ends_with('\n') || input.ends_with('\r') {
        input.pop();
    }
    Ok(input)
}
