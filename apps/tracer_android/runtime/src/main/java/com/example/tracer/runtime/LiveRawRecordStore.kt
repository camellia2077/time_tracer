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
    private val txtFileStore = LiveRawTxtFileStore()
    private val normalization = LiveRawRecordNormalization()
    private val parsing = LiveRawRecordParsing(normalization)
    private val persistence = LiveRawRecordPersistence(parsing)

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
        return txtFileStore.listTxtFiles(liveRawInputPath)
    }

    fun readTxtFile(liveRawInputPath: String, relativePath: String): TxtFileContentResult {
        return txtFileStore.readTxtFile(liveRawInputPath, relativePath)
    }

    fun writeTxtFile(liveRawInputPath: String, relativePath: String, content: String): TxtFileContentResult {
        return txtFileStore.writeTxtFile(liveRawInputPath, relativePath, content)
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
}
