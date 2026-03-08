use std::ffi::{CStr, c_char, c_int, c_void};
use std::io::{self, IsTerminal};

use super::CoreApi;
use super::env_flags::{ENV_CORE_LOG, ENV_CRYPTO_PROGRESS, env_flag};

mod logging;
mod progress;
mod terminal;

pub(crate) fn configure_callbacks(api: &CoreApi) {
    let enable_core_log = env_flag(ENV_CORE_LOG, true);
    let enable_crypto_progress =
        env_flag(ENV_CRYPTO_PROGRESS, true) && io::stderr().is_terminal();

    terminal::configure_dynamic_progress(enable_crypto_progress);

    unsafe {
        if enable_core_log {
            (api.symbols.set_log_callback)(Some(core_log_callback), std::ptr::null_mut());
            (api.symbols.set_diagnostics_callback)(
                Some(core_diagnostics_callback),
                std::ptr::null_mut(),
            );
        } else {
            (api.symbols.set_log_callback)(None, std::ptr::null_mut());
            (api.symbols.set_diagnostics_callback)(None, std::ptr::null_mut());
        }

        if let Some(set_crypto_progress_callback) = api.symbols.set_crypto_progress_callback {
            if enable_crypto_progress {
                set_crypto_progress_callback(Some(core_crypto_progress_callback), std::ptr::null_mut());
            } else {
                set_crypto_progress_callback(None, std::ptr::null_mut());
            }
        }
    }
}

pub(crate) fn finalize_crypto_progress_line() {
    let _ = std::panic::catch_unwind(terminal::finalize_active_progress_line);
}

unsafe extern "C" fn core_log_callback(
    severity: c_int,
    utf8_message: *const c_char,
    _user_data: *mut c_void,
) {
    logging::emit_callback_line("log", severity, utf8_message);
}

unsafe extern "C" fn core_diagnostics_callback(
    severity: c_int,
    utf8_message: *const c_char,
    _user_data: *mut c_void,
) {
    logging::emit_callback_line("diag", severity, utf8_message);
}

unsafe extern "C" fn core_crypto_progress_callback(
    utf8_progress_json: *const c_char,
    _user_data: *mut c_void,
) {
    if utf8_progress_json.is_null() {
        return;
    }

    let _ = std::panic::catch_unwind(|| {
        let payload = unsafe { CStr::from_ptr(utf8_progress_json) }.to_string_lossy();
        let trimmed = payload.trim();
        if trimmed.is_empty() {
            return;
        }

        let event: progress::CryptoProgressEvent = match serde_json::from_str(trimmed) {
            Ok(parsed) => parsed,
            Err(_) => return,
        };

        let line = progress::format_progress_line(&event);
        terminal::render_progress_line(&line);
        if progress::is_terminal_phase(&event.phase) {
            terminal::finalize_active_progress_line();
        }
    });
}
