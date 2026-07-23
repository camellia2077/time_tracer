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

    suspend fun reportStructured(
        request: TemporalReportQueryRequest
    ): StructuredReportCallResult = withContext(Dispatchers.IO) {
        val requestJson = requestCodec.encodeStructuredQuery(request)
        val result = executeReportAfterInit("structured_${request.displayMode.wireValue}") {
            nativeReportJson(requestJson)
        }
        parseStructuredReportResult(result)
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

    private fun parseStructuredReportResult(
        result: ReportCallResult
    ): StructuredReportCallResult {
        if (!result.operationOk) {
            return StructuredReportCallResult(
                initialized = result.initialized,
                operationOk = false,
                report = null,
                rawResponse = result.rawResponse,
                errorMessage = result.outputText,
                operationId = result.operationId
            )
        }
        return runCatching {
            val root = org.json.JSONObject(result.rawResponse)
            val report = root.optJSONObject("report")
                ?: error("Structured report payload is missing report.")
            val records = report.optJSONArray("detailed_records")
            val activities = buildList {
                if (records != null) {
                    for (index in 0 until records.length()) {
                        val record = records.optJSONObject(index) ?: continue
                        val startTime = record.optString("start_time", "")
                        val endTime = record.optString("end_time", "")
                        val activityName = record.optString("project_path", "")
                        val logicalId = record.optLong("logical_id", 0L)
                        Log.i(
                            REPORT_LOG_TAG,
                            "structured timeline record index=$index logicalId=$logicalId " +
                                "date=${report.optString("date", "")} " +
                                "start=$startTime end=$endTime activity=$activityName"
                        )
                        add(
                            ActivityTimelineItem(
                                logicalId = logicalId,
                                startTime = startTime,
                                endTime = endTime,
                                activityName = activityName,
                                durationSeconds = record.optLong("duration_seconds", 0L)
                                    .coerceAtLeast(0L),
                                remark = record.optString("activity_remark", "")
                                    .ifBlank { null }
                            )
                        )
                    }
                }
            }
            StructuredReportCallResult(
                initialized = result.initialized,
                operationOk = true,
                report = StructuredDailyReport(
                    date = report.optString("date", ""),
                    totalDurationSeconds = report.optLong("total_duration", 0L)
                        .coerceAtLeast(0L),
                    dayRemark = report.optJSONObject("metadata")
                        ?.optString("remark", "")
                        .orEmpty(),
                    activities = activities
                ),
                rawResponse = result.rawResponse,
                operationId = result.operationId
            )
        }.getOrElse { error ->
            StructuredReportCallResult(
                initialized = result.initialized,
                operationOk = false,
                report = null,
                rawResponse = result.rawResponse,
                errorMessage = error.message ?: "Invalid structured report payload.",
                operationId = result.operationId
            )
        }
    }
}
