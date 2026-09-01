package com.example.tracer.ui.components

import java.text.SimpleDateFormat
import java.time.LocalTime
import java.time.format.DateTimeFormatter
import java.util.Date
import java.util.Locale

/**
 * Formats user-facing clock values without changing the ISO values used by runtime APIs.
 *
 * A time-only value may arrive with or without seconds, so the 12-hour renderer preserves
 * that precision. Invalid values are returned unchanged so presentation cannot hide the
 * original runtime value behind a fabricated time.
 */
fun formatDisplayClockTime(value: String, use12Hour: Boolean): String {
    if (!use12Hour) return value

    val trimmed = value.trim()
    val parsed = runCatching { LocalTime.parse(trimmed, DateTimeFormatter.ISO_LOCAL_TIME) }
        .getOrNull()
        ?: return value
    val hasSeconds = trimmed.count { it == ':' } >= 2
    val formatter = if (hasSeconds) {
        TWELVE_HOUR_TIME_WITH_SECONDS_FORMATTER
    } else {
        TWELVE_HOUR_TIME_FORMATTER
    }
    return parsed.format(formatter)
}

/** Formats a clock value to minute precision, retaining ISO output when 24-hour mode is active. */
fun formatDisplayClockTimeWithoutSeconds(value: String, use12Hour: Boolean): String {
    if (!use12Hour) return value.take(5)

    val trimmed = value.trim()
    val parsed = runCatching { LocalTime.parse(trimmed, DateTimeFormatter.ISO_LOCAL_TIME) }
        .getOrNull()
        ?: return value
    return parsed.format(TWELVE_HOUR_TIME_FORMATTER)
}

/** Formats the live device-local clock used by the Record screen. */
fun formatDisplayDateTime(epochMillis: Long, use12Hour: Boolean): String {
    val pattern = if (use12Hour) "yyyy-MM-dd h:mm:ss a" else "yyyy-MM-dd HH:mm:ss"
    return SimpleDateFormat(pattern, Locale.US).format(Date(epochMillis))
}

private val TWELVE_HOUR_TIME_FORMATTER: DateTimeFormatter =
    DateTimeFormatter.ofPattern("h:mm a", Locale.US)

private val TWELVE_HOUR_TIME_WITH_SECONDS_FORMATTER: DateTimeFormatter =
    DateTimeFormatter.ofPattern("h:mm:ss a", Locale.US)
