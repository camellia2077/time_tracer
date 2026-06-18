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
            val hhmm = extractEventTimeToken(lines[index]) ?: continue
            if (parseHhmmToMinutes(hhmm) != null) {
                return hhmm
            }
        }
        return null
    }

    fun isStrictlyAfter(eventTime: String, baselineTime: String): Boolean {
        val eventMinutes = parseHhmmToMinutes(eventTime) ?: return false
        val baselineMinutes = parseHhmmToMinutes(baselineTime) ?: return false
        return eventMinutes > baselineMinutes
    }

    fun extractEventTimeToken(line: String): String? {
        val trimmed = line.trimStart()
        if (trimmed.length < 4) {
            return null
        }
        val firstTime = trimmed.substring(0, 4)
        if (!firstTime.all { it.isDigit() }) {
            return null
        }
        if (
            trimmed.length >= 9 &&
            trimmed[4] == '-' &&
            trimmed.substring(5, 9).all { it.isDigit() }
        ) {
            return trimmed.substring(5, 9)
        }
        return firstTime
    }

    fun extractActivityName(line: String): String {
        val trimmed = line.trimStart()
        if (trimmed.length < 4) {
            return ""
        }

        val firstTime = trimmed.substring(0, 4)
        if (!firstTime.all { it.isDigit() }) {
            return ""
        }

        val bodyOffset = if (
            trimmed.length >= 9 &&
            trimmed[4] == '-' &&
            trimmed.substring(5, 9).all { it.isDigit() }
        ) {
            9
        } else {
            4
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
        return trimmed.length == 4 && trimmed.all { it.isDigit() }
    }

    fun parseHhmmToMinutes(hhmm: String): Int? {
        if (hhmm.length != 4 || !hhmm.all { it.isDigit() }) {
            return null
        }
        val hours = hhmm.substring(0, 2).toIntOrNull() ?: return null
        val minutes = hhmm.substring(2, 4).toIntOrNull() ?: return null
        if (hours !in 0..23 || minutes !in 0..59) {
            return null
        }
        return hours * 60 + minutes
    }
}
