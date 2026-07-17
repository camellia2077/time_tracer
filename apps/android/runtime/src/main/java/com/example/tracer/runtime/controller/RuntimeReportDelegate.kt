package com.example.tracer

import android.util.Log
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

private const val REPORT_LOG_TAG = "TimeTracerReport"

// Keep request fields in this tag: it makes malformed period parameters
// diagnosable from logcat without requiring a debugger on the device.

internal class RuntimeReportDelegate(
    private val executeReportAfterInit: (
        operationName: String,
        action: (RuntimePaths) -> String
    ) -> ReportCallResult,
    private val nativeReportJson: (String) -> String,
    private val requestCodec: TemporalReportRequestJsonCodec = TemporalReportRequestJsonCodec()
) {
    suspend fun reportMarkdown(request: TemporalReportQueryRequest): ReportCallResult =
        withContext(Dispatchers.IO) {
            val requestJson = requestCodec.encodeQuery(request)
            Log.i(
                REPORT_LOG_TAG,
                "reportMarkdown request mode=${request.displayMode.wireValue} " +
                    "format=${request.format.wireValue} json=$requestJson"
            )
            executeReportAfterInit(buildReportOperationName(request.displayMode)) {
                nativeReportJson(requestJson)
            }
        }

    private fun buildReportOperationName(displayMode: ReportDisplayMode): String {
        val suffix = when (displayMode) {
            ReportDisplayMode.DAY -> "day"
            ReportDisplayMode.MONTH -> "month"
            ReportDisplayMode.YEAR -> "year"
            ReportDisplayMode.WEEK -> "week"
            ReportDisplayMode.RECENT -> "recent"
            ReportDisplayMode.RANGE -> "range"
        }
        return "native_report_$suffix"
    }
}
