package com.example.tracer

import java.io.File
import java.text.SimpleDateFormat
import java.util.Calendar
import java.util.Date
import java.util.Locale

internal class LiveRawRecordStore {
    private val appMonthFileFormatter = SimpleDateFormat("yyyy-MM", Locale.US)
    private val dayMarkerFormatter = SimpleDateFormat("MMdd", Locale.US)
    private val hhmmFormatter = SimpleDateFormat("HHmm", Locale.US)
    private val normalization = InputNormalization()
    private val parsing = RecordParsing(normalization)
    private val persistence = RecordPersistence(parsing)

    fun ensureCurrentMonthFile(liveRawInputPath: String): EnsureMonthFileResult {
        val cal = Calendar.getInstance()
        return ensureMonthFile(
            liveRawInputPath = liveRawInputPath,
            year = cal.get(Calendar.YEAR),
            month = cal.get(Calendar.MONTH) + 1
        )
    }

    fun ensureMonthFile(
        liveRawInputPath: String,
        year: Int,
        month: Int
    ): EnsureMonthFileResult {
        require(month in 1..12) { "Month must be between 1 and 12." }
        val monthFileName = String.format(Locale.US, "%04d-%02d.txt", year, month)
        val monthFile = File(liveRawInputPath, monthFileName)
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
        liveRawInputPath: String,
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
            liveRawInputPath = liveRawInputPath,
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
        if (
            insertResult.firstActivityName != null &&
            !parsing.isWakeLikeActivity(insertResult.firstActivityName)
        ) {
            warnings += "First entry of the day is not wake-related."
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
        liveRawInputPath: String,
        logicalDate: Date,
        preferredRelativePath: String?
    ): File {
        if (!preferredRelativePath.isNullOrBlank()) {
            val root = File(liveRawInputPath).canonicalFile
            val candidate = File(root, preferredRelativePath.trim()).canonicalFile
            val relative = candidate.relativeToOrNull(root)
            if (relative != null &&
                candidate.extension.equals("txt", ignoreCase = true)
            ) {
                return candidate
            }
        }

        val monthFileName = "${appMonthFileFormatter.format(logicalDate)}.txt"
        return File(liveRawInputPath, monthFileName)
    }

    fun listTxtFiles(liveRawInputPath: String): TxtHistoryListResult {
        val root = File(liveRawInputPath)
        if (!root.exists()) {
            return TxtHistoryListResult(
                ok = true,
                files = emptyList(),
                message = "No live TXT directory."
            )
        }

        val files = root.walkTopDown()
            .filter { it.isFile && it.extension.equals("txt", ignoreCase = true) }
            .map { it.relativeTo(root).invariantSeparatorsPath }
            .sorted()
            .toList()

        return TxtHistoryListResult(
            ok = true,
            files = files,
            message = "Found ${files.size} TXT file(s)."
        )
    }

    fun readTxtFile(liveRawInputPath: String, relativePath: String): TxtFileContentResult {
        val requested = relativePath.trim()
        if (requested.isEmpty()) {
            return TxtFileContentResult(
                ok = false,
                filePath = "",
                content = "",
                message = "TXT file path is empty."
            )
        }

        val root = File(liveRawInputPath).canonicalFile
        val target = File(root, requested).canonicalFile
        val relative = target.relativeToOrNull(root)
        if (relative == null) {
            return TxtFileContentResult(
                ok = false,
                filePath = requested,
                content = "",
                message = "TXT path is outside live input root."
            )
        }
        if (!target.exists() || !target.isFile) {
            return TxtFileContentResult(
                ok = false,
                filePath = requested,
                content = "",
                message = "TXT file not found."
            )
        }

        return TxtFileContentResult(
            ok = true,
            filePath = relative.invariantSeparatorsPath,
            content = target.readText(),
            message = "Read TXT success."
        )
    }

    fun writeTxtFile(liveRawInputPath: String, relativePath: String, content: String): TxtFileContentResult {
        val requested = relativePath.trim()
        if (requested.isEmpty()) {
            return TxtFileContentResult(
                ok = false,
                filePath = "",
                content = "",
                message = "TXT file path is empty."
            )
        }

        val root = File(liveRawInputPath).canonicalFile
        val target = File(root, requested).canonicalFile
        val relative = target.relativeToOrNull(root)
        if (relative == null) {
            return TxtFileContentResult(
                ok = false,
                filePath = requested,
                content = "",
                message = "TXT path is outside live input root."
            )
        }
        if (!target.exists() || !target.isFile) {
            return TxtFileContentResult(
                ok = false,
                filePath = requested,
                content = "",
                message = "TXT file not found."
            )
        }

        target.writeText(content)
        return TxtFileContentResult(
            ok = true,
            filePath = relative.invariantSeparatorsPath,
            content = content,
            message = "Save TXT success."
        )
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

    private class InputNormalization {
        fun normalizeActivityName(raw: String): String {
            val cleaned = raw.replace("\n", " ").replace("\r", " ").trim()
            if (cleaned.isEmpty()) {
                return ""
            }
            return when (cleaned.lowercase(Locale.US)) {
                "zh" -> "zhihu"
                else -> cleaned
            }
        }

        fun normalizeRemark(raw: String): String {
            return raw.replace("\n", " ").replace("\r", " ").trim()
        }

        fun normalizeForComparison(value: String): String {
            return value.trim()
                .lowercase(Locale.US)
                .replace("\\s+".toRegex(), "")
        }
    }

    private class RecordParsing(
        private val normalization: InputNormalization
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
            val hhmm = trimmed.substring(0, 4)
            if (!hhmm.all { it.isDigit() }) {
                return null
            }
            return hhmm
        }

        fun extractActivityName(line: String): String {
            val trimmed = line.trimStart()
            if (trimmed.length < 4) {
                return ""
            }

            val hhmm = trimmed.substring(0, 4)
            if (!hhmm.all { it.isDigit() }) {
                return ""
            }

            val rawBody = trimmed.substring(4).trim()
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

        fun isWakeLikeActivity(activityName: String): Boolean {
            val normalized = normalization.normalizeForComparison(activityName)
            if (normalized.isEmpty()) {
                return false
            }

            return normalized == "w" ||
                normalized.contains("wake") ||
                normalized.contains("起床") ||
                normalized.contains("新的一天开始了")
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

    private class RecordPersistence(
        private val parsing: RecordParsing
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

        fun appendNewDayBlock(lines: MutableList<String>, dayMarker: String, eventLine: String) {
            if (lines.isNotEmpty() && lines.last().isNotEmpty()) {
                lines += ""
            }
            lines += dayMarker
            lines += eventLine
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
    }
}
