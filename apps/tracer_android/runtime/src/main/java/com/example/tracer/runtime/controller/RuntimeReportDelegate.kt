package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

internal class RuntimeReportDelegate(
    private val executeReportAfterInit: ((RuntimePaths) -> String) -> ReportCallResult
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
        executeReportAfterInit {
            NativeBridge.nativeReport(
                mode = NativeBridge.REPORT_MODE_SINGLE,
                reportType = reportType,
                argument = argument,
                format = NativeBridge.REPORT_FORMAT_MARKDOWN,
                daysList = null
            )
        }
    }
}
