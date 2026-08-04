use std::time::Instant;

use serde_json::Value;

use crate::error::{AppError, AppExitCode};

use super::super::codec::{read_c_json, to_request_json};
use super::super::env_flags::log_timing;
use super::super::errors::{format_error_detail, ErrorContract};
use super::super::ffi::RuntimeJsonFn;
use super::super::CoreRuntime;
use super::responses::{AckResponse, TextResponse};

pub(crate) fn run_ack(
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

pub(crate) fn run_text(
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

    use super::super::super::errors::ErrorContract;
    use super::map_runtime_text_error;

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
        assert!(error
            .render_for_stderr()
            .contains("reporting.target.not_found"));
    }
}

pub(crate) fn map_runtime_text_error(error_message: String, contract: &ErrorContract) -> AppError {
    let detail = format_error_detail(error_message, contract);
    if contract.error_code == "reporting.target.not_found" {
        return AppError::Plain {
            message: detail,
            code: AppExitCode::ReportTargetNotFound,
        };
    }
    AppError::Logic(detail)
}
