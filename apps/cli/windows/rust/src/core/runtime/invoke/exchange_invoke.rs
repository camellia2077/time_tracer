use serde_json::Value;

use crate::error::AppError;

use super::super::CoreRuntime;
use super::transport::run_text;

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
