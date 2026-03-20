use std::fs;
use std::io::{self, IsTerminal, Write};
use std::path::PathBuf;

use rpassword::prompt_password;
use serde_json::json;

use crate::cli::{CryptoAction, CryptoArgs, SecurityLevel};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::core::runtime::CoreApi;
use crate::error::AppError;

pub struct CryptoHandler;

struct CryptoProgressLineGuard;

impl Drop for CryptoProgressLineGuard {
    fn drop(&mut self) {
        crate::core::runtime::finalize_crypto_progress_line();
    }
}

impl CommandHandler<CryptoArgs> for CryptoHandler {
    fn handle(&self, args: CryptoArgs, ctx: &CommandContext) -> Result<(), AppError> {
        let _progress_guard = CryptoProgressLineGuard;
        let CryptoArgs {
            action,
            input,
            security_level,
        } = args;

        validate_security_level_usage(&action, security_level.as_ref())?;
        validate_output_usage(&action, ctx.output_path.as_ref())?;
        let input = absolute_existing_path(&input)?;
        validate_input_usage(&action, &input)?;

        let api = CoreApi::load()?;
        let bootstrap_ctx = CommandContext {
            db_path: ctx.db_path.clone(),
            output_path: None,
        };
        let (runtime, cli_config) = api.bootstrap("crypto", &bootstrap_ctx)?;

        match action {
            CryptoAction::Encrypt => {
                let output = absolute_output_path_for_encrypt(ctx.output_path.as_deref())?;
                let passphrase = prompt_passphrase_for_encrypt()?;
                let security_level = map_security_level(security_level);
                let date_check_mode = cli_config
                    .default_date_check_mode
                    .unwrap_or_else(|| "none".to_string());

                let request = json!({
                    "input_path": input.to_string_lossy().to_string(),
                    "output_path": output.to_string_lossy().to_string(),
                    "passphrase": passphrase,
                    "security_level": security_level,
                    "date_check_mode": date_check_mode,
                });
                let content = runtime.run_crypto_encrypt(&request)?;
                print_content(&content);
                Ok(())
            }
            CryptoAction::Decrypt => {
                let passphrase = prompt_passphrase_for_decrypt()?;
                let output = absolute_output_path(ctx.output_path.as_deref())?;

                let mut request = json!({
                    "input_path": input.to_string_lossy().to_string(),
                    "passphrase": passphrase,
                });
                if let Some(output) = output {
                    request["output_path"] = json!(output.to_string_lossy().to_string());
                }
                let content = runtime.run_crypto_decrypt(&request)?;
                print_content(&content);
                Ok(())
            }
            CryptoAction::Inspect => {
                let passphrase = prompt_passphrase_for_decrypt()?;
                let request = json!({
                    "input_path": input.to_string_lossy().to_string(),
                    "passphrase": passphrase,
                });
                let content = runtime.run_crypto_inspect(&request)?;
                print_content(&content);
                Ok(())
            }
        }
    }
}

fn validate_security_level_usage(
    action: &CryptoAction,
    security_level: Option<&SecurityLevel>,
) -> Result<(), AppError> {
    if security_level.is_none() || matches!(action, CryptoAction::Encrypt) {
        return Ok(());
    }
    Err(AppError::InvalidArguments(
        "--security-level is only valid for encrypt.".to_string(),
    ))
}

fn validate_output_usage(action: &CryptoAction, output: Option<&String>) -> Result<(), AppError> {
    if output.is_none() || matches!(action, CryptoAction::Encrypt | CryptoAction::Decrypt) {
        return Ok(());
    }
    Err(AppError::InvalidArguments(
        "--output/--out is not valid for inspect.".to_string(),
    ))
}

fn validate_input_usage(action: &CryptoAction, input: &PathBuf) -> Result<(), AppError> {
    match action {
        CryptoAction::Encrypt if !input.is_dir() => Err(AppError::InvalidArguments(
            "--in must point to a directory containing .txt files for encrypt.".to_string(),
        )),
        CryptoAction::Decrypt | CryptoAction::Inspect if !input.is_file() => Err(
            AppError::InvalidArguments("--in must point to a .tracer file.".to_string()),
        ),
        _ => Ok(()),
    }
}

fn absolute_existing_path(raw: &str) -> Result<PathBuf, AppError> {
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

fn absolute_output_path_for_encrypt(raw: Option<&str>) -> Result<PathBuf, AppError> {
    let value = raw
        .ok_or_else(|| {
            AppError::InvalidArguments("--output/--out is required for encrypt.".to_string())
        })?;
    if value.trim().is_empty() {
        return Err(AppError::InvalidArguments(
            "--output/--out is required for encrypt.".to_string(),
        ));
    }
    absolute_output_path(Some(value))?.ok_or_else(|| {
        AppError::InvalidArguments("--output/--out is required for encrypt.".to_string())
    })
}

fn absolute_output_path(raw: Option<&str>) -> Result<Option<PathBuf>, AppError> {
    let Some(value) = raw else {
        return Ok(None);
    };
    if value.trim().is_empty() {
        return Err(AppError::InvalidArguments(
            "--output/--out must not be empty.".to_string(),
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

fn prompt_passphrase_for_encrypt() -> Result<String, AppError> {
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

fn prompt_passphrase_for_decrypt() -> Result<String, AppError> {
    let passphrase = read_secret_prompt("Enter passphrase (required): ")?;
    if passphrase.is_empty() {
        return Err(AppError::InvalidArguments(
            "Passphrase is required.".to_string(),
        ));
    }
    Ok(passphrase)
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

fn map_security_level(level: Option<SecurityLevel>) -> &'static str {
    match level.unwrap_or(SecurityLevel::Interactive) {
        SecurityLevel::Min => "min",
        SecurityLevel::Interactive => "interactive",
        SecurityLevel::Moderate => "moderate",
        SecurityLevel::High => "high",
        SecurityLevel::Max => "max",
    }
}

fn print_content(content: &str) {
    if content.is_empty() {
        return;
    }
    if content.ends_with('\n') {
        print!("{content}");
        return;
    }
    println!("{content}");
}
