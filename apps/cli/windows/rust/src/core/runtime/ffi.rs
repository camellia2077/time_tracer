use std::ffi::{c_char, c_int, c_void};

pub(crate) type RuntimeCheckEnvironmentFn =
    unsafe extern "C" fn(*const c_char, i32) -> *const c_char;
pub(crate) type RuntimeResolveCliContextFn = unsafe extern "C" fn(
    *const c_char,
    *const c_char,
    *const c_char,
    *const c_char,
) -> *const c_char;
pub(crate) type RuntimeCreateFn =
    unsafe extern "C" fn(*const c_char, *const c_char, *const c_char) -> *mut c_void;
pub(crate) type RuntimeDestroyFn = unsafe extern "C" fn(*mut c_void);
pub(crate) type RuntimeJsonFn = unsafe extern "C" fn(*mut c_void, *const c_char) -> *const c_char;
pub(crate) type LogCallbackFn = unsafe extern "C" fn(c_int, *const c_char, *mut c_void);
pub(crate) type DiagnosticsCallbackFn = unsafe extern "C" fn(c_int, *const c_char, *mut c_void);
pub(crate) type CryptoProgressCallbackFn = unsafe extern "C" fn(*const c_char, *mut c_void);
pub(crate) type SetLogCallbackFn = unsafe extern "C" fn(Option<LogCallbackFn>, *mut c_void);
pub(crate) type SetDiagnosticsCallbackFn =
    unsafe extern "C" fn(Option<DiagnosticsCallbackFn>, *mut c_void);
pub(crate) type SetCryptoProgressCallbackFn =
    unsafe extern "C" fn(Option<CryptoProgressCallbackFn>, *mut c_void);

pub(crate) struct RuntimeSymbols {
    pub(crate) set_log_callback: SetLogCallbackFn,
    pub(crate) set_diagnostics_callback: SetDiagnosticsCallbackFn,
    pub(crate) set_crypto_progress_callback: Option<SetCryptoProgressCallbackFn>,
    pub(crate) runtime_check_environment: RuntimeCheckEnvironmentFn,
    pub(crate) runtime_resolve_cli_context: RuntimeResolveCliContextFn,
    pub(crate) runtime_create: RuntimeCreateFn,
    pub(crate) runtime_destroy: RuntimeDestroyFn,
    pub(crate) runtime_ingest: RuntimeJsonFn,
    pub(crate) runtime_convert: RuntimeJsonFn,
    pub(crate) runtime_import: RuntimeJsonFn,
    pub(crate) runtime_validate_structure: RuntimeJsonFn,
    pub(crate) runtime_validate_logic: RuntimeJsonFn,
    pub(crate) runtime_query: RuntimeJsonFn,
    pub(crate) runtime_report: RuntimeJsonFn,
    pub(crate) runtime_report_batch: RuntimeJsonFn,
    pub(crate) runtime_export: RuntimeJsonFn,
    pub(crate) runtime_tree: RuntimeJsonFn,
    pub(crate) runtime_crypto_encrypt: RuntimeJsonFn,
    pub(crate) runtime_crypto_decrypt: RuntimeJsonFn,
    pub(crate) runtime_crypto_inspect: RuntimeJsonFn,
}
