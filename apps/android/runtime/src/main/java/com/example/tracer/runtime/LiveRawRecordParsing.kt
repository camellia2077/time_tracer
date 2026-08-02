package com.example.tracer

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

        for (index in (blockStart + 1) until blockEnd) {
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
        for (index in (blockStart + 1) until blockEnd) {
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
        for (index in (blockEnd - 1) downTo (blockStart + 1)) {
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
        if (trimmed.length < 4) {
            return null
        }
        val timeLength = authoredTimeLength(trimmed) ?: return null
        if (trimmed.length > timeLength && trimmed[timeLength] == '-' &&
            trimmed.length >= (timeLength * 2) + 1 &&
            trimmed.substring(timeLength + 1, (timeLength * 2) + 1).all { it.isDigit() }
        ) {
            return trimmed.substring(timeLength + 1, (timeLength * 2) + 1)
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
        for (index in (blockStart + 1) until lines.size) {
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
        return trimmed.length == 5 &&
            trimmed.first() == 'd' &&
            trimmed.drop(1).all { it.isDigit() }
    }

    fun parseTimeToSeconds(time: String): Int? {
        if ((time.length != 4 && time.length != 6) || !time.all { it.isDigit() }) {
            return null
        }
        val hours = time.substring(0, 2).toIntOrNull() ?: return null
        val minutes = time.substring(2, 4).toIntOrNull() ?: return null
        val seconds = if (time.length == 6) {
            time.substring(4, 6).toIntOrNull() ?: return null
        } else {
            0
        }
        if (hours !in 0..23 || minutes !in 0..59 || seconds !in 0..59) {
            return null
        }
        return (hours * 60 + minutes) * 60 + seconds
    }

    private fun authoredTimeLength(line: String): Int? {
        if (line.length < 4 || !line.substring(0, 4).all { it.isDigit() }) {
            return null
        }
        return if (line.length >= 6 && line.substring(0, 6).all { it.isDigit() }) 6 else 4
    }
}
