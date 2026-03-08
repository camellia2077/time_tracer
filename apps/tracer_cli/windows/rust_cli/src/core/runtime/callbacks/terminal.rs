use std::ffi::{c_int, c_void};
use std::io::{self, Write};
use std::sync::{Mutex, OnceLock};
use std::{env, mem};

static PROGRESS_STATE: OnceLock<Mutex<ProgressRenderState>> = OnceLock::new();

#[derive(Default)]
struct ProgressRenderState {
    active_line: bool,
    last_line_width: usize,
    supports_dynamic: bool,
}

pub(super) fn configure_dynamic_progress(enabled: bool) {
    with_progress_state(|state| {
        state.supports_dynamic = enabled;
        if !enabled {
            clear_progress_line_locked(state);
        }
    });
}

pub(super) fn render_progress_line(line: &str) {
    with_progress_state(|state| {
        if !state.supports_dynamic {
            return;
        }
        render_progress_line_locked(state, line);
    });
}

pub(super) fn finalize_active_progress_line() {
    with_progress_state(finish_progress_line_locked);
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

    const STD_ERROR_HANDLE: c_int = -12;
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

#[cfg(test)]
mod tests {
    use super::fit_line_to_width;

    #[test]
    fn fit_line_to_width_keeps_short_lines() {
        assert_eq!(fit_line_to_width("short", 10), "short");
    }

    #[test]
    fn fit_line_to_width_truncates_with_ellipsis() {
        assert_eq!(fit_line_to_width("abcdefghij", 7), "abcd...");
        assert_eq!(fit_line_to_width("abcdef", 3), "...");
    }

    #[test]
    fn fit_line_to_width_handles_zero_width() {
        assert_eq!(fit_line_to_width("abcdef", 0), "");
    }
}
