use std::time::Instant;

use serde_json::{json, Value};

use crate::error::AppError;

use super::super::codec::{read_c_json, to_request_json};
use super::super::env_flags::log_timing;
use super::super::CoreRuntime;
use super::responses::{ReportTargetsResponse, ReportTextOutput};
use super::transport::{map_runtime_text_error, run_ack, run_text};

pub(crate) fn run_query_data(runtime: &CoreRuntime, request: &Value) -> Result<String, AppError> {
    let payload = run_text(runtime, runtime.api.symbols.runtime_query, request, "query")?;
    Ok(payload.content)
}

pub(crate) fn run_report_text(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<ReportTextOutput, AppError> {
    let payload = run_text(
        runtime,
        runtime.api.symbols.runtime_report,
        request,
        "report",
    )?;
    Ok(payload.into_report_text_output())
}

pub(crate) fn run_report_batch_text(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<ReportTextOutput, AppError> {
    let payload = run_text(
        runtime,
        runtime.api.symbols.runtime_report_batch,
        request,
        "report_batch",
    )?;
    Ok(payload.into_report_text_output())
}

pub(crate) fn run_report_targets(
    runtime: &CoreRuntime,
    display_mode: &str,
) -> Result<Vec<String>, AppError> {
    let run_start = Instant::now();
    let request_json = to_request_json(&json!({
        "operation_kind": "targets",
        "display_mode": display_mode,
    }))?;
    let raw =
        unsafe { (runtime.api.symbols.runtime_report)(runtime.handle, request_json.as_ptr()) };
    let payload = read_c_json::<ReportTargetsResponse>(raw, "report_targets")?;
    log_timing("runtime.report_targets", run_start.elapsed());
    if payload.ok {
        return Ok(payload.items);
    }
    Err(map_runtime_text_error(
        payload.error_message,
        &payload.error_contract,
    ))
}

pub(crate) fn run_report_export(runtime: &CoreRuntime, request: &Value) -> Result<(), AppError> {
    run_ack(
        runtime,
        runtime.api.symbols.runtime_report,
        request,
        "report_export",
    )
}
