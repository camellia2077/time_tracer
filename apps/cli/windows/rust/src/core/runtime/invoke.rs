use std::time::Instant;

use serde::Deserialize;
use serde_json::{Value, json};

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

#[derive(Deserialize)]
struct ReportTargetsResponse {
    ok: bool,
    #[serde(default)]
    error_message: String,
    #[serde(default)]
    items: Vec<String>,
    #[serde(flatten)]
    error_contract: ErrorContract,
}

pub(crate) fn run_query_data(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<String, AppError> {
    let payload = run_text(runtime, runtime.api.symbols.runtime_query, request, "query")?;
    Ok(payload.content)
}

pub(crate) fn run_report_text(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<String, AppError> {
    let payload = run_text(
        runtime,
        runtime.api.symbols.runtime_report,
        request,
        "report",
    )?;
    Ok(payload.content)
}

pub(crate) fn run_report_batch_text(
    runtime: &CoreRuntime,
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

pub(crate) fn run_report_targets(
    runtime: &CoreRuntime,
    target_type: &str,
) -> Result<Vec<String>, AppError> {
    let run_start = Instant::now();
    let request_json = to_request_json(&json!({ "type": target_type }))?;
    let raw = unsafe { (runtime.api.symbols.runtime_report_targets)(runtime.handle, request_json.as_ptr()) };
    let payload = read_c_json::<ReportTargetsResponse>(raw, "report_targets")?;
    log_timing("runtime.report_targets", run_start.elapsed());
    if payload.ok {
        return Ok(payload.items);
    }
    Err(AppError::Logic(format_error_detail(
        payload.error_message,
        &payload.error_contract,
    )))
}

pub(crate) fn run_pipeline_convert(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<(), AppError> {
    run_ack(
        runtime,
        runtime.api.symbols.runtime_convert,
        request,
        "convert",
    )
}

pub(crate) fn run_pipeline_import(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<(), AppError> {
    run_ack(
        runtime,
        runtime.api.symbols.runtime_import,
        request,
        "import",
    )
}

pub(crate) fn run_pipeline_ingest(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<(), AppError> {
    run_ack(
        runtime,
        runtime.api.symbols.runtime_ingest,
        request,
        "ingest",
    )
}

pub(crate) fn run_pipeline_validate_structure(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<(), AppError> {
    run_ack(
        runtime,
        runtime.api.symbols.runtime_validate_structure,
        request,
        "validate_structure",
    )
}

pub(crate) fn run_pipeline_validate_logic(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<(), AppError> {
    run_ack(
        runtime,
        runtime.api.symbols.runtime_validate_logic,
        request,
        "validate_logic",
    )
}

pub(crate) fn run_tree_query(
    runtime: &CoreRuntime,
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

pub(crate) fn run_tracer_exchange_export(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<String, AppError> {
    let payload = run_text(
        runtime,
        runtime.api.symbols.runtime_crypto_encrypt,
        request,
        "exchange_export",
    )?;
    Ok(payload.content)
}

pub(crate) fn run_tracer_exchange_import(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<String, AppError> {
    let payload = run_text(
        runtime,
        runtime.api.symbols.runtime_crypto_decrypt,
        request,
        "exchange_import",
    )?;
    Ok(payload.content)
}

pub(crate) fn run_tracer_exchange_inspect(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<String, AppError> {
    let payload = run_text(
        runtime,
        runtime.api.symbols.runtime_crypto_inspect,
        request,
        "exchange_inspect",
    )?;
    Ok(payload.content)
}

fn run_ack(
    runtime: &CoreRuntime,
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
    runtime: &CoreRuntime,
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
