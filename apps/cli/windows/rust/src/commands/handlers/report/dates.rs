use crate::cli::ReportExportPeriod;

use crate::error::AppError;

pub(crate) fn normalize_export_argument(
    period: ReportExportPeriod,
    argument: &str,
) -> Result<String, AppError> {
    match period {
        ReportExportPeriod::Day => normalize_day_argument("`report export day`", argument),
        ReportExportPeriod::Month => normalize_month_argument("`report export month`", argument),
        ReportExportPeriod::Week => normalize_week_argument("`report export week`", argument),
        ReportExportPeriod::Year => normalize_year_argument("`report export year`", argument),
        ReportExportPeriod::Recent => normalize_recent_argument("`report export recent`", argument),
        ReportExportPeriod::Range => normalize_range_argument("`report export range`", argument),
    }
}

pub(crate) fn normalize_day_argument(command_label: &str, value: &str) -> Result<String, AppError> {
    let normalized = if value.len() == 8 && value.chars().all(|ch| ch.is_ascii_digit()) {
        format!("{}-{}-{}", &value[..4], &value[4..6], &value[6..8])
    } else if value.len() == 10
        && value.as_bytes()[4] == b'-'
        && value.as_bytes()[7] == b'-'
        && value
            .chars()
            .enumerate()
            .all(|(index, ch)| matches!(index, 4 | 7) || ch.is_ascii_digit())
    {
        value.to_string()
    } else {
        return Err(AppError::InvalidArguments(format!(
            "{command_label} expects YYYYMMDD or YYYY-MM-DD, got `{value}`."
        )));
    };

    // Semantic validation: reject impossible dates like 2026-02-30.
    let year: u32 = normalized[..4].parse().unwrap_or(0);
    let month: u32 = normalized[5..7].parse().unwrap_or(0);
    let day: u32 = normalized[8..10].parse().unwrap_or(0);
    let max_day = match month {
        1 | 3 | 5 | 7 | 8 | 10 | 12 => 31,
        4 | 6 | 9 | 11 => 30,
        2 => {
            if (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0) {
                29
            } else {
                28
            }
        }
        _ => 0,
    };
    if month < 1 || month > 12 || day < 1 || day > max_day {
        return Err(AppError::InvalidArguments(format!(
            "{command_label} expects YYYYMMDD or YYYY-MM-DD, got `{value}`."
        )));
    }

    Ok(normalized)
}

pub(crate) fn parse_iso_date_to_serial_days(iso_date: &str) -> Result<i64, AppError> {
    if iso_date.len() != 10 || iso_date.as_bytes()[4] != b'-' || iso_date.as_bytes()[7] != b'-' {
        return Err(AppError::InvalidArguments(format!(
            "Expected ISO date YYYY-MM-DD, got `{iso_date}`."
        )));
    }

    let year = iso_date[..4].parse::<i32>().map_err(|error| {
        AppError::InvalidArguments(format!("Invalid ISO year in `{iso_date}`: {error}"))
    })?;
    let month = iso_date[5..7].parse::<u32>().map_err(|error| {
        AppError::InvalidArguments(format!("Invalid ISO month in `{iso_date}`: {error}"))
    })?;
    let day = iso_date[8..10].parse::<u32>().map_err(|error| {
        AppError::InvalidArguments(format!("Invalid ISO day in `{iso_date}`: {error}"))
    })?;

    let month_i32 = i32::try_from(month)
        .map_err(|_| AppError::InvalidArguments(format!("Invalid ISO month in `{iso_date}`.")))?;
    let day_i32 = i32::try_from(day)
        .map_err(|_| AppError::InvalidArguments(format!("Invalid ISO day in `{iso_date}`.")))?;

    // Howard Hinnant civil-date conversion: days since 1970-01-01.
    let year_adj = year - if month <= 2 { 1 } else { 0 };
    let era = if year_adj >= 0 {
        year_adj
    } else {
        year_adj - 399
    } / 400;
    let yoe = year_adj - era * 400;
    let month_prime = month_i32 + if month > 2 { -3 } else { 9 };
    let doy = (153 * month_prime + 2) / 5 + day_i32 - 1;
    let doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    Ok(i64::from(era) * 146_097 + i64::from(doe) - 719_468)
}

pub(crate) fn format_serial_days_to_iso_date(serial_days: i64) -> Result<String, AppError> {
    // Inverse Howard Hinnant civil-date conversion.
    let z = serial_days + 719_468;
    let era = if z >= 0 { z } else { z - 146_096 } / 146_097;
    let doe = z - era * 146_097; // [0, 146096]
    let yoe = (doe - doe / 1_460 + doe / 36_524 - doe / 146_096) / 365; // [0,399]
    let y = yoe + era * 400;
    let doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    let mp = (5 * doy + 2) / 153; // [0,11]
    let d = doy - (153 * mp + 2) / 5 + 1; // [1,31]
    let m = mp + if mp < 10 { 3 } else { -9 }; // [1,12]
    let year = y + if m <= 2 { 1 } else { 0 };

    let month_u32 = u32::try_from(m).map_err(|_| {
        AppError::InvalidArguments(format!(
            "Failed to convert computed month `{m}` to ISO date."
        ))
    })?;
    let day_u32 = u32::try_from(d).map_err(|_| {
        AppError::InvalidArguments(format!("Failed to convert computed day `{d}` to ISO date."))
    })?;
    let year_i32 = i32::try_from(year).map_err(|_| {
        AppError::InvalidArguments(format!(
            "Failed to convert computed year `{year}` to ISO date."
        ))
    })?;

    Ok(format!("{year_i32:04}-{month_u32:02}-{day_u32:02}"))
}

pub(crate) fn normalize_month_argument(
    command_label: &str,
    value: &str,
) -> Result<String, AppError> {
    if value.len() == 6 && value.chars().all(|ch| ch.is_ascii_digit()) {
        return Ok(format!("{}-{}", &value[..4], &value[4..6]));
    }
    if value.len() == 7
        && value.as_bytes()[4] == b'-'
        && value
            .chars()
            .enumerate()
            .all(|(index, ch)| index == 4 || ch.is_ascii_digit())
    {
        return Ok(value.to_string());
    }
    Err(AppError::InvalidArguments(format!(
        "{command_label} expects YYYYMM or YYYY-MM, got `{value}`."
    )))
}

pub(crate) fn month_to_range(
    command_label: &str,
    value: &str,
) -> Result<(String, String), AppError> {
    let month = normalize_month_argument(command_label, value)?;
    let year: u32 = month[..4].parse().unwrap_or(0);
    let month_number: u32 = month[5..7].parse().unwrap_or(0);
    let last_day = match month_number {
        1 | 3 | 5 | 7 | 8 | 10 | 12 => 31,
        4 | 6 | 9 | 11 => 30,
        2 => {
            if (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0) {
                29
            } else {
                28
            }
        }
        _ => {
            return Err(AppError::InvalidArguments(format!(
                "{command_label} expects YYYYMM or YYYY-MM, got `{value}`."
            )));
        }
    };
    Ok((format!("{month}-01"), format!("{month}-{last_day:02}")))
}

pub(crate) fn normalize_week_argument(
    command_label: &str,
    value: &str,
) -> Result<String, AppError> {
    if value.len() == 8
        && value.as_bytes()[4] == b'-'
        && value.as_bytes()[5] == b'W'
        && value[..4].chars().all(|ch| ch.is_ascii_digit())
        && value[6..].chars().all(|ch| ch.is_ascii_digit())
    {
        return Ok(value.to_string());
    }
    Err(AppError::InvalidArguments(format!(
        "{command_label} expects YYYY-Www, got `{value}`."
    )))
}

pub(crate) fn week_to_range(
    command_label: &str,
    value: &str,
) -> Result<(String, String), AppError> {
    let week = normalize_week_argument(command_label, value)?;
    let iso_year = week[..4].parse::<i32>().map_err(|error| {
        AppError::InvalidArguments(format!(
            "{command_label} expects YYYY-Www, got `{value}`: {error}"
        ))
    })?;
    let iso_week = week[6..8].parse::<u32>().map_err(|error| {
        AppError::InvalidArguments(format!(
            "{command_label} expects YYYY-Www, got `{value}`: {error}"
        ))
    })?;
    if !(1..=53).contains(&iso_week) {
        return Err(AppError::InvalidArguments(format!(
            "{command_label} expects YYYY-Www, got `{value}`."
        )));
    }

    let january_fourth = parse_iso_date_to_serial_days(&format!("{iso_year:04}-01-04"))?;
    let monday_based_weekday = ((january_fourth + 3).rem_euclid(7)) + 1;
    let week_one_monday = january_fourth - (monday_based_weekday - 1);
    let start_days = week_one_monday + i64::from((iso_week - 1) * 7);
    let end_days = start_days + 6;
    Ok((
        format_serial_days_to_iso_date(start_days)?,
        format_serial_days_to_iso_date(end_days)?,
    ))
}

pub(crate) fn normalize_year_argument(
    command_label: &str,
    value: &str,
) -> Result<String, AppError> {
    if value.len() == 4 && value.chars().all(|ch| ch.is_ascii_digit()) {
        return Ok(value.to_string());
    }
    Err(AppError::InvalidArguments(format!(
        "{command_label} expects a 4-digit year, got `{value}`."
    )))
}

pub(crate) fn normalize_recent_argument(
    command_label: &str,
    value: &str,
) -> Result<String, AppError> {
    let days = value.parse::<i32>().map_err(|error| {
        AppError::InvalidArguments(format!(
            "{command_label} expects a positive integer, got `{value}`: {error}"
        ))
    })?;
    if days <= 0 {
        return Err(AppError::InvalidArguments(format!(
            "{command_label} expects a positive integer, got `{value}`."
        )));
    }
    Ok(days.to_string())
}

pub(crate) fn normalize_range_argument(
    command_label: &str,
    value: &str,
) -> Result<String, AppError> {
    let Some((start, end)) = value.split_once('|') else {
        return Err(AppError::InvalidArguments(format!(
            "{command_label} expects `start|end`."
        )));
    };
    if end.contains('|') {
        return Err(AppError::InvalidArguments(format!(
            "{command_label} expects exactly one `|` separator."
        )));
    }

    let normalized_start = normalize_day_argument(command_label, start.trim())?;
    let normalized_end = normalize_day_argument(command_label, end.trim())?;
    if normalized_start > normalized_end {
        return Err(AppError::InvalidArguments(format!(
            "{command_label} expects start <= end."
        )));
    }

    Ok(format!("{normalized_start}|{normalized_end}"))
}

pub(crate) fn split_normalized_range(
    command_label: &str,
    value: &str,
) -> Result<(String, String), AppError> {
    let normalized = normalize_range_argument(command_label, value)?;
    let (start_date, end_date) = normalized.split_once('|').ok_or_else(|| {
        AppError::InvalidArguments(format!("{command_label} expects `start|end`."))
    })?;
    Ok((start_date.to_string(), end_date.to_string()))
}
