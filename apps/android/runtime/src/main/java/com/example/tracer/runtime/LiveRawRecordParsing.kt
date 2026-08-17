package com.example.tracer

private const val DAY_BLOCK_CONTENT_OFFSET = 1
private const val SHORT_TIME_LENGTH = 4
private const val LONG_TIME_LENGTH = 6
private const val HOURS_START = 0
private const val MINUTES_START = 2
private const val SECONDS_START = 4
private const val MINUTES_PER_HOUR = 60
private const val SECONDS_PER_MINUTE = 60
private const val MAX_HOUR = 23
private const val MAX_MINUTE_OR_SECOND = 59
private const val DAY_MARKER_LENGTH = 5

internal class LiveRawRecordParsing(
    private val normalization: LiveRawRecordNormalization
) {
    fun hasDuplicateInDayBlock(
        lines: List<String>,
        blockStart: Int,
        blockEnd: Int,
        eventTime: String,
        activity: String
    ): Boolean {
        val normalizedTarget = normalization.normalizeForComparison(activity)
        if (normalizedTarget.isEmpty()) {
            return false
        }

        for (index in (blockStart + DAY_BLOCK_CONTENT_OFFSET) until blockEnd) {
            val lineTime = extractEventTimeToken(lines[index]) ?: continue
            if (lineTime != eventTime) {
                continue
            }

            val lineActivity = extractActivityName(lines[index])
            val normalizedLine = normalization.normalizeForComparison(lineActivity)
            if (normalizedLine == normalizedTarget) {
                return true
            }
        }
        return false
    }

    fun findFirstActivityName(
        lines: List<String>,
        blockStart: Int,
        blockEnd: Int
    ): String? {
        for (index in (blockStart + DAY_BLOCK_CONTENT_OFFSET) until blockEnd) {
            val activity = extractActivityName(lines[index])
            if (activity.isNotEmpty()) {
                return activity
            }
        }
        return null
    }

    fun findLastValidEventTimeToken(
        lines: List<String>,
        blockStart: Int,
        blockEnd: Int
    ): String? {
        for (index in (blockEnd - DAY_BLOCK_CONTENT_OFFSET) downTo (blockStart + DAY_BLOCK_CONTENT_OFFSET)) {
            val time = extractEventTimeToken(lines[index]) ?: continue
            if (parseTimeToSeconds(time) != null) {
                return time
            }
        }
        return null
    }

    fun isStrictlyAfter(eventTime: String, baselineTime: String): Boolean {
        val eventSeconds = parseTimeToSeconds(eventTime) ?: return false
        val baselineSeconds = parseTimeToSeconds(baselineTime) ?: return false
        return eventSeconds > baselineSeconds
    }

    fun extractEventTimeToken(line: String): String? {
        val trimmed = line.trimStart()
        if (trimmed.length < SHORT_TIME_LENGTH) {
            return null
        }
        val timeLength = authoredTimeLength(trimmed) ?: return null
        if (trimmed.length > timeLength && trimmed[timeLength] == '-' &&
            trimmed.length >= (timeLength * 2) + DAY_BLOCK_CONTENT_OFFSET &&
            trimmed.substring(
                timeLength + DAY_BLOCK_CONTENT_OFFSET,
                (timeLength * 2) + DAY_BLOCK_CONTENT_OFFSET
            ).all { it.isDigit() }
        ) {
            return trimmed.substring(
                timeLength + DAY_BLOCK_CONTENT_OFFSET,
                (timeLength * 2) + DAY_BLOCK_CONTENT_OFFSET
            )
        }
        return trimmed.substring(0, timeLength)
    }

    fun extractActivityName(line: String): String {
        val trimmed = line.trimStart()
        val timeLength = authoredTimeLength(trimmed) ?: return ""
        val bodyOffset = if (
            trimmed.length > timeLength && trimmed[timeLength] == '-' &&
            trimmed.length >= (timeLength * 2) + 1 &&
            trimmed.substring(timeLength + 1, (timeLength * 2) + 1).all { it.isDigit() }
        ) {
            (timeLength * 2) + 1
        } else {
            timeLength
        }

        val rawBody = trimmed.substring(bodyOffset).trim()
        if (rawBody.isEmpty()) {
            return ""
        }

        var cutAt = rawBody.length
        val separators = listOf("//", "#", ";")
        for (separator in separators) {
            val index = rawBody.indexOf(separator)
            if (index >= 0 && index < cutAt) {
                cutAt = index
            }
        }
        return rawBody.substring(0, cutAt).trim()
    }

    fun findDayBlockEnd(lines: List<String>, blockStart: Int): Int {
        for (index in (blockStart + DAY_BLOCK_CONTENT_OFFSET) until lines.size) {
            if (isDayMarker(lines[index])) {
                return index
            }
        }
        return lines.size
    }

    fun isDayMarker(line: String): Boolean {
        val trimmed = line.trim()
        // TXT day-block headers use dMMDD so bare HHMM event lines such as 1921
        // never parse as calendar days. Android callers still pass day identity as MMDD.
        return trimmed.length == DAY_MARKER_LENGTH &&
            trimmed.first() == 'd' &&
            trimmed.drop(1).all { it.isDigit() }
    }

    @Suppress("ComplexCondition")
    fun parseTimeToSeconds(time: String): Int? {
        val hasValidLength = time.length == SHORT_TIME_LENGTH || time.length == LONG_TIME_LENGTH
        if (!hasValidLength || !time.all { it.isDigit() }) {
            return null
        }
        val hours = time.substring(HOURS_START, MINUTES_START).toIntOrNull()
        val minutes = time.substring(MINUTES_START, SHORT_TIME_LENGTH).toIntOrNull()
        val seconds = if (time.length == LONG_TIME_LENGTH) {
            time.substring(SECONDS_START, LONG_TIME_LENGTH).toIntOrNull()
        } else {
            HOURS_START
        }
        if (hours == null || minutes == null || seconds == null ||
            hours !in HOURS_START..MAX_HOUR ||
            minutes !in HOURS_START..MAX_MINUTE_OR_SECOND ||
            seconds !in HOURS_START..MAX_MINUTE_OR_SECOND
        ) {
            return null
        }
        return (hours * MINUTES_PER_HOUR + minutes) * SECONDS_PER_MINUTE + seconds
    }

    private fun authoredTimeLength(line: String): Int? {
        if (line.length < SHORT_TIME_LENGTH ||
            !line.substring(HOURS_START, SHORT_TIME_LENGTH).all { it.isDigit() }
        ) {
            return null
        }
        return if (line.length >= LONG_TIME_LENGTH &&
            line.substring(HOURS_START, LONG_TIME_LENGTH).all { it.isDigit() }
        ) {
            LONG_TIME_LENGTH
        } else {
            SHORT_TIME_LENGTH
        }
    }
}
