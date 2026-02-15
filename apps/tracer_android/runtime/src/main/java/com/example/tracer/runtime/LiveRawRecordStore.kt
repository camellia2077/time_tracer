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
        ensureRawMonthFile(monthFile, year)
        return EnsureMonthFileResult(monthFile = monthFile, created = created)
    }

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

    fun appendRecord(
        liveRawInputPath: String,
        logicalDateString: String, // Expected YYYY-MM-DD
        activityName: String,
        remark: String
    ): RecordWriteSnapshot {
        val now = Date()
        val logicalDate = SimpleDateFormat("yyyy-MM-dd", Locale.US).parse(logicalDateString)
            ?: throw IllegalArgumentException("Invalid logicalDate format: $logicalDateString")

        val monthFileName = "${appMonthFileFormatter.format(logicalDate)}.txt"
        val dayMarker = dayMarkerFormatter.format(logicalDate)
        val eventTime = hhmmFormatter.format(now)

        val monthFile = File(liveRawInputPath, monthFileName)
        val logicalCal = Calendar.getInstance().apply { time = logicalDate }
        ensureRawMonthFile(monthFile, logicalCal.get(Calendar.YEAR))

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
        if (insertResult.firstActivityName != null && !isWakeLikeActivity(insertResult.firstActivityName)) {
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
        if (remark.isEmpty()) {
            return "$hhmm$activity"
        }
        return "$hhmm$activity // $remark"
    }

    private fun ensureRawMonthFile(monthFile: File, year: Int) {
        if (monthFile.exists()) {
            return
        }
        monthFile.parentFile?.mkdirs()
        val lines = mutableListOf<String>()
        lines += "y$year"
        lines += ""
        monthFile.writeText(lines.joinToString(separator = "\n", postfix = "\n"))
    }

    private fun insertEventIntoDayBlock(
        monthFile: File,
        dayMarker: String,
        eventLine: String,
        eventTime: String,
        normalizedActivity: String
    ): InsertEventResult {
        val lines = if (monthFile.exists()) {
            monthFile.readLines().toMutableList()
        } else {
            mutableListOf()
        }

        val blockStart = lines.indexOfFirst { it.trim() == dayMarker }
        if (blockStart < 0) {
            appendNewDayBlock(lines, dayMarker, eventLine)
            monthFile.writeText(lines.joinToString(separator = "\n", postfix = "\n"))
            return InsertEventResult(
                duplicateSuspected = false,
                firstActivityName = extractActivityName(eventLine)
            )
        }

        val blockEnd = findDayBlockEnd(lines, blockStart)
        val duplicateSuspected = hasDuplicateInDayBlock(
            lines = lines,
            blockStart = blockStart,
            blockEnd = blockEnd,
            eventTime = eventTime,
            activity = normalizedActivity
        )
        val insertAt = findInsertionIndex(lines, blockStart, blockEnd, eventLine)
        lines.add(insertAt, eventLine)
        monthFile.writeText(lines.joinToString(separator = "\n", postfix = "\n"))

        val updatedBlockEnd = findDayBlockEnd(lines, blockStart)
        val firstActivityName = findFirstActivityName(lines, blockStart, updatedBlockEnd)
        return InsertEventResult(
            duplicateSuspected = duplicateSuspected,
            firstActivityName = firstActivityName
        )
    }

    private data class InsertEventResult(
        val duplicateSuspected: Boolean,
        val firstActivityName: String?
    )

    private fun hasDuplicateInDayBlock(
        lines: List<String>,
        blockStart: Int,
        blockEnd: Int,
        eventTime: String,
        activity: String
    ): Boolean {
        val normalizedTarget = normalizeForComparison(activity)
        if (normalizedTarget.isEmpty()) {
            return false
        }

        for (index in (blockStart + 1) until blockEnd) {
            val lineTime = extractEventTimeToken(lines[index]) ?: continue
            if (lineTime != eventTime) {
                continue
            }

            val lineActivity = extractActivityName(lines[index])
            val normalizedLine = normalizeForComparison(lineActivity)
            if (normalizedLine == normalizedTarget) {
                return true
            }
        }
        return false
    }

    private fun findFirstActivityName(
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

    private fun extractEventTimeToken(line: String): String? {
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

    private fun extractActivityName(line: String): String {
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

    private fun isWakeLikeActivity(activityName: String): Boolean {
        val normalized = normalizeForComparison(activityName)
        if (normalized.isEmpty()) {
            return false
        }

        return normalized == "w" ||
            normalized.contains("wake") ||
            normalized.contains("起床") ||
            normalized.contains("新的一天开始了")
    }

    private fun normalizeForComparison(value: String): String {
        return value.trim()
            .lowercase(Locale.US)
            .replace("\\s+".toRegex(), "")
    }

    private fun appendNewDayBlock(lines: MutableList<String>, dayMarker: String, eventLine: String) {
        if (lines.isNotEmpty() && lines.last().isNotEmpty()) {
            lines += ""
        }
        lines += dayMarker
        lines += eventLine
    }

    private fun findDayBlockEnd(lines: List<String>, blockStart: Int): Int {
        for (index in (blockStart + 1) until lines.size) {
            if (isDayMarker(lines[index])) {
                return index
            }
        }
        return lines.size
    }

    private fun findInsertionIndex(
        lines: List<String>,
        blockStart: Int,
        blockEnd: Int,
        eventLine: String
    ): Int {
        val eventTime = parseEventTime(eventLine)
        if (eventTime == null) {
            return blockEnd
        }

        var insertAt = blockEnd
        for (index in (blockStart + 1) until blockEnd) {
            val lineTime = parseEventTime(lines[index]) ?: continue
            if (lineTime > eventTime) {
                insertAt = index
                break
            }
        }
        return insertAt
    }

    private fun isDayMarker(line: String): Boolean {
        val trimmed = line.trim()
        return trimmed.length == 4 && trimmed.all { it.isDigit() }
    }

    private fun parseEventTime(line: String): Int? {
        val trimmed = line.trimStart()
        if (trimmed.length < 4) {
            return null
        }
        val hhmm = trimmed.substring(0, 4)
        if (!hhmm.all { it.isDigit() }) {
            return null
        }
        return hhmm.toIntOrNull()
    }
}
