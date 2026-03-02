use std::ffi::{CStr, c_char, c_int, c_void};
use std::io::{self, IsTerminal, Write};
use std::path::Path;
use std::sync::{Mutex, OnceLock};
use std::{env, mem};
use std::time::{SystemTime, UNIX_EPOCH};

use serde::Deserialize;

use super::CoreApi;
use super::env_flags::{ENV_CORE_LOG, ENV_CRYPTO_PROGRESS, ENV_LOG_TIMESTAMP, env_flag};

const LOG_WARN: c_int = 1;
const LOG_ERROR: c_int = 2;

static PROGRESS_STATE: OnceLock<Mutex<ProgressRenderState>> = OnceLock::new();

#[derive(Default)]
struct ProgressRenderState {
    active_line: bool,
    last_line_width: usize,
    supports_dynamic: bool,
}

#[derive(Default, Deserialize)]
struct CryptoProgressEvent {
    #[serde(default)]
    operation: String,
    #[serde(default)]
    phase: String,
    #[serde(default)]
    current_group_label: String,
    #[serde(default)]
    current_file_index: usize,
    #[serde(default)]
    total_files: usize,
    #[serde(default)]
    current_file_done_bytes: u64,
    #[serde(default)]
    current_file_total_bytes: u64,
    #[serde(default)]
    speed_bytes_per_sec: u64,
    #[serde(default)]
    eta_seconds: u64,
    #[serde(default)]
    current_input_path: String,
}

pub(crate) fn configure_callbacks(api: &CoreApi) {
    let enable_core_log = env_flag(ENV_CORE_LOG, true);
    let enable_crypto_progress =
        env_flag(ENV_CRYPTO_PROGRESS, true) && io::stderr().is_terminal();

    with_progress_state(|state| {
        state.supports_dynamic = enable_crypto_progress;
        if !enable_crypto_progress {
            clear_progress_line_locked(state);
        }
    });

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
    let _ = std::panic::catch_unwind(|| {
        with_progress_state(|state| {
            finish_progress_line_locked(state);
        });
    });
}

unsafe extern "C" fn core_log_callback(
    severity: c_int,
    utf8_message: *const c_char,
    _user_data: *mut c_void,
) {
    emit_callback_line("log", severity, utf8_message);
}

unsafe extern "C" fn core_diagnostics_callback(
    severity: c_int,
    utf8_message: *const c_char,
    _user_data: *mut c_void,
) {
    emit_callback_line("diag", severity, utf8_message);
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

        let event: CryptoProgressEvent = match serde_json::from_str(trimmed) {
            Ok(parsed) => parsed,
            Err(_) => return,
        };

        with_progress_state(|state| {
            if !state.supports_dynamic {
                return;
            }

            let line = format_progress_line(&event);
            render_progress_line_locked(state, &line);
            if is_terminal_phase(&event.phase) {
                finish_progress_line_locked(state);
            }
        });
    });
}

fn emit_callback_line(channel: &str, severity: c_int, utf8_message: *const c_char) {
    if utf8_message.is_null() {
        return;
    }

    let _ = std::panic::catch_unwind(|| {
        let message = unsafe { CStr::from_ptr(utf8_message) }.to_string_lossy();
        let trimmed = message.trim();
        if trimmed.is_empty() {
            return;
        }

        with_progress_state(|state| {
            finish_progress_line_locked(state);
        });

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

        // Keep stdout clean for JSON/text contract parsing; callback logs go to stderr.
        eprintln!("{line}");
    });
}

fn with_progress_state<R>(handle: impl FnOnce(&mut ProgressRenderState) -> R) -> R {
    let mutex = PROGRESS_STATE.get_or_init(|| Mutex::new(ProgressRenderState::default()));
    let mut guard = match mutex.lock() {
        Ok(guard) => guard,
        Err(poisoned) => poisoned.into_inner(),
    };
    handle(&mut guard)
}

fn render_progress_line_locked(state: &mut ProgressRenderState, line: &str) {
    let max_width = detect_terminal_width().unwrap_or(100).saturating_sub(1).max(24);
    let fitted = fit_line_to_width(line, max_width);
    let mut stderr = io::stderr();
    let width = fitted.chars().count();

    let _ = write!(stderr, "\r{fitted}");
    if state.last_line_width > width {
        let padding = " ".repeat(state.last_line_width - width);
        let _ = write!(stderr, "{padding}\r{fitted}");
    }
    let _ = stderr.flush();

    state.active_line = true;
    state.last_line_width = width;
}

fn clear_progress_line_locked(state: &mut ProgressRenderState) {
    if !state.active_line {
        return;
    }

    let mut stderr = io::stderr();
    let clear = " ".repeat(state.last_line_width);
    let _ = write!(stderr, "\r{clear}\r");
    let _ = stderr.flush();

    state.active_line = false;
    state.last_line_width = 0;
}

fn finish_progress_line_locked(state: &mut ProgressRenderState) {
    if !state.active_line {
        return;
    }

    let mut stderr = io::stderr();
    let _ = writeln!(stderr);
    let _ = stderr.flush();

    state.active_line = false;
    state.last_line_width = 0;
}

fn format_progress_line(event: &CryptoProgressEvent) -> String {
    let operation = short_operation(&event.operation);
    let phase = short_phase(&event.phase);
    let file_progress = if event.total_files > 0 {
        format!("{}/{}", event.current_file_index.min(event.total_files), event.total_files)
    } else {
        "-/-".to_string()
    };

    let percent = if event.current_file_total_bytes > 0 {
        (event.current_file_done_bytes as f64 / event.current_file_total_bytes as f64 * 100.0)
            .clamp(0.0, 100.0)
    } else if event.total_files > 0 {
        (event.current_file_index as f64 / event.total_files as f64 * 100.0).clamp(0.0, 100.0)
    } else {
        0.0
    };

    let label = file_label(event);
    let mut line = format!("[core][crypto] {operation}/{phase} {file_progress} {percent:.0}% {label}");

    if event.speed_bytes_per_sec > 0 {
        line.push_str(&format!(" {}/s", format_bytes(event.speed_bytes_per_sec)));
    }
    if event.eta_seconds > 0 {
        line.push_str(&format!(" eta {}s", event.eta_seconds));
    }

    line
}

fn is_terminal_phase(phase: &str) -> bool {
    let normalized = phase.trim().to_ascii_lowercase();
    matches!(normalized.as_str(), "completed" | "cancelled" | "failed")
}

fn short_operation(operation: &str) -> &'static str {
    match operation.trim().to_ascii_lowercase().as_str() {
        "encrypt" => "enc",
        "decrypt" => "dec",
        _ => "crypto",
    }
}

fn short_phase(phase: &str) -> &'static str {
    match phase.trim().to_ascii_lowercase().as_str() {
        "scan" => "scan",
        "read_input" => "read",
        "compress" => "cmp",
        "derive_key" => "kdf",
        "encrypt" => "enc",
        "decrypt" => "dec",
        "decompress" => "dcmp",
        "write_output" => "write",
        "completed" => "done",
        "cancelled" => "cancel",
        "failed" => "fail",
        _ => "progress",
    }
}

fn file_label(event: &CryptoProgressEvent) -> String {
    if !event.current_input_path.trim().is_empty() {
        let path = Path::new(event.current_input_path.trim());
        if let Some(name) = path.file_name().and_then(|value| value.to_str()) {
            if !name.trim().is_empty() {
                return name.to_string();
            }
        }
        return event.current_input_path.trim().to_string();
    }
    if !event.current_group_label.trim().is_empty() {
        return event.current_group_label.trim().to_string();
    }
    "-".to_string()
}

fn format_bytes(bytes: u64) -> String {
    const UNITS: [&str; 5] = ["B", "KB", "MB", "GB", "TB"];
    let mut value = bytes as f64;
    let mut unit_idx = 0usize;
    while value >= 1024.0 && unit_idx + 1 < UNITS.len() {
        value /= 1024.0;
        unit_idx += 1;
    }
    if unit_idx == 0 {
        return format!("{} {}", bytes, UNITS[unit_idx]);
    }
    format!("{value:.1} {}", UNITS[unit_idx])
}

fn fit_line_to_width(text: &str, max_width: usize) -> String {
    if max_width == 0 {
        return String::new();
    }
    let current_width = text.chars().count();
    if current_width <= max_width {
        return text.to_string();
    }

    if max_width <= 3 {
        return ".".repeat(max_width);
    }

    let keep = max_width - 3;
    let mut fitted = String::with_capacity(max_width);
    for ch in text.chars().take(keep) {
        fitted.push(ch);
    }
    fitted.push_str("...");
    fitted
}

fn detect_terminal_width() -> Option<usize> {
    if let Some(from_env) = env::var("COLUMNS")
        .ok()
        .and_then(|raw| raw.trim().parse::<usize>().ok())
        .filter(|value| *value > 0)
    {
        return Some(from_env);
    }

    #[cfg(windows)]
    {
        return detect_terminal_width_windows();
    }

    #[cfg(not(windows))]
    {
        None
    }
}

#[cfg(windows)]
fn detect_terminal_width_windows() -> Option<usize> {
    #[repr(C)]
    struct Coord {
        x: i16,
        y: i16,
    }

    #[repr(C)]
    struct SmallRect {
        left: i16,
        top: i16,
        right: i16,
        bottom: i16,
    }

    #[repr(C)]
    struct ConsoleScreenBufferInfo {
        dw_size: Coord,
        dw_cursor_position: Coord,
        w_attributes: u16,
        sr_window: SmallRect,
        dw_maximum_window_size: Coord,
    }

    #[link(name = "kernel32")]
    unsafe extern "system" {
        fn GetStdHandle(n_std_handle: i32) -> *mut c_void;
        fn GetConsoleScreenBufferInfo(
            h_console_output: *mut c_void,
            lp_console_screen_buffer_info: *mut ConsoleScreenBufferInfo,
        ) -> i32;
    }

    const STD_ERROR_HANDLE: i32 = -12;
    unsafe {
        let handle = GetStdHandle(STD_ERROR_HANDLE);
        if handle.is_null() {
            return None;
        }

        let mut info: ConsoleScreenBufferInfo = mem::zeroed();
        let ok = GetConsoleScreenBufferInfo(handle, &mut info);
        if ok == 0 {
            return None;
        }

        let width = i32::from(info.sr_window.right) - i32::from(info.sr_window.left) + 1;
        if width <= 0 {
            return None;
        }
        Some(width as usize)
    }
}
