package com.example.tracer

interface ReportGateway {
    suspend fun reportDayMarkdown(date: String): ReportCallResult
    suspend fun reportMonthMarkdown(month: String): ReportCallResult
    suspend fun reportYearMarkdown(year: String): ReportCallResult
    suspend fun reportWeekMarkdown(week: String): ReportCallResult
    suspend fun reportRecentMarkdown(days: String): ReportCallResult
    suspend fun reportRange(startDate: String, endDate: String): ReportCallResult
}
