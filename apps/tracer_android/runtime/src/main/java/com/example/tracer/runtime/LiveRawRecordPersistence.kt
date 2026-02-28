package com.example.tracer

import java.io.File
import java.util.Locale

internal class LiveRawRecordPersistence(
    private val parsing: LiveRawRecordParsing
) {
    fun buildRawEventLine(hhmm: String, activity: String, remark: String): String {
        if (remark.isEmpty()) {
            return "$hhmm$activity"
        }
        return "$hhmm$activity // $remark"
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
        monthFile.writeText(lines.joinToString(separator = "\n", postfix = "\n"))
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

        val blockStart = lines.indexOfFirst { it.trim() == dayMarker }
        if (blockStart < 0) {
            appendNewDayBlock(lines, dayMarker, eventLine)
            monthFile.writeText(lines.joinToString(separator = "\n", postfix = "\n"))
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
        monthFile.writeText(lines.joinToString(separator = "\n", postfix = "\n"))

        val updatedBlockEnd = parsing.findDayBlockEnd(lines, blockStart)
        val firstActivityName = parsing.findFirstActivityName(lines, blockStart, updatedBlockEnd)
        return resultFactory(
            duplicateSuspected,
            firstActivityName
        )
    }

    private fun appendNewDayBlock(lines: MutableList<String>, dayMarker: String, eventLine: String) {
        if (lines.isNotEmpty() && lines.last().isNotEmpty()) {
            lines += ""
        }
        lines += dayMarker
        lines += eventLine
    }
}
