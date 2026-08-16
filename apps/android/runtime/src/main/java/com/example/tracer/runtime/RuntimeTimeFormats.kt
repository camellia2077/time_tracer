package com.example.tracer

internal fun normalizeIsoClockTime(rawTime: String): String? {
    val time = rawTime.trim()
    if (time.length != 8 || time[2] != ':' || time[5] != ':' ||
        time.indices.any { index -> index != 2 && index != 5 && !time[index].isDigit() }
    ) {
        return null
    }

    val hours = time.substring(0, 2).toIntOrNull() ?: return null
    val minutes = time.substring(3, 5).toIntOrNull() ?: return null
    val seconds = time.substring(6, 8).toIntOrNull() ?: return null
    return time.takeIf {
        hours in 0..23 && minutes in 0..59 && seconds in 0..59
    }
}
