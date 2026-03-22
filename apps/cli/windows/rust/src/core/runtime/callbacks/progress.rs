use std::path::Path;

use serde::Deserialize;

#[derive(Default, Deserialize)]
pub(super) struct CryptoProgressEvent {
    #[serde(default)]
    pub(super) operation: String,
    #[serde(default)]
    pub(super) phase: String,
    #[serde(default)]
    pub(super) current_group_label: String,
    #[serde(default)]
    pub(super) current_file_index: usize,
    #[serde(default)]
    pub(super) total_files: usize,
    #[serde(default)]
    pub(super) current_file_done_bytes: u64,
    #[serde(default)]
    pub(super) current_file_total_bytes: u64,
    #[serde(default)]
    pub(super) speed_bytes_per_sec: u64,
    #[serde(default)]
    pub(super) eta_seconds: u64,
    #[serde(default)]
    pub(super) current_input_path: String,
}

pub(super) fn format_progress_line(event: &CryptoProgressEvent) -> String {
    let operation = short_operation(&event.operation);
    let phase = short_phase(&event.phase);
    let file_progress = if event.total_files > 0 {
        format!(
            "{}/{}",
            event.current_file_index.min(event.total_files),
            event.total_files
        )
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
    let mut line =
        format!("[core][crypto] {operation}/{phase} {file_progress} {percent:.0}% {label}");

    if event.speed_bytes_per_sec > 0 {
        line.push_str(&format!(" {}/s", format_bytes(event.speed_bytes_per_sec)));
    }
    if event.eta_seconds > 0 {
        line.push_str(&format!(" eta {}s", event.eta_seconds));
    }

    line
}

pub(super) fn is_terminal_phase(phase: &str) -> bool {
    let normalized = phase.trim().to_ascii_lowercase();
    matches!(normalized.as_str(), "completed" | "cancelled" | "failed")
}

pub(super) fn short_operation(operation: &str) -> &'static str {
    match operation.trim().to_ascii_lowercase().as_str() {
        "encrypt" => "enc",
        "decrypt" => "dec",
        _ => "crypto",
    }
}

pub(super) fn short_phase(phase: &str) -> &'static str {
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

pub(super) fn file_label(event: &CryptoProgressEvent) -> String {
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

pub(super) fn format_bytes(bytes: u64) -> String {
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

#[cfg(test)]
mod tests {
    use super::{
        CryptoProgressEvent, file_label, format_bytes, format_progress_line, is_terminal_phase,
        short_operation, short_phase,
    };

    #[test]
    fn format_bytes_keeps_expected_units() {
        assert_eq!(format_bytes(999), "999 B");
        assert_eq!(format_bytes(1024), "1.0 KB");
        assert_eq!(format_bytes(1024 * 1024), "1.0 MB");
    }

    #[test]
    fn short_operation_maps_known_values() {
        assert_eq!(short_operation("encrypt"), "enc");
        assert_eq!(short_operation("decrypt"), "dec");
        assert_eq!(short_operation("inspect"), "crypto");
    }

    #[test]
    fn short_phase_maps_known_values() {
        assert_eq!(short_phase("read_input"), "read");
        assert_eq!(short_phase("completed"), "done");
        assert_eq!(short_phase("unknown"), "progress");
    }

    #[test]
    fn file_label_prefers_file_name_then_group_label() {
        let event = CryptoProgressEvent {
            current_input_path: "C:/tmp/input/file.tracer".to_string(),
            ..Default::default()
        };
        assert_eq!(file_label(&event), "file.tracer");

        let event = CryptoProgressEvent {
            current_group_label: "group-a".to_string(),
            ..Default::default()
        };
        assert_eq!(file_label(&event), "group-a");
    }

    #[test]
    fn terminal_phase_detection_is_stable() {
        assert!(is_terminal_phase("completed"));
        assert!(is_terminal_phase("cancelled"));
        assert!(!is_terminal_phase("encrypt"));
    }

    #[test]
    fn format_progress_line_includes_progress_details() {
        let event = CryptoProgressEvent {
            operation: "encrypt".to_string(),
            phase: "write_output".to_string(),
            current_file_index: 2,
            total_files: 4,
            current_file_done_bytes: 512,
            current_file_total_bytes: 1024,
            speed_bytes_per_sec: 2048,
            eta_seconds: 3,
            current_input_path: "C:/tmp/input/file.tracer".to_string(),
            ..Default::default()
        };

        let line = format_progress_line(&event);
        assert!(line.contains("[core][crypto] enc/write"));
        assert!(line.contains("2/4"));
        assert!(line.contains("50%"));
        assert!(line.contains("file.tracer"));
        assert!(line.contains("2.0 KB/s"));
        assert!(line.contains("eta 3s"));
    }
}
