use std::time::Instant;

use serde::Deserialize;
use serde_json::{Value, json};

use crate::error::{AppError, AppExitCode};

use super::codec::{read_c_json, to_request_json};
use super::env_flags::log_timing;
use super::errors::{ErrorContract, format_error_detail, format_tree_error_detail};
use super::ffi::RuntimeJsonFn;
use super::{CoreRuntime, TreeResponse};

#[derive(Clone, Debug, PartialEq, Eq)]
pub(crate) struct ReportWindowMetadata {
    pub(crate) has_records: bool,
    pub(crate) matched_day_count: i32,
    pub(crate) matched_record_count: i32,
    pub(crate) start_date: String,
    pub(crate) end_date: String,
    pub(crate) requested_days: i32,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub(crate) struct ReportTextOutput {
    pub(crate) content: String,
    pub(crate) report_window_metadata: Option<ReportWindowMetadata>,
}

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
    #[serde(default)]
    has_records: Option<bool>,
    #[serde(default)]
    matched_day_count: Option<i32>,
    #[serde(default)]
    matched_record_count: Option<i32>,
    #[serde(default)]
    start_date: Option<String>,
    #[serde(default)]
    end_date: Option<String>,
    #[serde(default)]
    requested_days: Option<i32>,
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
    target_type: &str,
) -> Result<Vec<String>, AppError> {
    let run_start = Instant::now();
    let request_json = to_request_json(&json!({ "type": target_type }))?;
    let raw = unsafe {
        (runtime.api.symbols.runtime_report_targets)(runtime.handle, request_json.as_ptr())
    };
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

pub(crate) fn run_pipeline_convert(runtime: &CoreRuntime, request: &Value) -> Result<(), AppError> {
    run_ack(
        runtime,
        runtime.api.symbols.runtime_convert,
        request,
        "convert",
    )
}

pub(crate) fn run_pipeline_import(runtime: &CoreRuntime, request: &Value) -> Result<(), AppError> {
    run_ack(
        runtime,
        runtime.api.symbols.runtime_import,
        request,
        "import",
    )
}

pub(crate) fn run_pipeline_ingest(runtime: &CoreRuntime, request: &Value) -> Result<(), AppError> {
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

pub(crate) fn run_tracer_exchange_unpack(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<String, AppError> {
    let payload = run_text(
        runtime,
        runtime.api.symbols.runtime_crypto_unpack,
        request,
        "exchange_unpack",
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
    Err(map_runtime_text_error(
        payload.error_message,
        &payload.error_contract,
    ))
}

#[cfg(test)]
mod tests {
    use crate::error::AppExitCode;

    use super::{ErrorContract, map_runtime_text_error};

    #[test]
    fn reporting_target_not_found_maps_to_dedicated_exit_code() {
        let error = map_runtime_text_error(
            "Report target not found: day `2024-12-31`.".to_string(),
            &ErrorContract {
                error_code: "reporting.target.not_found".to_string(),
                error_category: "reporting".to_string(),
                hints: vec![
                    "Check that the requested report target exists in the current database."
                        .to_string(),
                ],
            },
        );

        assert!(matches!(
            error,
            crate::error::AppError::Plain {
                code: AppExitCode::ReportTargetNotFound,
                ..
            }
        ));
        assert!(
            error
                .render_for_stderr()
                .contains("reporting.target.not_found")
        );
    }
}

fn map_runtime_text_error(error_message: String, contract: &ErrorContract) -> AppError {
    let detail = format_error_detail(error_message, contract);
    if contract.error_code == "reporting.target.not_found" {
        return AppError::Plain {
            message: detail,
            code: AppExitCode::ReportTargetNotFound,
        };
    }
    AppError::Logic(detail)
}

impl TextResponse {
    fn into_report_text_output(self) -> ReportTextOutput {
        let report_window_metadata = self.report_window_metadata();
        ReportTextOutput {
            content: self.content,
            report_window_metadata,
        }
    }

    fn report_window_metadata(&self) -> Option<ReportWindowMetadata> {
        let has_any = self.has_records.is_some()
            || self.matched_day_count.is_some()
            || self.matched_record_count.is_some()
            || self.start_date.is_some()
            || self.end_date.is_some()
            || self.requested_days.is_some();
        if !has_any {
            return None;
        }

        Some(ReportWindowMetadata {
            has_records: self.has_records.unwrap_or(false),
            matched_day_count: self.matched_day_count.unwrap_or(0),
            matched_record_count: self.matched_record_count.unwrap_or(0),
            start_date: self.start_date.clone().unwrap_or_default(),
            end_date: self.end_date.clone().unwrap_or_default(),
            requested_days: self.requested_days.unwrap_or(0),
        })
    }
}
