package com.example.tracer

import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.test.advanceUntilIdle
import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Rule
import org.junit.Test

@OptIn(ExperimentalCoroutinesApi::class)
class QueryInsightsPreviousPeriodComparisonViewModelTest {
    @get:Rule
    val mainDispatcherRule = MainDispatcherRule()

    @Test
    fun comparisonToggleImmediatelyLoadsThePreviousWeek() = runTest {
        val gateway = ComparisonInsightsGateway()
        val viewModel = QueryInsightsViewModel(gateway, ComparisonQueryGateway())
        viewModel.uiState = comparisonState(InsightsMode.WEEK).copy(insightsWeek = "202601")

        viewModel.onPeriodComparisonToggle()
        advanceUntilIdle()

        assertEquals(1, gateway.structuredRequests.size)
        assertEquals("2025-12-22", gateway.structuredRequests.single().selection.startDate)
        assertTrue(viewModel.uiState.periodComparison is InsightsPeriodComparisonState.Ready)
        val ready = viewModel.uiState.periodComparison as InsightsPeriodComparisonState.Ready
        assertEquals(1, ready.activityDays.size)
        assertEquals("previous", ready.projectTree.single().name)
    }

    @Test
    fun comparisonFailureLeavesOverviewAvailableForRetry() = runTest {
        val gateway = ComparisonInsightsGateway(shouldFail = true)
        val viewModel = QueryInsightsViewModel(gateway, ComparisonQueryGateway())
        viewModel.uiState = comparisonState(InsightsMode.MONTH).copy(insightsMonth = "202602")

        viewModel.onPeriodComparisonToggle()
        viewModel.onComparisonPeriodSelected(
            InsightsPeriodSelection(
                date = "20260201",
                month = "2026-01",
                year = "2026",
                week = "202605"
            )
        )
        advanceUntilIdle()

        val failure = viewModel.uiState.periodComparison as InsightsPeriodComparisonState.Failed
        assertEquals("previous period unavailable", failure.message)
        assertTrue(viewModel.canComparePreviousPeriod())
    }

    @Test
    fun currentPeriodChangeKeepsTheSelectedComparisonPeriod() = runTest {
        val viewModel = QueryInsightsViewModel(ComparisonInsightsGateway(), ComparisonQueryGateway())
        viewModel.uiState = comparisonState(InsightsMode.MONTH).copy(insightsMonth = "202602")
        val selectedComparison = InsightsPeriodSelection(
            date = "20260201",
            month = "2025-01",
            year = "2025",
            week = "202605"
        )

        viewModel.onPeriodComparisonToggle()
        viewModel.onComparisonPeriodSelected(selectedComparison)
        viewModel.onInsightsMonthChange("202603")
        advanceUntilIdle()

        assertEquals(
            selectedComparison,
            viewModel.uiState.periodComparison.selectionOrNull()
        )
    }

    @Test
    fun staleCurrentPeriodResultPreservesTheReadyComparison() {
        val selectedComparison = InsightsPeriodSelection(
            date = "20260201",
            month = "2025-01",
            year = "2025",
            week = "202605"
        )
        val latestState = comparisonState(InsightsMode.MONTH).copy(
            periodComparison = InsightsPeriodComparisonState.Ready(
                label = "Jan 2025",
                selection = selectedComparison,
                activityDays = listOf(StructuredDailyInsights("2025-01-01", 1_800)),
                projectTree = listOf(StructuredInsightsProjectNode("previous", 1_800)),
                activityAggregate = ActivityAggregate(),
                chartRenderModel = null
            ),
            periodComparisonVersion = 2
        )
        val staleCurrentPeriodResult = comparisonState(InsightsMode.MONTH)

        val mergedResult = staleCurrentPeriodResult.preserveLatestPeriodComparison(latestState)

        assertEquals(2, mergedResult.periodComparisonVersion)
        assertEquals(selectedComparison, mergedResult.periodComparison.selectionOrNull())
        assertTrue(mergedResult.periodComparison is InsightsPeriodComparisonState.Ready)
    }

    @Test
    fun comparisonAcceptsTheIsoMonthValueEmittedByTheCalendarPicker() {
        val request = resolveComparisonPeriodRequest(
            state = comparisonState(InsightsMode.MONTH),
            selection = InsightsPeriodSelection(
                date = "20260201",
                month = "2026-04",
                year = "2026",
                week = "202605"
            ),
            locale = "en"
        )

        assertEquals("2026-04-01", request?.request?.selection?.startDate)
        assertEquals("2026-04-30", request?.request?.selection?.endDate)
    }

    private fun comparisonState(mode: InsightsMode): QueryInsightsUiState {
        val period = mode.toDataTreePeriod()
        return QueryInsightsUiState(
            insightsMode = mode,
            insightsResultsByPeriod = mapOf(period to QueryResult.Insights("current")),
            periodActivityDays = mapOf(
                period to listOf(StructuredDailyInsights("2026-01-01", 3_600))
            )
        )
    }
}

private class ComparisonInsightsGateway(
    private val shouldFail: Boolean = false
) : InsightsGateway {
    val structuredRequests = mutableListOf<TemporalInsightsQueryRequest>()

    override suspend fun insightsMarkdown(request: TemporalInsightsQueryRequest): InsightsCallResult =
        InsightsCallResult(true, true, "", "")

    override suspend fun insightsStructured(request: TemporalInsightsQueryRequest): StructuredInsightsCallResult {
        structuredRequests += request
        return if (shouldFail) {
            StructuredInsightsCallResult(true, false, null, "", errorMessage = "previous period unavailable")
        } else {
            StructuredInsightsCallResult(
                initialized = true,
                operationOk = true,
                insights = null,
                rawResponse = "",
                activityDays = listOf(StructuredDailyInsights("2025-12-22", 1_800)),
                projectTree = listOf(StructuredInsightsProjectNode("previous", 1_800))
            )
        }
    }
}

private class ComparisonQueryGateway : QueryGateway {
    override suspend fun queryFrequentActivities(lookbackDays: Int, topN: Int, anchorDateIso: String?) =
        ActivityFrequentResult(true, emptyList(), "")
    override suspend fun queryDayDurations(params: DataDurationQueryParams) = DataQueryTextResult(true, "", "")
    override suspend fun queryDayDurationStats(params: DataDurationQueryParams) = DataQueryTextResult(true, "", "")
    override suspend fun queryProjectTree(params: DataTreeQueryParams) = TreeQueryResult(
        ok = true,
        found = false,
        message = ""
    )
    override suspend fun queryInsightsChart(params: InsightsChartQueryParams) = InsightsChartQueryResult(
        true,
        InsightsChartData(emptyList(), "", params.lookbackDays, emptyList()),
        ""
    )
    override suspend fun listActivityMappingNames() = ActivityMappingNamesResult(true, emptyList(), "")
}
