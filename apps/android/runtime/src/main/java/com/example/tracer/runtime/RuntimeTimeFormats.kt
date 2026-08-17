package com.example.tracer

private const val ISO_TIME_LENGTH = 8
private const val HOUR_START = 0
private const val MINUTE_START = 3
private const val SECOND_START = 6
private const val MAX_HOUR = 23
private const val MAX_MINUTE_OR_SECOND = 59

@Suppress("ReturnCount")
internal fun normalizeIsoClockTime(rawTime: String): String? {
    val time = rawTime.trim()
    if (time.length != ISO_TIME_LENGTH ||
        time[MINUTE_START - 1] != ':' ||
        time[SECOND_START - 1] != ':'
    ) {
        return null
    }
    if (time.indices.any { index ->
            index != MINUTE_START - 1 && index != SECOND_START - 1 && !time[index].isDigit()
        }
    ) {
        return null
    }

    val hours = time.substring(HOUR_START, MINUTE_START - 1).toIntOrNull() ?: return null
    val minutes = time.substring(MINUTE_START, SECOND_START - 1).toIntOrNull() ?: return null
    val seconds = time.substring(SECOND_START, ISO_TIME_LENGTH).toIntOrNull() ?: return null
    return time.takeIf {
        hours in HOUR_START..MAX_HOUR &&
            minutes in HOUR_START..MAX_MINUTE_OR_SECOND &&
            seconds in HOUR_START..MAX_MINUTE_OR_SECOND
    }
}
