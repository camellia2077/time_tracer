use std::time::Instant;

use serde::Deserialize;
use serde_json::Value;

use crate::error::AppError;

use super::codec::{read_c_json, to_request_json};
use super::env_flags::log_timing;
use super::errors::{ErrorContract, format_error_detail, format_tree_error_detail};
use super::ffi::RuntimeJsonFn;
use super::{CoreRuntime, TreeResponse};

#[derive(Deserialize)]
struct AckResponse {
    ok: bool,
    #[serde(default)]
    error_message: String,
    #[serde(flatten)]
    error_contract: ErrorContract,
}

#[derive(Deserialize)]
struct TextResponse {
    ok: bool,
    #[serde(default)]
    error_message: String,
    #[serde(default)]
    content: String,
    #[serde(flatten)]
    error_contract: ErrorContract,
}

pub(crate) fn run_query(runtime: &CoreRuntime<'_>, request: &Value) -> Result<String, AppError> {
    let payload = run_text(runtime, runtime.api.symbols.runtime_query, request, "query")?;
    Ok(payload.content)
}

pub(crate) fn run_report(runtime: &CoreRuntime<'_>, request: &Value) -> Result<String, AppError> {
    let payload = run_text(
        runtime,
        runtime.api.symbols.runtime_report,
        request,
        "report",
    )?;
    Ok(payload.content)
}

pub(crate) fn run_report_batch(
    runtime: &CoreRuntime<'_>,
    request: &Value,
) -> Result<String, AppError> {
    let payload = run_text(
        runtime,
        runtime.api.symbols.runtime_report_batch,
        request,
        "report_batch",
    )?;
    Ok(payload.content)
}

pub(crate) fn run_export(runtime: &CoreRuntime<'_>, request: &Value) -> Result<(), AppError> {
    run_ack(
        runtime,
        runtime.api.symbols.runtime_export,
        request,
        "export",
    )
}

pub(crate) fn run_convert(runtime: &CoreRuntime<'_>, request: &Value) -> Result<(), AppError> {
    run_ack(
        runtime,
        runtime.api.symbols.runtime_convert,
        request,
        "convert",
    )
}

pub(crate) fn run_import(runtime: &CoreRuntime<'_>, request: &Value) -> Result<(), AppError> {
    run_ack(
        runtime,
        runtime.api.symbols.runtime_import,
        request,
        "import",
    )
}

pub(crate) fn run_ingest(runtime: &CoreRuntime<'_>, request: &Value) -> Result<(), AppError> {
    run_ack(
        runtime,
        runtime.api.symbols.runtime_ingest,
        request,
        "ingest",
    )
}

pub(crate) fn run_validate_structure(
    runtime: &CoreRuntime<'_>,
    request: &Value,
) -> Result<(), AppError> {
    run_ack(
        runtime,
        runtime.api.symbols.runtime_validate_structure,
        request,
        "validate_structure",
    )
}

pub(crate) fn run_validate_logic(
    runtime: &CoreRuntime<'_>,
    request: &Value,
) -> Result<(), AppError> {
    run_ack(
        runtime,
        runtime.api.symbols.runtime_validate_logic,
        request,
        "validate_logic",
    )
}

pub(crate) fn run_tree(
    runtime: &CoreRuntime<'_>,
    request: &Value,
) -> Result<TreeResponse, AppError> {
    let run_start = Instant::now();
    let request_json = to_request_json(request)?;
    let raw = unsafe { (runtime.api.symbols.runtime_tree)(runtime.handle, request_json.as_ptr()) };
    let payload = read_c_json::<TreeResponse>(raw, "tree")?;
    log_timing("runtime.tree", run_start.elapsed());
    if payload.ok {
        return Ok(payload);
    }
    Err(AppError::Logic(format_tree_error_detail(&payload)))
}

pub(crate) fn run_crypto_encrypt(
    runtime: &CoreRuntime<'_>,
    request: &Value,
) -> Result<String, AppError> {
    let payload = run_text(
        runtime,
        runtime.api.symbols.runtime_crypto_encrypt,
        request,
        "crypto_encrypt",
    )?;
    Ok(payload.content)
}

pub(crate) fn run_crypto_decrypt(
    runtime: &CoreRuntime<'_>,
    request: &Value,
) -> Result<String, AppError> {
    let payload = run_text(
        runtime,
        runtime.api.symbols.runtime_crypto_decrypt,
        request,
        "crypto_decrypt",
    )?;
    Ok(payload.content)
}

pub(crate) fn run_crypto_inspect(
    runtime: &CoreRuntime<'_>,
    request: &Value,
) -> Result<String, AppError> {
    let payload = run_text(
        runtime,
        runtime.api.symbols.runtime_crypto_inspect,
        request,
        "crypto_inspect",
    )?;
    Ok(payload.content)
}

fn run_ack(
    runtime: &CoreRuntime<'_>,
    function: RuntimeJsonFn,
    request: &Value,
    context: &str,
) -> Result<(), AppError> {
    let run_start = Instant::now();
    let request_json = to_request_json(request)?;
    let raw = unsafe { function(runtime.handle, request_json.as_ptr()) };
    let payload = read_c_json::<AckResponse>(raw, context)?;
    log_timing(&format!("runtime.{context}"), run_start.elapsed());
    if payload.ok {
        return Ok(());
    }
    Err(AppError::Logic(format_error_detail(
        payload.error_message,
        &payload.error_contract,
    )))
}

fn run_text(
    runtime: &CoreRuntime<'_>,
    function: RuntimeJsonFn,
    request: &Value,
    context: &str,
) -> Result<TextResponse, AppError> {
    let run_start = Instant::now();
    let request_json = to_request_json(request)?;
    let raw = unsafe { function(runtime.handle, request_json.as_ptr()) };
    let payload = read_c_json::<TextResponse>(raw, context)?;
    log_timing(&format!("runtime.{context}"), run_start.elapsed());
    if payload.ok {
        return Ok(payload);
    }
    Err(AppError::Logic(format_error_detail(
        payload.error_message,
        &payload.error_contract,
    )))
}
