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
    logicalDate: String,
    preferredTxtPath: String?
): RecordTarget {
    val preferredInnerPath = preferredTxtPath?.let {
        resolveCanonicalTxtRelativePath(paths.inputRootPath, it)
    }
    val targetMonthFileName = buildMonthRelativePath(logicalDate.substring(0, 7))
    val targetFile = if (preferredInnerPath.isNullOrBlank()) {
        java.io.File(paths.inputRootPath, targetMonthFileName)
    } else {
        java.io.File(paths.inputRootPath, preferredInnerPath)
    }
    return RecordTarget(
        inputPath = paths.inputRootPath,
        preferredInnerPath = preferredInnerPath,
        targetFile = targetFile
    )
}

internal fun buildMonthRelativePath(monthKey: String): String {
    val normalized = monthKey.trim()
    require(Regex("""^\d{4}-\d{2}$""").matches(normalized)) {
        "Invalid month key: $monthKey"
    }
    val year = normalized.substring(0, 4)
    return "$year/$normalized.txt"
}

internal fun buildRecordSyncFailureResult(
    recordSummary: String,
    syncResult: NativeCallResult,
    responseCodec: NativeResponseCodec
): RecordActionResult? {
    if (!syncResult.initialized) {
        return RecordActionResult(
            ok = false,
            message = appendFailureContext(
                message = "$recordSummary\nsync: failed (native init failed)",
                operationId = syncResult.operationId,
                errorLogPath = syncResult.errorLogPath
            ),
            operationId = syncResult.operationId
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
        message = appendFailureContext(
            message = "$recordSummary\nsync: failed ($error)",
            operationId = syncResult.operationId,
            errorLogPath = syncResult.errorLogPath
        ),
        operationId = syncResult.operationId
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
    // Android local fallback summary has no previous-event context here,
    // so keep the same field contract with `n/a`.
    parts += "gap_from_previous: n/a"
    if (monthFileCreated) {
        parts += "month_file_created: true"
    }
    for (warning in snapshot.warnings) {
        parts += if (warning.startsWith("Warning:")) warning else "warning: $warning"
    }
    return parts.joinToString(separator = "\n")
}
