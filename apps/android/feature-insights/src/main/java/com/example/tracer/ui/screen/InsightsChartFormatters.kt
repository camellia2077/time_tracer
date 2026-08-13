package com.example.tracer

internal enum class InsightsDurationFormat {
    FULL,
    HOURS_MINUTES,
    COMPACT
}

/** Shared duration renderer for Insights charts, activity records, and timelines. */
internal fun formatInsightsDuration(
    durationSeconds: Long,
    format: InsightsDurationFormat
): String {
    val totalSeconds = durationSeconds.coerceAtLeast(0L)
    val days = totalSeconds / 86_400L
    val remainingSeconds = totalSeconds % 86_400L
    val hours = remainingSeconds / 3_600L
    val minutes = (remainingSeconds % 3_600L) / 60L
    val seconds = remainingSeconds % 60L
    return when (format) {
        InsightsDurationFormat.FULL -> when {
            days > 0L -> "${days}d ${hours}h ${minutes}m ${seconds}s"
            else -> "${hours}h ${minutes}m ${seconds}s"
        }
        InsightsDurationFormat.HOURS_MINUTES -> if (days > 0L) {
            "${days}d ${hours}h ${minutes}m"
        } else {
            "${hours}h ${minutes}m"
        }
        InsightsDurationFormat.COMPACT -> when {
            days > 0L && minutes > 0L -> "${days}d ${hours}h ${minutes}m"
            days > 0L && hours > 0L -> "${days}d ${hours}h"
            days > 0L -> "${days}d"
            hours > 0L -> "${hours}h ${minutes}m"
            minutes > 0L -> "${minutes}m ${seconds}s"
            else -> "${seconds}s"
        }
    }
}

internal fun formatDurationHoursMinutes(durationSeconds: Long): String =
    formatInsightsDuration(durationSeconds, InsightsDurationFormat.FULL)

internal fun formatTreemapDurationHoursMinutes(durationSeconds: Long): String =
    formatInsightsDuration(durationSeconds, InsightsDurationFormat.HOURS_MINUTES)

internal fun String.toMonthDayLabel(): String {
    return if (length >= 10) {
        substring(5, 10)
    } else {
        this
    }
}
