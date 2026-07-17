package com.example.tracer

import java.time.Clock
import java.time.LocalDate
import java.time.LocalTime
import java.time.YearMonth
import java.time.format.DateTimeFormatter
import java.time.temporal.WeekFields
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
        val message: String = ""
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
    val parameterSection: ReportParameterSection = ReportParameterSection.DAY,
    val chartSemanticMode: ReportChartSemanticMode = ReportChartSemanticMode.COMPOSITION,
    val preferredChartSemanticMode: ReportChartSemanticMode =
        ReportChartSemanticMode.COMPOSITION,
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

internal fun initialQueryReportUiState(
    clock: Clock = Clock.systemDefaultZone()
): QueryReportUiState = QueryReportUiState(
    reportDate = currentDateDigits(clock),
    reportMonth = currentMonthDigits(clock),
    reportYear = currentIsoYear(clock),
    reportWeek = currentWeekDigits(clock),
    reportRangeStartDate = currentMonthStartDateDigits(clock),
    reportRangeEndDate = currentDateDigits(clock)
)

internal fun currentDateDigits(clock: Clock = Clock.systemDefaultZone()): String =
    resolveCurrentReportLogicalDate(clock).format(QUERY_REPORT_DAY_FORMATTER)

private fun currentMonthDigits(clock: Clock = Clock.systemDefaultZone()): String =
    YearMonth.from(resolveCurrentReportLogicalDate(clock)).format(QUERY_REPORT_MONTH_FORMATTER)

private fun currentIsoYear(clock: Clock = Clock.systemDefaultZone()): String =
    resolveCurrentReportLogicalDate(clock).year.toString()

private fun currentWeekDigits(clock: Clock = Clock.systemDefaultZone()): String =
    resolveCurrentReportLogicalDate(clock).let { date ->
        // Core reporting uses ISO 8601 weeks. Do not replace this with the
        // locale-dependent `YYYYww` formatter: near year boundaries it can
        // produce a different week-based year and an unqueryable target.
        val isoWeekFields = WeekFields.ISO
        String.format(
            Locale.US,
            "%04d%02d",
            date.get(isoWeekFields.weekBasedYear()),
            date.get(isoWeekFields.weekOfWeekBasedYear())
        )
    }

private fun currentMonthStartDateDigits(clock: Clock = Clock.systemDefaultZone()): String =
    YearMonth.from(resolveCurrentReportLogicalDate(clock))
        .atDay(1)
        .format(QUERY_REPORT_DAY_FORMATTER)

private fun resolveCurrentReportLogicalDate(clock: Clock): LocalDate {
    val now = LocalDate.now(clock)
    val localTime = LocalTime.now(clock)
    return if (localTime.isBefore(QUERY_REPORT_LOGICAL_DAY_CUTOFF)) {
        now.minusDays(1)
    } else {
        now
    }
}

private val QUERY_REPORT_LOGICAL_DAY_CUTOFF: LocalTime = LocalTime.of(6, 0)
private val QUERY_REPORT_DAY_FORMATTER: DateTimeFormatter = DateTimeFormatter.BASIC_ISO_DATE
private val QUERY_REPORT_MONTH_FORMATTER: DateTimeFormatter = DateTimeFormatter.ofPattern("yyyyMM")
