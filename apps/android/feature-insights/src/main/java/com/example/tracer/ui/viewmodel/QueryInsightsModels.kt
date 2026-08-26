package com.example.tracer

import java.time.Clock
import java.time.LocalDate
import java.time.LocalTime
import java.time.YearMonth
import java.time.format.DateTimeFormatter
import java.time.temporal.WeekFields
import java.util.Locale

sealed class QueryResult {
    data class Insights(
        val text: String,
        val summary: InsightsSummary? = null
    ) : QueryResult()
    data class Tree(
        val period: DataTreePeriod,
        val nodes: List<TreeNode>,
        val found: Boolean,
        val roots: List<String> = emptyList(),
        val message: String = "",
        val maxAvailableDepth: Int = 0
    ) : QueryResult()
}

sealed interface InsightsSummary {
    data class NoData(
        val period: DataTreePeriod
    ) : InsightsSummary

    data class MissingTarget(
        val period: DataTreePeriod,
        val errorCode: String,
        val errorCategory: String,
        val hints: List<String> = emptyList()
    ) : InsightsSummary

    data class WindowMetadata(
        val period: DataTreePeriod,
        val metadata: InsightsWindowMetadata
    ) : InsightsSummary
}

sealed interface InsightsPeriodComparisonState {
    data object Hidden : InsightsPeriodComparisonState
    data class Loading(
        val selection: InsightsPeriodSelection
    ) : InsightsPeriodComparisonState
    data class Ready(
        val label: String,
        val selection: InsightsPeriodSelection,
        val activityDays: List<StructuredDailyInsights>,
        val projectTree: List<StructuredInsightsProjectNode>,
        val activityAggregate: ActivityAggregate,
        val chartRenderModel: ChartRenderModel?
    ) : InsightsPeriodComparisonState
    data class Failed(
        val selection: InsightsPeriodSelection,
        val message: String
    ) : InsightsPeriodComparisonState
}

internal fun InsightsPeriodComparisonState.selectionOrNull(): InsightsPeriodSelection? = when (this) {
    is InsightsPeriodComparisonState.Loading -> selection
    is InsightsPeriodComparisonState.Ready -> selection
    is InsightsPeriodComparisonState.Failed -> selection
    InsightsPeriodComparisonState.Hidden -> null
}

enum class InsightsResultDisplayMode {
    DETAILS,
    CHART
}

data class QueryInsightsUiState(
    val averageDayBasis: InsightsAverageDayBasis = InsightsAverageDayBasis.ACTIVE_DAYS,
    val insightsMode: InsightsMode = InsightsMode.DAY,
    val insightsDate: String = currentDateDigits(),
    val insightsMonth: String = currentMonthDigits(),
    val insightsYear: String = currentIsoYear(),
    val insightsWeek: String = currentWeekDigits(),
    val insightsRangeStartDate: String = currentMonthStartDateDigits(),
    val insightsRangeEndDate: String = currentDateDigits(),
    val insightsRecentDays: String = "7",
    val availableInsightsMonths: List<String> = emptyList(),
    val insightsResultsByPeriod: Map<DataTreePeriod, QueryResult.Insights> = emptyMap(),
    val insightsSummariesByPeriod: Map<DataTreePeriod, InsightsSummary> = emptyMap(),
    val insightsErrorsByPeriod: Map<DataTreePeriod, String> = emptyMap(),
    val dayTimeline: StructuredDailyInsights? = null,
    val periodActivityDays: Map<DataTreePeriod, List<StructuredDailyInsights>> = emptyMap(),
    val periodActivityAggregates: Map<DataTreePeriod, ActivityAggregate> = emptyMap(),
    val periodActivityProjectTrees: Map<DataTreePeriod, List<StructuredInsightsProjectNode>> = emptyMap(),
    val periodComparison: InsightsPeriodComparisonState = InsightsPeriodComparisonState.Hidden,
    // Incremented whenever comparison is disabled or its selected period changes, so a delayed
    // response cannot replace a newer comparison selection. Current-window changes intentionally
    // do not increment it because the comparison period is selected independently.
    val periodComparisonVersion: Long = 0L,
    // Trend comparison uses the chart query so a selected root is preserved. It is kept
    // separate from the Activities comparison, whose payload is an aggregate insights query.
    val trendChartComparison: InsightsPeriodComparisonState = InsightsPeriodComparisonState.Hidden,
    val trendChartComparisonVersion: Long = 0L,
    val statusValues: List<InsightsStatusValue> = emptyList(),
    // Timeline remark edits are applied locally first to preserve scroll position. The next
    // Markdown/insights load clears this flag after Core has supplied a fresh projection.
    val dayInsightsNeedsRefresh: Boolean = false,
    val activeResult: QueryResult? = null,
    val treePeriod: DataTreePeriod = DataTreePeriod.RECENT,
    val treeLevel: Int = -1,
    val resultDisplayMode: InsightsResultDisplayMode = InsightsResultDisplayMode.DETAILS,
    val isPresentationRestored: Boolean = false,
    val parameterSection: InsightsParameterSection = InsightsParameterSection.DAY,
    val chartSemanticMode: InsightsChartSemanticMode = InsightsChartSemanticMode.COMPOSITION,
    val preferredChartSemanticMode: InsightsChartSemanticMode =
        InsightsChartSemanticMode.COMPOSITION,
    val compositionVisualMode: InsightsCompositionVisualMode =
        InsightsCompositionVisualMode.HORIZONTAL_BAR,
    val trendChartRoots: List<String> = emptyList(),
    val trendChartSelectedRoot: String = "",
    val trendChartRenderModel: ChartRenderModel? = null,
    val trendChartLastTrace: ChartQueryTrace? = null,
    val trendChartPoints: List<InsightsChartPoint> = emptyList(),
    val trendChartAverageDurationSeconds: Long? = null,
    val trendChartTotalDurationSeconds: Long? = null,
    val trendChartActiveDays: Int? = null,
    val trendChartRangeDays: Int? = null,
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

internal fun initialQueryInsightsUiState(
    clock: Clock = Clock.systemDefaultZone()
): QueryInsightsUiState = QueryInsightsUiState(
    insightsDate = currentDateDigits(clock),
    insightsMonth = currentMonthDigits(clock),
    insightsYear = currentIsoYear(clock),
    insightsWeek = currentWeekDigits(clock),
    insightsRangeStartDate = currentMonthStartDateDigits(clock),
    insightsRangeEndDate = currentDateDigits(clock)
)

internal fun currentDateDigits(clock: Clock = Clock.systemDefaultZone()): String =
    resolveCurrentInsightsLogicalDate(clock).format(QUERY_INSIGHTS_DAY_FORMATTER)

private fun currentMonthDigits(clock: Clock = Clock.systemDefaultZone()): String =
    YearMonth.from(resolveCurrentInsightsLogicalDate(clock)).format(QUERY_INSIGHTS_MONTH_FORMATTER)

private fun currentIsoYear(clock: Clock = Clock.systemDefaultZone()): String =
    resolveCurrentInsightsLogicalDate(clock).year.toString()

private fun currentWeekDigits(clock: Clock = Clock.systemDefaultZone()): String =
    resolveCurrentInsightsLogicalDate(clock).let { date ->
        // Core insights uses ISO 8601 weeks. Do not replace this with the
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
    YearMonth.from(resolveCurrentInsightsLogicalDate(clock))
        .atDay(1)
        .format(QUERY_INSIGHTS_DAY_FORMATTER)

private fun resolveCurrentInsightsLogicalDate(clock: Clock): LocalDate {
    val now = LocalDate.now(clock)
    val localTime = LocalTime.now(clock)
    return if (localTime.isBefore(QUERY_INSIGHTS_LOGICAL_DAY_CUTOFF)) {
        now.minusDays(1)
    } else {
        now
    }
}

private val QUERY_INSIGHTS_LOGICAL_DAY_CUTOFF: LocalTime = LocalTime.of(6, 0)
private val QUERY_INSIGHTS_DAY_FORMATTER: DateTimeFormatter = DateTimeFormatter.BASIC_ISO_DATE
private val QUERY_INSIGHTS_MONTH_FORMATTER: DateTimeFormatter = DateTimeFormatter.ofPattern("yyyyMM")
