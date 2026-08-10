use std::time::Instant;

use serde_json::{json, Value};

use crate::error::AppError;

use super::super::codec::{read_c_json, to_request_json};
use super::super::env_flags::log_timing;
use super::super::CoreRuntime;
use super::responses::{InsightsTargetsResponse, InsightsTextOutput};
use super::transport::{map_runtime_text_error, run_ack, run_text};

pub(crate) fn run_query_data(runtime: &CoreRuntime, request: &Value) -> Result<String, AppError> {
    let payload = run_text(runtime, runtime.api.symbols.runtime_query, request, "query")?;
    Ok(payload.content)
}

pub(crate) fn run_insights_text(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<InsightsTextOutput, AppError> {
    let payload = run_text(
        runtime,
        runtime.api.symbols.runtime_insights,
        request,
        "insights",
    )?;
    Ok(payload.into_insights_text_output())
}

pub(crate) fn run_insights_batch_text(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<InsightsTextOutput, AppError> {
    let payload = run_text(
        runtime,
        runtime.api.symbols.runtime_insights_batch,
        request,
        "insights_batch",
    )?;
    Ok(payload.into_insights_text_output())
}

pub(crate) fn run_insights_targets(
    runtime: &CoreRuntime,
    display_mode: &str,
) -> Result<Vec<String>, AppError> {
    let run_start = Instant::now();
    let request_json = to_request_json(&json!({
        "operation_kind": "targets",
        "display_mode": display_mode,
    }))?;
    let raw =
        unsafe { (runtime.api.symbols.runtime_insights)(runtime.handle, request_json.as_ptr()) };
    let payload = read_c_json::<InsightsTargetsResponse>(raw, "insights_targets")?;
    log_timing("runtime.insights_targets", run_start.elapsed());
    if payload.ok {
        return Ok(payload.items);
    }
    Err(map_runtime_text_error(
        payload.error_message,
        &payload.error_contract,
    ))
}

pub(crate) fn run_insights_export(runtime: &CoreRuntime, request: &Value) -> Result<(), AppError> {
    run_ack(
        runtime,
        runtime.api.symbols.runtime_insights,
        request,
        "insights_export",
    )
}
