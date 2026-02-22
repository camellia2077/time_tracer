package com.example.tracer

import java.text.SimpleDateFormat
import java.util.Calendar
import java.util.Locale

internal data class RecordTarget(
    val inputPath: String,
    val preferredInnerPath: String?,
    val targetFile: java.io.File
)

internal data class LogicalDateParseResult(
    val ok: Boolean,
    val date: String?,
    val message: String
)

internal fun parseLogicalDate(targetDateIso: String?): LogicalDateParseResult {
    if (targetDateIso.isNullOrBlank()) {
        val nowStr = SimpleDateFormat("yyyy-MM-dd", Locale.US).format(Calendar.getInstance().time)
        return LogicalDateParseResult(
            ok = true,
            date = nowStr,
            message = ""
        )
    }

    return try {
        val sdf = SimpleDateFormat("yyyy-MM-dd", Locale.US).apply { isLenient = false }
        val parsed = sdf.parse(targetDateIso.trim())
        LogicalDateParseResult(
            ok = true,
            date = SimpleDateFormat("yyyy-MM-dd", Locale.US).format(parsed!!),
            message = ""
        )
    } catch (_: Exception) {
        LogicalDateParseResult(
            ok = false,
            date = null,
            message = "Invalid target date. Use ISO YYYY-MM-DD."
        )
    }
}

internal fun resolveRecordTarget(
    paths: RuntimePaths,
    storage: TextStorage,
    logicalDate: String,
    preferredTxtPath: String?
): RecordTarget {
    val resolvedSourceTarget = preferredTxtPath?.let { storage.resolveSourceTarget(it) }
    val targetInputPath = resolvedSourceTarget?.first ?: paths.liveRawInputPath
    val preferredInnerPath = resolvedSourceTarget?.second
    val targetMonthFileName = "${logicalDate.substring(0, 7)}.txt"
    val targetFile = if (preferredInnerPath.isNullOrBlank()) {
        java.io.File(targetInputPath, targetMonthFileName)
    } else {
        java.io.File(targetInputPath, preferredInnerPath)
    }
    return RecordTarget(
        inputPath = targetInputPath,
        preferredInnerPath = preferredInnerPath,
        targetFile = targetFile
    )
}

internal fun buildRecordSyncFailureResult(
    recordSummary: String,
    syncResult: NativeCallResult,
    responseCodec: NativeResponseCodec
): RecordActionResult? {
    if (!syncResult.initialized) {
        return RecordActionResult(
            ok = false,
            message = "$recordSummary\nsync: failed (native init failed)"
        )
    }
    if (syncResult.operationOk) {
        return null
    }

    val payload = responseCodec.parse(syncResult.rawResponse)
    val error = if (payload.errorMessage.isNotEmpty()) {
        payload.errorMessage
    } else {
        syncResult.rawResponse
    }
    return RecordActionResult(
        ok = false,
        message = "$recordSummary\nsync: failed ($error)"
    )
}

internal fun buildRecordSummary(
    snapshot: RecordWriteSnapshot,
    monthFileCreated: Boolean
): String {
    val parts = mutableListOf<String>()
    parts += "record: ok"
    parts += "logical_date: ${snapshot.logicalDate}"
    parts += "target_file: ${snapshot.monthFile.absolutePath}"
    parts += "write_time: ${snapshot.eventTime}"
    if (monthFileCreated) {
        parts += "month_file_created: true"
    }
    for (warning in snapshot.warnings) {
        parts += "warning: $warning"
    }
    return parts.joinToString(separator = "\n")
}
