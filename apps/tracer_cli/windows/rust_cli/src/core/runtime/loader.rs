use std::env;
use std::path::{Path, PathBuf};

use libloading::Library;

use crate::error::AppError;

use super::ffi::{
    RuntimeCheckEnvironmentFn, RuntimeCreateFn, RuntimeDestroyFn, RuntimeJsonFn,
    RuntimeResolveCliContextFn, RuntimeSymbols, SetCryptoProgressCallbackFn,
    SetDiagnosticsCallbackFn, SetLogCallbackFn,
};

pub(crate) fn resolve_core_dll_path() -> Result<PathBuf, AppError> {
    if let Ok(path) = env::var("TRACER_CORE_DLL") {
        return Ok(PathBuf::from(path));
    }
    let exe = env::current_exe()
        .map_err(|e| AppError::Io(format!("Resolve current exe path failed: {e}")))?;
    Ok(exe
        .parent()
        .unwrap_or_else(|| Path::new("."))
        .join("tracer_core.dll"))
}

pub(crate) fn load_runtime_symbols(lib: &Library) -> Result<RuntimeSymbols, AppError> {
    unsafe {
        let set_log_callback: SetLogCallbackFn = *lib
            .get(b"tracer_core_set_log_callback")
            .map_err(symbol_error)?;
        let set_diagnostics_callback: SetDiagnosticsCallbackFn = *lib
            .get(b"tracer_core_set_diagnostics_callback")
            .map_err(symbol_error)?;
        let set_crypto_progress_callback: Option<SetCryptoProgressCallbackFn> =
            match lib.get(b"tracer_core_set_crypto_progress_callback") {
                Ok(symbol) => Some(*symbol),
                Err(_) => None,
            };
        let runtime_check_environment: RuntimeCheckEnvironmentFn = *lib
            .get(b"tracer_core_runtime_check_environment_json")
            .map_err(symbol_error)?;
        let runtime_resolve_cli_context: RuntimeResolveCliContextFn = *lib
            .get(b"tracer_core_runtime_resolve_cli_context_json")
            .map_err(symbol_error)?;
        let runtime_create: RuntimeCreateFn = *lib
            .get(b"tracer_core_runtime_create")
            .map_err(symbol_error)?;
        let runtime_destroy: RuntimeDestroyFn = *lib
            .get(b"tracer_core_runtime_destroy")
            .map_err(symbol_error)?;
        let runtime_ingest: RuntimeJsonFn = *lib
            .get(b"tracer_core_runtime_ingest_json")
            .map_err(symbol_error)?;
        let runtime_convert: RuntimeJsonFn = *lib
            .get(b"tracer_core_runtime_convert_json")
            .map_err(symbol_error)?;
        let runtime_import: RuntimeJsonFn = *lib
            .get(b"tracer_core_runtime_import_json")
            .map_err(symbol_error)?;
        let runtime_validate_structure: RuntimeJsonFn = *lib
            .get(b"tracer_core_runtime_validate_structure_json")
            .map_err(symbol_error)?;
        let runtime_validate_logic: RuntimeJsonFn = *lib
            .get(b"tracer_core_runtime_validate_logic_json")
            .map_err(symbol_error)?;
        let runtime_query: RuntimeJsonFn = *lib
            .get(b"tracer_core_runtime_query_json")
            .map_err(symbol_error)?;
        let runtime_report: RuntimeJsonFn = *lib
            .get(b"tracer_core_runtime_report_json")
            .map_err(symbol_error)?;
        let runtime_report_batch: RuntimeJsonFn = *lib
            .get(b"tracer_core_runtime_report_batch_json")
            .map_err(symbol_error)?;
        let runtime_export: RuntimeJsonFn = *lib
            .get(b"tracer_core_runtime_export_json")
            .map_err(symbol_error)?;
        let runtime_tree: RuntimeJsonFn = *lib
            .get(b"tracer_core_runtime_tree_json")
            .map_err(symbol_error)?;
        let runtime_crypto_encrypt: RuntimeJsonFn = *lib
            .get(b"tracer_core_runtime_crypto_encrypt_json")
            .map_err(symbol_error)?;
        let runtime_crypto_decrypt: RuntimeJsonFn = *lib
            .get(b"tracer_core_runtime_crypto_decrypt_json")
            .map_err(symbol_error)?;
        let runtime_crypto_inspect: RuntimeJsonFn = *lib
            .get(b"tracer_core_runtime_crypto_inspect_json")
            .map_err(symbol_error)?;

        Ok(RuntimeSymbols {
            set_log_callback,
            set_diagnostics_callback,
            set_crypto_progress_callback,
            runtime_check_environment,
            runtime_resolve_cli_context,
            runtime_create,
            runtime_destroy,
            runtime_ingest,
            runtime_convert,
            runtime_import,
            runtime_validate_structure,
            runtime_validate_logic,
            runtime_query,
            runtime_report,
            runtime_report_batch,
            runtime_export,
            runtime_tree,
            runtime_crypto_encrypt,
            runtime_crypto_decrypt,
            runtime_crypto_inspect,
        })
    }
}

fn symbol_error(error: libloading::Error) -> AppError {
    AppError::DllCompatibility(format!("Load core symbol failed: {error}"))
}
