package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

internal class RuntimeReportDelegate(
    private val executeReportAfterInit: (
        operationName: String,
        action: (RuntimePaths) -> String
    ) -> ReportCallResult,
    private val nativeReportJson: (String) -> String,
    private val requestCodec: TemporalReportRequestJsonCodec = TemporalReportRequestJsonCodec(),
    private val structuredReportResultParser: StructuredReportResultParser =
        StructuredReportResultParser()
) {
    suspend fun reportMarkdown(request: TemporalReportQueryRequest): ReportCallResult =
        withContext(Dispatchers.IO) {
            val requestJson = requestCodec.encodeQuery(request)
            executeReportAfterInit(buildReportOperationName(request.displayMode)) {
                nativeReportJson(requestJson)
            }
        }

    suspend fun reportStructured(
        request: TemporalReportQueryRequest
    ): StructuredReportCallResult = withContext(Dispatchers.IO) {
        val requestJson = requestCodec.encodeStructuredQuery(request)
        val result = executeReportAfterInit("structured_${request.displayMode.wireValue}") {
            nativeReportJson(requestJson)
        }
        structuredReportResultParser.parse(result)
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
