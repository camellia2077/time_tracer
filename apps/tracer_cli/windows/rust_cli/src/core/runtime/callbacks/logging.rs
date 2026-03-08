use std::ffi::{CStr, c_char, c_int};
use std::time::{SystemTime, UNIX_EPOCH};

use super::super::env_flags::{ENV_LOG_TIMESTAMP, env_flag};
use super::terminal;

const LOG_WARN: c_int = 1;
const LOG_ERROR: c_int = 2;

pub(super) fn emit_callback_line(channel: &str, severity: c_int, utf8_message: *const c_char) {
    if utf8_message.is_null() {
        return;
    }

    let _ = std::panic::catch_unwind(|| {
        let message = unsafe { CStr::from_ptr(utf8_message) }.to_string_lossy();
        let trimmed = message.trim();
        if trimmed.is_empty() {
            return;
        }

        terminal::finalize_active_progress_line();

        let severity_name = match severity {
            LOG_WARN => "warn",
            LOG_ERROR => "error",
            _ => "info",
        };

        let mut line = String::new();
        if env_flag(ENV_LOG_TIMESTAMP, false) {
            let ts_millis = SystemTime::now()
                .duration_since(UNIX_EPOCH)
                .map(|value| value.as_millis())
                .unwrap_or(0);
            line.push_str(&format!("[ts={ts_millis}] "));
        }
        line.push_str("[core]");
        line.push_str(&format!("[{channel}]"));
        line.push_str(&format!("[{severity_name}] "));
        line.push_str(trimmed);

        eprintln!("{line}");
    });
}
