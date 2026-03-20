package com.example.tracer

internal fun formatDurationHoursMinutes(durationSeconds: Long): String {
    val safeSeconds = durationSeconds.coerceAtLeast(0L)
    val totalMinutes = safeSeconds / 60L
    val hours = totalMinutes / 60L
    val minutes = totalMinutes % 60L
    return "${hours}h ${minutes}m"
}

internal fun String.toMonthDayLabel(): String {
    return if (length >= 10) {
        substring(5, 10)
    } else {
        this
    }
}
