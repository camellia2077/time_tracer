package com.example.tracer

import java.io.File
import java.util.Locale

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
        require(isoTime.length == 8 && isoTime[2] == ':' && isoTime[5] == ':') {
            "Time must use ISO HH:mm:ss."
        }
        return isoTime.substring(0, 2) + isoTime.substring(3, 5) + isoTime.substring(6, 8)
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
        if (lastEventTime != null && !parsing.isStrictlyAfter(eventTime, lastEventTime)) {
            throw IllegalStateException(
                "Record rejected: new time $eventTime must be later than last event time $lastEventTime in day $dayMarker. Use DAY/ALL editor for backfill edits."
            )
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
