use std::ffi::{CStr, CString, c_char};
use std::path::Path;

use serde::Deserialize;
use serde_json::Value;

use crate::error::AppError;

pub(crate) fn read_c_json<T: for<'de> Deserialize<'de>>(
    raw: *const c_char,
    context: &str,
) -> Result<T, AppError> {
    if raw.is_null() {
        return Err(AppError::Io(format!("{context}: null response")));
    }
    let text = unsafe { CStr::from_ptr(raw) }
        .to_str()
        .map_err(|e| AppError::Io(format!("{context}: invalid UTF-8 response: {e}")))?;
    serde_json::from_str::<T>(text).map_err(|e| {
        AppError::Io(format!(
            "{context}: invalid JSON response: {e}; payload={text}"
        ))
    })
}

pub(crate) fn path_to_cstring(path: &Path) -> Result<CString, AppError> {
    CString::new(path.to_string_lossy().to_string())
        .map_err(|e| AppError::InvalidArguments(format!("Invalid path string: {e}")))
}

pub(crate) fn option_to_cstring(value: Option<&str>) -> Result<CString, AppError> {
    CString::new(value.unwrap_or(""))
        .map_err(|e| AppError::InvalidArguments(format!("Invalid option string: {e}")))
}

pub(crate) fn to_request_json(request: &Value) -> Result<CString, AppError> {
    let text = serde_json::to_string(request)
        .map_err(|e| AppError::InvalidArguments(format!("Serialize request JSON failed: {e}")))?;
    CString::new(text)
        .map_err(|e| AppError::InvalidArguments(format!("Request JSON contains NUL: {e}")))
}
