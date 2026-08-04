use std::time::Instant;

use serde_json::Value;

use crate::error::AppError;

use super::super::codec::{read_c_json, to_request_json};
use super::super::env_flags::log_timing;
use super::super::CoreRuntime;
use super::super::{TxtCanonicalReplaceOutput, TxtReplaceOutput, TxtResolveOutput};
use super::responses::{TxtReplaceResponse, TxtResolveResponse};
use super::transport::map_runtime_text_error;

pub(crate) fn run_txt_resolve_day_block(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<TxtResolveOutput, AppError> {
    let run_start = Instant::now();
    let request_json = to_request_json(request)?;
    let raw =
        unsafe { (runtime.api.symbols.runtime_config)(runtime.handle, request_json.as_ptr()) };
    let payload = read_c_json::<TxtResolveResponse>(raw, "txt")?;
    log_timing("runtime.txt", run_start.elapsed());
    if !payload.ok {
        return Err(map_runtime_text_error(
            payload.error_message,
            &payload.error_contract,
        ));
    }
    Ok(TxtResolveOutput {
        normalized_day_marker: payload.normalized_day_marker,
        found: payload.found,
        is_marker_valid: payload.is_marker_valid,
        can_save: payload.can_save,
        day_body: payload.day_body,
        day_content_iso_date: payload.day_content_iso_date,
    })
}

pub(crate) fn run_txt_replace_day_block(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<TxtReplaceOutput, AppError> {
    let run_start = Instant::now();
    let request_json = to_request_json(request)?;
    let raw =
        unsafe { (runtime.api.symbols.runtime_config)(runtime.handle, request_json.as_ptr()) };
    let payload = read_c_json::<TxtReplaceResponse>(raw, "txt")?;
    log_timing("runtime.txt", run_start.elapsed());
    if !payload.ok {
        return Err(map_runtime_text_error(
            payload.error_message,
            &payload.error_contract,
        ));
    }
    Ok(TxtReplaceOutput {
        normalized_day_marker: payload.normalized_day_marker,
        found: payload.found,
        is_marker_valid: payload.is_marker_valid,
        updated_content: payload.updated_content,
    })
}

pub(crate) fn run_txt_replace_canonical_activity_names(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<TxtCanonicalReplaceOutput, AppError> {
    let run_start = Instant::now();
    let request_json = to_request_json(request)?;
    let raw =
        unsafe { (runtime.api.symbols.runtime_config)(runtime.handle, request_json.as_ptr()) };
    let payload = read_c_json::<TxtReplaceResponse>(raw, "txt")?;
    log_timing("runtime.txt", run_start.elapsed());
    if !payload.ok {
        return Err(map_runtime_text_error(
            payload.error_message,
            &payload.error_contract,
        ));
    }
    Ok(TxtCanonicalReplaceOutput {
        updated_content: payload.updated_content,
    })
}

pub(crate) fn run_txt_replace_alias_activity_names(
    runtime: &CoreRuntime,
    request: &Value,
) -> Result<TxtCanonicalReplaceOutput, AppError> {
    let run_start = Instant::now();
    let request_json = to_request_json(request)?;
    let raw =
        unsafe { (runtime.api.symbols.runtime_config)(runtime.handle, request_json.as_ptr()) };
    let payload = read_c_json::<TxtReplaceResponse>(raw, "txt")?;
    log_timing("runtime.txt", run_start.elapsed());
    if !payload.ok {
        return Err(map_runtime_text_error(
            payload.error_message,
            &payload.error_contract,
        ));
    }
    Ok(TxtCanonicalReplaceOutput {
        updated_content: payload.updated_content,
    })
}
