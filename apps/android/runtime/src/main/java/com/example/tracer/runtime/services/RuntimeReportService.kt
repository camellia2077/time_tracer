package com.example.tracer

internal class RuntimeReportService(
    private val reportDelegate: RuntimeReportDelegate
) {
    suspend fun reportDayMarkdown(date: String): ReportCallResult =
        reportDelegate.reportDayMarkdown(date)

    suspend fun reportMonthMarkdown(month: String): ReportCallResult =
        reportDelegate.reportMonthMarkdown(month)

    suspend fun reportYearMarkdown(year: String): ReportCallResult =
        reportDelegate.reportYearMarkdown(year)

    suspend fun reportWeekMarkdown(week: String): ReportCallResult =
        reportDelegate.reportWeekMarkdown(week)

    suspend fun reportRecentMarkdown(days: String): ReportCallResult =
        reportDelegate.reportRecentMarkdown(days)

    suspend fun reportRange(startDate: String, endDate: String): ReportCallResult =
        reportDelegate.reportRange(startDate, endDate)
}
