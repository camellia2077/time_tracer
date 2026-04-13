package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

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
