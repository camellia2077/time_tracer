package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

internal class RuntimeReportDelegate(
    private val executeReportAfterInit: (
        operationName: String,
        action: (RuntimePaths) -> String
    ) -> ReportCallResult
) {
    suspend fun reportDayMarkdown(date: String): ReportCallResult =
        runMarkdownReportFlow(
            reportType = NativeBridge.REPORT_TYPE_DAY,
            argument = date
        )

    suspend fun reportMonthMarkdown(month: String): ReportCallResult =
        runMarkdownReportFlow(
            reportType = NativeBridge.REPORT_TYPE_MONTH,
            argument = month
        )

    suspend fun reportYearMarkdown(year: String): ReportCallResult =
        runMarkdownReportFlow(
            reportType = NativeBridge.REPORT_TYPE_YEAR,
            argument = year
        )

    suspend fun reportWeekMarkdown(week: String): ReportCallResult =
        runMarkdownReportFlow(
            reportType = NativeBridge.REPORT_TYPE_WEEK,
            argument = week
        )

    suspend fun reportRecentMarkdown(days: String): ReportCallResult =
        runMarkdownReportFlow(
            reportType = NativeBridge.REPORT_TYPE_RECENT,
            argument = days
        )

    suspend fun reportRange(startDate: String, endDate: String): ReportCallResult =
        runMarkdownReportFlow(
            reportType = NativeBridge.REPORT_TYPE_RANGE,
            argument = "$startDate|$endDate"
        )

    private suspend fun runMarkdownReportFlow(
        reportType: Int,
        argument: String
    ): ReportCallResult = withContext(Dispatchers.IO) {
        executeReportAfterInit(buildReportOperationName(reportType)) {
            NativeBridge.nativeReport(
                mode = NativeBridge.REPORT_MODE_SINGLE,
                reportType = reportType,
                argument = argument,
                format = NativeBridge.REPORT_FORMAT_MARKDOWN,
                daysList = null
            )
        }
    }

    private fun buildReportOperationName(reportType: Int): String {
        val suffix = when (reportType) {
            NativeBridge.REPORT_TYPE_DAY -> "day"
            NativeBridge.REPORT_TYPE_MONTH -> "month"
            NativeBridge.REPORT_TYPE_YEAR -> "year"
            NativeBridge.REPORT_TYPE_WEEK -> "week"
            NativeBridge.REPORT_TYPE_RECENT -> "recent"
            NativeBridge.REPORT_TYPE_RANGE -> "range"
            else -> "unknown"
        }
        return "native_report_$suffix"
    }
}
