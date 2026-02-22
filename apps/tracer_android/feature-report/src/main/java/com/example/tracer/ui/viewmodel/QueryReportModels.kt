package com.example.tracer

import java.text.SimpleDateFormat
import java.util.Calendar
import java.util.Date
import java.util.Locale

sealed class QueryResult {
    data class Report(val text: String) : QueryResult()
    data class Stats(val text: String, val period: DataTreePeriod) : QueryResult()
    data class Tree(val text: String, val period: DataTreePeriod) : QueryResult()
}

enum class ReportResultDisplayMode {
    TEXT,
    CHART
}

enum class ChartDateInputMode {
    LOOKBACK,
    RANGE
}

data class QueryReportUiState(
    val reportDate: String = currentDateDigits(),
    val reportMonth: String = currentMonthDigits(),
    val reportYear: String = currentIsoYear(),
    val reportWeek: String = currentWeekDigits(),
    val reportRangeStartDate: String = currentMonthStartDateDigits(),
    val reportRangeEndDate: String = currentDateDigits(),
    val reportRecentDays: String = "7",
    val activeResult: QueryResult? = null,
    val statsPeriod: DataTreePeriod = DataTreePeriod.RECENT,
    val treePeriod: DataTreePeriod = DataTreePeriod.RECENT,
    val resultDisplayMode: ReportResultDisplayMode = ReportResultDisplayMode.TEXT,
    val chartRoots: List<String> = emptyList(),
    val chartSelectedRoot: String = "",
    val chartDateInputMode: ChartDateInputMode = ChartDateInputMode.LOOKBACK,
    val chartLookbackDays: String = "7",
    val chartRangeStartDate: String = "",
    val chartRangeEndDate: String = "",
    val chartPoints: List<ReportChartPoint> = emptyList(),
    val chartAverageDurationSeconds: Long? = null,
    val chartTotalDurationSeconds: Long? = null,
    val chartActiveDays: Int? = null,
    val chartRangeDays: Int? = null,
    val chartUsesLegacyStatsFallback: Boolean = false,
    val chartLoading: Boolean = false,
    val chartError: String = "",
    val analysisLoading: Boolean = false,
    val analysisError: String = "",
    val statusText: String = ""
)

private fun currentDateDigits(): String =
    SimpleDateFormat("yyyyMMdd", Locale.US).format(Date())

private fun currentMonthDigits(): String =
    SimpleDateFormat("yyyyMM", Locale.US).format(Date())

private fun currentIsoYear(): String =
    Calendar.getInstance().get(Calendar.YEAR).toString()

private fun currentWeekDigits(): String =
    SimpleDateFormat("YYYYww", Locale.US).format(Date())

private fun currentMonthStartDateDigits(): String {
    val cal = Calendar.getInstance().apply {
        set(Calendar.DAY_OF_MONTH, 1)
    }
    return SimpleDateFormat("yyyyMMdd", Locale.US).format(cal.time)
}
