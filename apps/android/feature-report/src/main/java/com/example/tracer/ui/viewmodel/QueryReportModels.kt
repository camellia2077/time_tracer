package com.example.tracer

import java.text.SimpleDateFormat
import java.util.Calendar
import java.util.Date
import java.util.Locale

sealed class QueryResult {
    data class Report(
        val text: String,
        val summary: ReportSummary? = null
    ) : QueryResult()
    data class Stats(val text: String, val period: DataTreePeriod) : QueryResult()
    data class Tree(
        val period: DataTreePeriod,
        val nodes: List<TreeNode>,
        val found: Boolean,
        val roots: List<String> = emptyList(),
        val message: String = "",
        val fallbackText: String = "",
        val usesTextFallback: Boolean = false
    ) : QueryResult()
}

sealed interface ReportSummary {
    data class MissingTarget(
        val period: DataTreePeriod,
        val errorCode: String,
        val errorCategory: String,
        val hints: List<String> = emptyList()
    ) : ReportSummary

    data class WindowMetadata(
        val period: DataTreePeriod,
        val metadata: ReportWindowMetadata
    ) : ReportSummary
}

enum class ReportResultDisplayMode {
    TEXT,
    CHART
}

data class QueryReportUiState(
    val reportMode: ReportMode = ReportMode.DAY,
    val reportDate: String = currentDateDigits(),
    val reportMonth: String = currentMonthDigits(),
    val reportYear: String = currentIsoYear(),
    val reportWeek: String = currentWeekDigits(),
    val reportRangeStartDate: String = currentMonthStartDateDigits(),
    val reportRangeEndDate: String = currentDateDigits(),
    val reportRecentDays: String = "7",
    val reportResultsByPeriod: Map<DataTreePeriod, QueryResult.Report> = emptyMap(),
    val reportSummariesByPeriod: Map<DataTreePeriod, ReportSummary> = emptyMap(),
    val reportErrorsByPeriod: Map<DataTreePeriod, String> = emptyMap(),
    val activeResult: QueryResult? = null,
    val statsPeriod: DataTreePeriod = DataTreePeriod.RECENT,
    val treePeriod: DataTreePeriod = DataTreePeriod.RECENT,
    val resultDisplayMode: ReportResultDisplayMode = ReportResultDisplayMode.TEXT,
    val chartSemanticMode: ReportChartSemanticMode = ReportChartSemanticMode.COMPOSITION,
    val compositionVisualMode: ReportCompositionVisualMode =
        ReportCompositionVisualMode.HORIZONTAL_BAR,
    val trendChartRoots: List<String> = emptyList(),
    val trendChartSelectedRoot: String = "",
    val trendChartRenderModel: ChartRenderModel? = null,
    val trendChartLastTrace: ChartQueryTrace? = null,
    val trendChartPoints: List<ReportChartPoint> = emptyList(),
    val trendChartAverageDurationSeconds: Long? = null,
    val trendChartTotalDurationSeconds: Long? = null,
    val trendChartActiveDays: Int? = null,
    val trendChartRangeDays: Int? = null,
    val trendChartUsesLegacyStatsFallback: Boolean = false,
    val trendChartLoading: Boolean = false,
    val trendChartError: String = "",
    val compositionChartRenderModel: CompositionChartRenderModel? = null,
    val compositionChartLastTrace: ChartQueryTrace? = null,
    val compositionChartLoading: Boolean = false,
    val compositionChartError: String = "",
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
