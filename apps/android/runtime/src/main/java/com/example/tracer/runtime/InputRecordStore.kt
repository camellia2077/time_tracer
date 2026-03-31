package com.example.tracer

import java.io.File
import java.text.SimpleDateFormat
import java.util.Calendar
import java.util.Date
import java.util.Locale

internal class InputRecordStore {
    private val appMonthFileFormatter = SimpleDateFormat("yyyy-MM", Locale.US)
    private val dayMarkerFormatter = SimpleDateFormat("MMdd", Locale.US)
    private val hhmmFormatter = SimpleDateFormat("HHmm", Locale.US)
    private val txtFileStore = InputTxtFileStore()
    private val normalization = LiveRawRecordNormalization()
    private val parsing = LiveRawRecordParsing(normalization)
    private val persistence = LiveRawRecordPersistence(parsing)

    fun ensureCurrentMonthFile(inputRootPath: String): EnsureMonthFileResult {
        val cal = Calendar.getInstance()
        return ensureMonthFile(
            inputRootPath = inputRootPath,
            year = cal.get(Calendar.YEAR),
            month = cal.get(Calendar.MONTH) + 1
        )
    }

    fun ensureMonthFile(
        inputRootPath: String,
        year: Int,
        month: Int
    ): EnsureMonthFileResult {
        require(month in 1..12) { "Month must be between 1 and 12." }
        val monthFile = File(inputRootPath, buildMonthRelativePath(String.format(Locale.US, "%04d-%02d", year, month)))
        val created = !monthFile.exists()
        ensureRawMonthFile(monthFile, year, month)
        return EnsureMonthFileResult(monthFile = monthFile, created = created)
    }

    fun normalizeActivityName(raw: String): String {
        return normalization.normalizeActivityName(raw)
    }

    fun normalizeRemark(raw: String): String {
        return normalization.normalizeRemark(raw)
    }

    fun appendRecord(
        inputRootPath: String,
        logicalDateString: String, // Expected YYYY-MM-DD
        activityName: String,
        remark: String,
        preferredRelativePath: String? = null
    ): RecordWriteSnapshot {
        val now = Date()
        val logicalDate = SimpleDateFormat("yyyy-MM-dd", Locale.US).parse(logicalDateString)
            ?: throw IllegalArgumentException("Invalid logicalDate format: $logicalDateString")

        val dayMarker = dayMarkerFormatter.format(logicalDate)
        val eventTime = hhmmFormatter.format(now)
        val monthFile = resolveTargetMonthFile(
            inputRootPath = inputRootPath,
            logicalDate = logicalDate,
            preferredRelativePath = preferredRelativePath
        )
        val logicalCal = Calendar.getInstance().apply { time = logicalDate }
        ensureRawMonthFile(
            monthFile = monthFile,
            year = logicalCal.get(Calendar.YEAR),
            month = logicalCal.get(Calendar.MONTH) + 1
        )

        val eventLine = buildRawEventLine(eventTime, activityName, remark)
        val insertResult = insertEventIntoDayBlock(
            monthFile = monthFile,
            dayMarker = dayMarker,
            eventLine = eventLine,
            eventTime = eventTime,
            normalizedActivity = activityName
        )

        val warnings = mutableListOf<String>()
        if (insertResult.duplicateSuspected) {
            warnings += "Possible duplicate record: same HHmm and activity already exists in this day."
        }
        resolveCompletenessWarningForDayContent(
            content = monthFile.readText(),
            dayMarker = dayMarker,
            wakeKeywords = emptySet()
        )?.let { warning ->
            warnings += warning
        }

        return RecordWriteSnapshot(
            monthFile = monthFile,
            logicalDate = logicalDateString,
            dayMarker = dayMarker,
            eventTime = eventTime,
            warnings = warnings
        )
    }

    private fun resolveTargetMonthFile(
        inputRootPath: String,
        logicalDate: Date,
        preferredRelativePath: String?
    ): File {
        if (!preferredRelativePath.isNullOrBlank()) {
            val root = File(inputRootPath).canonicalFile
            val candidate = File(root, preferredRelativePath.trim()).canonicalFile
            val relative = candidate.relativeToOrNull(root)
            if (relative != null &&
                candidate.extension.equals("txt", ignoreCase = true)
            ) {
                return candidate
            }
        }

        return File(inputRootPath, buildMonthRelativePath(appMonthFileFormatter.format(logicalDate)))
    }

    fun listTxtFiles(inputRootPath: String): TxtHistoryListResult {
        return txtFileStore.listTxtFiles(inputRootPath)
    }

    fun readTxtFile(inputRootPath: String, relativePath: String): TxtFileContentResult {
        return txtFileStore.readTxtFile(inputRootPath, relativePath)
    }

    fun writeTxtFile(inputRootPath: String, relativePath: String, content: String): TxtFileContentResult {
        return txtFileStore.writeTxtFile(inputRootPath, relativePath, content)
    }

    fun resolveCompletenessWarningForMonthContent(
        content: String,
        wakeKeywords: Set<String> = emptySet()
    ): String? {
        val lines = content.lineSequence().toList()
        var hasIncompleteDay = false
        val normalizedWakeKeywords = wakeKeywords
            .asSequence()
            .map(normalization::normalizeForComparison)
            .filter { it.isNotEmpty() }
            .toSet()

        for (index in lines.indices) {
            if (!parsing.isDayMarker(lines[index])) {
                continue
            }

            val blockEnd = parsing.findDayBlockEnd(lines, index)
            when (
                resolveCompletenessWarningForDayBlock(
                    lines = lines,
                    blockStart = index,
                    blockEnd = blockEnd,
                    normalizedWakeKeywords = normalizedWakeKeywords
                )
            ) {
                WarningKind.OVERNIGHT_CONTINUATION -> return OVERNIGHT_CONTINUATION_WARNING
                WarningKind.INCOMPLETE_DAY -> hasIncompleteDay = true
                WarningKind.NONE -> Unit
            }
        }

        return if (hasIncompleteDay) INCOMPLETE_DAY_WARNING else null
    }

    fun resolveCompletenessWarningForDayContent(
        content: String,
        dayMarker: String,
        wakeKeywords: Set<String> = emptySet()
    ): String? {
        val lines = content.lineSequence().toList()
        val blockStart = lines.indexOfFirst { it.trim() == dayMarker }
        if (blockStart < 0 || !parsing.isDayMarker(lines[blockStart])) {
            return null
        }
        val normalizedWakeKeywords = wakeKeywords
            .asSequence()
            .map(normalization::normalizeForComparison)
            .filter { it.isNotEmpty() }
            .toSet()

        return when (
            resolveCompletenessWarningForDayBlock(
                lines = lines,
                blockStart = blockStart,
                blockEnd = parsing.findDayBlockEnd(lines, blockStart),
                normalizedWakeKeywords = normalizedWakeKeywords
            )
        ) {
            WarningKind.OVERNIGHT_CONTINUATION -> OVERNIGHT_CONTINUATION_WARNING
            WarningKind.INCOMPLETE_DAY -> INCOMPLETE_DAY_WARNING
            WarningKind.NONE -> null
        }
    }

    private fun buildRawEventLine(hhmm: String, activity: String, remark: String): String {
        return persistence.buildRawEventLine(hhmm, activity, remark)
    }

    private fun ensureRawMonthFile(monthFile: File, year: Int, month: Int) {
        persistence.ensureRawMonthFile(monthFile, year, month)
    }

    private fun insertEventIntoDayBlock(
        monthFile: File,
        dayMarker: String,
        eventLine: String,
        eventTime: String,
        normalizedActivity: String
    ): InsertEventResult {
        return persistence.insertEventIntoDayBlock(
            monthFile = monthFile,
            dayMarker = dayMarker,
            eventLine = eventLine,
            eventTime = eventTime,
            normalizedActivity = normalizedActivity,
            resultFactory = { duplicateSuspected, firstActivityName ->
                InsertEventResult(
                    duplicateSuspected = duplicateSuspected,
                    firstActivityName = firstActivityName
                )
            }
        )
    }

    private data class InsertEventResult(
        val duplicateSuspected: Boolean,
        val firstActivityName: String?
    )

    private fun resolveCompletenessWarningForDayBlock(
        lines: List<String>,
        blockStart: Int,
        blockEnd: Int,
        normalizedWakeKeywords: Set<String>
    ): WarningKind {
        var authoredEventCount = 0
        val firstActivityName = parsing.findFirstActivityName(lines, blockStart, blockEnd)
        for (index in (blockStart + 1) until blockEnd) {
            if (parsing.extractActivityName(lines[index]).isNotEmpty()) {
                authoredEventCount += 1
            }
        }

        if (authoredEventCount >= 2) {
            return WarningKind.NONE
        }
        val normalizedFirstActivity = firstActivityName
            ?.let(normalization::normalizeForComparison)
            ?.takeIf { it.isNotEmpty() }
        if (normalizedFirstActivity != null &&
            normalizedWakeKeywords.isNotEmpty() &&
            !normalizedWakeKeywords.contains(normalizedFirstActivity)
        ) {
            return WarningKind.OVERNIGHT_CONTINUATION
        }
        return WarningKind.INCOMPLETE_DAY
    }

    private enum class WarningKind {
        NONE,
        INCOMPLETE_DAY,
        OVERNIGHT_CONTINUATION
    }

    internal companion object {
        const val INCOMPLETE_DAY_WARNING =
            "Warning: this day currently has fewer than 2 authored events, so some intervals may not be computable yet."
        const val OVERNIGHT_CONTINUATION_WARNING =
            "Warning: possible overnight continuation; the first event of this day is not wake-related, so no sleep activity will be auto-generated."
    }
}
