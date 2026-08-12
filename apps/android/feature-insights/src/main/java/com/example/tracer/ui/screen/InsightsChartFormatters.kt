package com.example.tracer

internal fun formatDurationHoursMinutes(durationSeconds: Long): String {
    val safeSeconds = durationSeconds.coerceAtLeast(0L)
    val hours = safeSeconds / 3_600L
    val minutes = (safeSeconds % 3_600L) / 60L
    val seconds = safeSeconds % 60L
    return "${hours}h ${minutes}m ${seconds}s"
}

internal fun formatTreemapDurationHoursMinutes(durationSeconds: Long): String {
    val totalMinutes = durationSeconds.coerceAtLeast(0L) / 60L
    val totalHours = totalMinutes / 60L
    val days = totalHours / 24L
    val hours = totalHours % 24L
    val minutes = totalMinutes % 60L
    return if (days > 0L) {
        "${days}d ${hours}h ${minutes}m"
    } else {
        "${hours}h ${minutes}m"
    }
}

internal fun String.toMonthDayLabel(): String {
    return if (length >= 10) {
        substring(5, 10)
    } else {
        this
    }
}
