use std::env;
use std::ffi::CString;
use std::path::Path;
use std::time::Instant;

use serde::Deserialize;

use crate::commands::handler::CommandContext;
use crate::error::AppError;

use super::callbacks::configure_callbacks;
use super::codec::{option_to_cstring, path_to_cstring, read_c_json};
use super::env_flags::log_timing;
use super::errors::{ErrorContract, format_error_detail};
use super::{CliConfig, CoreApi, CoreRuntime, ResolvedCliContext, ResolvedCliPaths};

#[derive(Deserialize)]
struct RuntimeCheckResponse {
    ok: bool,
    #[serde(default)]
    error_message: String,
    #[serde(default)]
    messages: Vec<String>,
    #[serde(flatten)]
    error_contract: ErrorContract,
}

#[derive(Deserialize)]
struct ResolveCliContextResponse {
    ok: bool,
    #[serde(default)]
    error_message: String,
    paths: Option<ResolvedCliPaths>,
    cli_config: Option<CliConfig>,
    #[serde(flatten)]
    error_contract: ErrorContract,
}

pub(crate) fn bootstrap<'a>(
    api: &'a CoreApi,
    command_name: &str,
    ctx: &CommandContext,
) -> Result<(CoreRuntime<'a>, CliConfig), AppError> {
    let bootstrap_start = Instant::now();
    configure_callbacks(api);
    let exe = env::current_exe()
        .map_err(|e| AppError::Io(format!("Resolve current exe path failed: {e}")))?;

    let check_start = Instant::now();
    check_environment(api, &exe)?;
    log_timing("runtime.check_environment", check_start.elapsed());

    let resolve_start = Instant::now();
    let resolved = resolve_cli_context(api, &exe, command_name, ctx)?;
    log_timing("runtime.resolve_cli_context", resolve_start.elapsed());

    let create_start = Instant::now();
    let runtime = create_runtime(api, &resolved.paths)?;
    log_timing("runtime.create", create_start.elapsed());
    log_timing("runtime.bootstrap_total", bootstrap_start.elapsed());
    Ok((runtime, resolved.cli_config))
}

fn check_environment(api: &CoreApi, exe: &Path) -> Result<(), AppError> {
    let exe_c = path_to_cstring(exe)?;
    let raw = unsafe { (api.symbols.runtime_check_environment)(exe_c.as_ptr(), 0) };
    let payload = read_c_json::<RuntimeCheckResponse>(raw, "runtime_check_environment")?;
    if payload.ok {
        return Ok(());
    }
    let mut detail = format_error_detail(payload.error_message, &payload.error_contract);
    if detail.is_empty() {
        detail = "runtime environment check failed".to_string();
    }
    if payload.error_contract.error_category == "config" {
        return Err(AppError::Config(detail));
    }
    if payload
        .messages
        .iter()
        .any(|message| message.contains("config/config.toml"))
    {
        return Err(AppError::Config(detail));
    }
    if payload
        .messages
        .iter()
        .any(|message| message.contains("missing required runtime file"))
    {
        return Err(AppError::DllCompatibility(detail));
    }
    Err(AppError::Io(detail))
}

fn resolve_cli_context(
    api: &CoreApi,
    exe: &Path,
    command_name: &str,
    ctx: &CommandContext,
) -> Result<ResolvedCliContext, AppError> {
    let exe_c = path_to_cstring(exe)?;
    let db_c = option_to_cstring(ctx.db_path.as_deref())?;
    let output_c = option_to_cstring(ctx.output_path.as_deref())?;
    let command_c = CString::new(command_name)
        .map_err(|e| AppError::InvalidArguments(format!("Invalid command name string: {e}")))?;
    let raw = unsafe {
        (api.symbols.runtime_resolve_cli_context)(
            exe_c.as_ptr(),
            db_c.as_ptr(),
            output_c.as_ptr(),
            command_c.as_ptr(),
        )
    };
    let payload = read_c_json::<ResolveCliContextResponse>(raw, "runtime_resolve_cli_context")?;
    if !payload.ok {
        return Err(AppError::Config(format_error_detail(
            payload.error_message,
            &payload.error_contract,
        )));
    }

    let paths = payload
        .paths
        .ok_or_else(|| AppError::Config("runtime_resolve_cli_context missing paths".into()))?;
    let cli_config = payload
        .cli_config
        .ok_or_else(|| AppError::Config("runtime_resolve_cli_context missing cli_config".into()))?;
    Ok(ResolvedCliContext { paths, cli_config })
}

fn create_runtime<'a>(
    api: &'a CoreApi,
    paths: &ResolvedCliPaths,
) -> Result<CoreRuntime<'a>, AppError> {
    let db_c = CString::new(paths.db_path.clone())
        .map_err(|e| AppError::InvalidArguments(format!("Invalid db path: {e}")))?;
    let output_c = CString::new(paths.runtime_output_root.clone())
        .map_err(|e| AppError::InvalidArguments(format!("Invalid output root: {e}")))?;
    let converter_c = CString::new(paths.converter_config_toml_path.clone())
        .map_err(|e| AppError::InvalidArguments(format!("Invalid converter config path: {e}")))?;
    let handle = unsafe {
        (api.symbols.runtime_create)(db_c.as_ptr(), output_c.as_ptr(), converter_c.as_ptr())
    };
    if handle.is_null() {
        return Err(AppError::DllCompatibility(
            "Create core runtime failed.".to_string(),
        ));
    }
    Ok(CoreRuntime { api, handle })
}
