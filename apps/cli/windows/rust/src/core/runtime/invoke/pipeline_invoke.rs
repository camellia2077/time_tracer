use serde_json::Value;

use crate::error::AppError;

use super::super::CoreRuntime;
use super::transport::run_ack;

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
