package com.example.tracer

import java.io.File
import java.util.Locale

private const val ISO_TIME_LENGTH = 8
private const val HOUR_START = 0
private const val HOUR_END = 2
private const val MINUTE_START = 3
private const val MINUTE_END = 5
private const val SECOND_START = 6

internal class LiveRawRecordPersistence(
    private val parsing: LiveRawRecordParsing
) {
    fun buildRawEventLine(isoTime: String, activity: String, remark: String): String {
        val txtTime = formatTxtTime(isoTime)
        if (remark.isEmpty()) {
            return "$txtTime$activity"
        }
        return "$txtTime$activity // ${encodeRemark(remark)}"
    }

    fun buildRawIntervalEventLine(
        startIsoTime: String,
        endIsoTime: String,
        activity: String,
        remark: String
    ): String {
        val startTxtTime = formatTxtTime(startIsoTime)
        val endTxtTime = formatTxtTime(endIsoTime)
        if (remark.isEmpty()) {
            return "$startTxtTime-$endTxtTime$activity"
        }
        return "$startTxtTime-$endTxtTime$activity // ${encodeRemark(remark)}"
    }

    fun formatTxtTime(isoTime: String): String {
        require(
            isoTime.length == ISO_TIME_LENGTH &&
                isoTime[HOUR_END] == ':' &&
                isoTime[SECOND_START - 1] == ':'
        ) {
            "Time must use ISO HH:mm:ss."
        }
        return isoTime.substring(HOUR_START, HOUR_END) +
            isoTime.substring(MINUTE_START, MINUTE_END) +
            isoTime.substring(SECOND_START, ISO_TIME_LENGTH)
    }

    fun ensureRawMonthFile(monthFile: File, year: Int, month: Int) {
        if (monthFile.exists()) {
            return
        }
        monthFile.parentFile?.mkdirs()
        val lines = mutableListOf<String>()
        lines += "y$year"
        lines += String.format(Locale.US, "m%02d", month)
        lines += ""
        CanonicalTextCodec.writeFile(
            monthFile,
            lines.joinToString(separator = "\n", postfix = "\n")
        )
    }

    @Suppress("LongParameterList")
    fun <T> insertEventIntoDayBlock(
        monthFile: File,
        dayMarker: String,
        eventLine: String,
        eventTime: String,
        normalizedActivity: String,
        resultFactory: (duplicateSuspected: Boolean, firstActivityName: String?) -> T
    ): T {
        val lines = if (monthFile.exists()) {
            monthFile.readLines().toMutableList()
        } else {
            mutableListOf()
        }

        val dayMarkerLine = buildDayMarkerLine(dayMarker)
        val blockStart = lines.indexOfFirst { it.trim() == dayMarkerLine }
        if (blockStart < 0) {
            appendNewDayBlock(lines, dayMarkerLine, eventLine)
            CanonicalTextCodec.writeFile(
                monthFile,
                lines.joinToString(separator = "\n", postfix = "\n")
            )
            return resultFactory(
                false,
                parsing.extractActivityName(eventLine)
            )
        }

        val blockEnd = parsing.findDayBlockEnd(lines, blockStart)
        val duplicateSuspected = parsing.hasDuplicateInDayBlock(
            lines = lines,
            blockStart = blockStart,
            blockEnd = blockEnd,
            eventTime = eventTime,
            activity = normalizedActivity
        )
        val lastEventTime = parsing.findLastValidEventTimeToken(
            lines = lines,
            blockStart = blockStart,
            blockEnd = blockEnd
        )
        check(lastEventTime == null || parsing.isStrictlyAfter(eventTime, lastEventTime)) {
            "Record rejected: new time $eventTime must be later than last event time $lastEventTime in day $dayMarker. Use DAY/ALL editor for backfill edits."
        }

        lines.add(blockEnd, eventLine)
        CanonicalTextCodec.writeFile(
            monthFile,
            lines.joinToString(separator = "\n", postfix = "\n")
        )

        val updatedBlockEnd = parsing.findDayBlockEnd(lines, blockStart)
        val firstActivityName = parsing.findFirstActivityName(lines, blockStart, updatedBlockEnd)
        return resultFactory(
            duplicateSuspected,
            firstActivityName
        )
    }

    private fun appendNewDayBlock(
        lines: MutableList<String>,
        dayMarkerLine: String,
        eventLine: String
    ) {
        if (lines.isNotEmpty() && lines.last().isNotEmpty()) {
            lines += ""
        }
        lines += dayMarkerLine
        lines += eventLine
    }

    private fun encodeRemark(remark: String): String = buildString(remark.length) {
        remark.forEach { character ->
            when (character) {
                '\\' -> append("\\\\")
                '\n' -> append("\\n")
                '\r' -> append("\\n")
                else -> append(character)
            }
        }
    }

    // dayMarker is the Android/API day identity (MMDD); month TXT stores the
    // structural day-block header as dMMDD to disambiguate it from HHMM events.
    private fun buildDayMarkerLine(dayMarker: String): String = "d$dayMarker"
}
