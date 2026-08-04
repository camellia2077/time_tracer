use std::path::Path;

use crate::cli::TxtAppendEventArgs;
use crate::error::AppError;

pub(crate) fn infer_selected_month_from_path(path: &Path) -> String {
    // CLI keeps filename-to-month inference as host policy; runtime txt still
    // receives raw content plus an optional selected_month hint.
    let Some(filename) = path.file_name().and_then(|value| value.to_str()) else {
        return String::new();
    };
    if filename.len() != 11 {
        return String::new();
    }
    let bytes = filename.as_bytes();
    if !filename.ends_with(".txt")
        || bytes[4] != b'-'
        || !bytes[0..4].iter().all(u8::is_ascii_digit)
        || !bytes[5..7].iter().all(u8::is_ascii_digit)
    {
        return String::new();
    }
    filename[..7].to_string()
}

pub(crate) fn build_authored_event_line(args: &TxtAppendEventArgs) -> Result<String, AppError> {
    let mut line = if let Some(time) = &args.time {
        validate_hhmm(time, "--time")?;
        format!("{time}{}", args.activity)
    } else {
        let start = args.start.as_deref().ok_or_else(|| {
            AppError::InvalidArguments("`--start` is required for interval events.".to_string())
        })?;
        let end = args.end.as_deref().ok_or_else(|| {
            AppError::InvalidArguments("`--end` is required for interval events.".to_string())
        })?;
        validate_hhmm(start, "--start")?;
        validate_hhmm(end, "--end")?;
        format!("{start}-{end}{}", args.activity)
    };

    if let Some(remark) = &args.remark {
        if !remark.is_empty() {
            line.push_str(" //");
            line.push_str(remark);
        }
    }
    Ok(line)
}

pub(crate) fn append_event_to_day_body(day_body: &str, event_line: &str) -> String {
    let lines: Vec<&str> = day_body.split('\n').collect();
    let trailing_empty_count = lines
        .iter()
        .rev()
        .take_while(|line| line.is_empty())
        .count();
    let insert_at = lines.len().saturating_sub(trailing_empty_count);

    let mut rebuilt = Vec::with_capacity(lines.len().saturating_add(1));
    rebuilt.extend(lines[..insert_at].iter().copied().map(str::to_string));
    rebuilt.push(event_line.to_string());
    rebuilt.extend(lines[insert_at..].iter().copied().map(str::to_string));
    rebuilt.join("\n")
}

fn validate_hhmm(value: &str, flag_name: &str) -> Result<(), AppError> {
    if value.len() != 4 || !value.as_bytes().iter().all(u8::is_ascii_digit) {
        return Err(AppError::InvalidArguments(format!(
            "{flag_name} expects HHMM digits, got `{value}`."
        )));
    }

    let hour: i32 = value[0..2].parse().map_err(|_| {
        AppError::InvalidArguments(format!("{flag_name} expects HHMM digits, got `{value}`."))
    })?;
    let minute: i32 = value[2..4].parse().map_err(|_| {
        AppError::InvalidArguments(format!("{flag_name} expects HHMM digits, got `{value}`."))
    })?;
    if !(0..24).contains(&hour) || !(0..60).contains(&minute) {
        return Err(AppError::InvalidArguments(format!(
            "{flag_name} expects a valid HHMM time, got `{value}`."
        )));
    }
    Ok(())
}
