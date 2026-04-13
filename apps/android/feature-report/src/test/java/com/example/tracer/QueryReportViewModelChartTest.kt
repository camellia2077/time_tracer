package com.example.tracer

import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.test.advanceUntilIdle
import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertTrue
import org.junit.Rule
import org.junit.Test

@OptIn(ExperimentalCoroutinesApi::class)
class QueryReportViewModelChartTest {
    @get:Rule
    val mainDispatcherRule = MainDispatcherRule()

    @Test
    fun switchToChart_autoLoads_withCurrentRootAndRecentMode() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = FakeChartReportGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onChartRootChange("study")
        viewModel.onReportModeChange(ReportMode.RECENT)
        viewModel.onReportRecentDaysChange("14")
        viewModel.onResultDisplayModeChange(ReportResultDisplayMode.CHART)
        advanceUntilIdle()

        assertEquals(1, fakeQueryGateway.chartQueryCount)
        assertEquals("study", fakeQueryGateway.lastChartParams?.root)
        assertEquals(14, fakeQueryGateway.lastChartParams?.lookbackDays)
        assertEquals(null, fakeQueryGateway.lastChartParams?.fromDateIso)
        assertEquals(null, fakeQueryGateway.lastChartParams?.toDateIso)

        val state = viewModel.uiState
        assertEquals(ReportResultDisplayMode.CHART, state.resultDisplayMode)
        assertTrue(!state.chartLoading)
        assertTrue(state.chartError.isEmpty())
        assertEquals(listOf("sleep", "study"), state.chartRoots)
        assertEquals("study", state.chartSelectedRoot)
        assertEquals(2, state.chartPoints.size)
        assertEquals("2026-02-13", state.chartPoints[0].date)
        assertEquals(4500L, state.chartAverageDurationSeconds)
        assertEquals(9000L, state.chartTotalDurationSeconds)
        assertEquals(2, state.chartActiveDays)
        assertEquals(2, state.chartRangeDays)
        assertEquals(false, state.chartUsesLegacyStatsFallback)
    }

    @Test
    fun switchToChart_invalidRecentDays_doesNotQueryGateway() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = FakeChartReportGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onReportModeChange(ReportMode.RECENT)
        viewModel.onReportRecentDaysChange("0")
        viewModel.onResultDisplayModeChange(ReportResultDisplayMode.CHART)
        advanceUntilIdle()

        assertEquals(0, fakeQueryGateway.chartQueryCount)
        assertTrue(viewModel.uiState.chartError.isNotBlank())
    }

    @Test
    fun switchToChart_withRange_usesSharedRangeParams() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = FakeChartReportGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onReportModeChange(ReportMode.RANGE)
        viewModel.onReportRangeStartDateChange("20260210")
        viewModel.onReportRangeEndDateChange("20260214")
        viewModel.onResultDisplayModeChange(ReportResultDisplayMode.CHART)
        advanceUntilIdle()

        assertEquals(1, fakeQueryGateway.chartQueryCount)
        assertEquals("2026-02-10", fakeQueryGateway.lastChartParams?.fromDateIso)
        assertEquals("2026-02-14", fakeQueryGateway.lastChartParams?.toDateIso)
        assertEquals("20260210", viewModel.uiState.reportRangeStartDate)
        assertEquals("20260214", viewModel.uiState.reportRangeEndDate)
        assertTrue(viewModel.uiState.chartError.isEmpty())
    }

    @Test
    fun switchToChart_withWeek_usesIsoWeekDateWindow() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = FakeChartReportGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onReportModeChange(ReportMode.WEEK)
        viewModel.onReportWeekChange("202615")
        viewModel.onResultDisplayModeChange(ReportResultDisplayMode.CHART)
        advanceUntilIdle()

        assertEquals(1, fakeQueryGateway.chartQueryCount)
        assertEquals("2026-04-06", fakeQueryGateway.lastChartParams?.fromDateIso)
        assertEquals("2026-04-12", fakeQueryGateway.lastChartParams?.toDateIso)
        assertEquals(7, fakeQueryGateway.lastChartParams?.lookbackDays)
    }

    @Test
    fun changingReportModeInChartMode_invalidatesAndReloadsChart() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = FakeChartReportGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onReportMonthChange("202602")
        viewModel.onReportModeChange(ReportMode.WEEK)
        viewModel.onReportWeekChange("202615")
        viewModel.onResultDisplayModeChange(ReportResultDisplayMode.CHART)
        advanceUntilIdle()

        assertEquals(1, fakeQueryGateway.chartQueryCount)
        assertEquals("2026-04-06", fakeQueryGateway.lastChartParams?.fromDateIso)
        assertEquals("2026-04-12", fakeQueryGateway.lastChartParams?.toDateIso)

        viewModel.onReportModeChange(ReportMode.MONTH)
        advanceUntilIdle()

        assertEquals(2, fakeQueryGateway.chartQueryCount)
        assertEquals("2026-02-01", fakeQueryGateway.lastChartParams?.fromDateIso)
        assertEquals("2026-02-28", fakeQueryGateway.lastChartParams?.toDateIso)
        assertEquals(28, fakeQueryGateway.lastChartParams?.lookbackDays)
        assertTrue(viewModel.uiState.chartError.isEmpty())
    }

    @Test
    fun changingReportParamsOutsideChart_invalidatesPreviousChartBeforeNextLoad() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = FakeChartReportGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onReportModeChange(ReportMode.WEEK)
        viewModel.onReportWeekChange("202615")
        viewModel.onResultDisplayModeChange(ReportResultDisplayMode.CHART)
        advanceUntilIdle()
        assertEquals(1, fakeQueryGateway.chartQueryCount)
        assertTrue(viewModel.uiState.chartPoints.isNotEmpty())

        viewModel.onResultDisplayModeChange(ReportResultDisplayMode.TEXT)
        viewModel.onReportModeChange(ReportMode.DAY)
        viewModel.onReportDateChange("20260413")
        advanceUntilIdle()

        assertEquals(1, fakeQueryGateway.chartQueryCount)
        assertEquals(null, viewModel.uiState.chartRenderModel)
        assertTrue(viewModel.uiState.chartPoints.isEmpty())

        viewModel.onResultDisplayModeChange(ReportResultDisplayMode.CHART)
        advanceUntilIdle()

        assertEquals(2, fakeQueryGateway.chartQueryCount)
        assertEquals("2026-04-13", fakeQueryGateway.lastChartParams?.fromDateIso)
        assertEquals("2026-04-13", fakeQueryGateway.lastChartParams?.toDateIso)
        assertEquals(1, fakeQueryGateway.lastChartParams?.lookbackDays)
    }

    @Test
    fun onChartRootChange_autoRefreshes_whenInChartMode() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = FakeChartReportGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onResultDisplayModeChange(ReportResultDisplayMode.CHART)
        advanceUntilIdle()
        assertEquals(1, fakeQueryGateway.chartQueryCount)
        assertEquals(null, fakeQueryGateway.lastChartParams?.root)

        viewModel.onChartRootChange("sleep")
        advanceUntilIdle()

        assertEquals(2, fakeQueryGateway.chartQueryCount)
        assertEquals("sleep", fakeQueryGateway.lastChartParams?.root)
    }

    @Test
    fun switchToChart_withoutCoreStats_usesDerivedStatsFromPipeline() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway(includeCoreStats = false)
        val viewModel = QueryReportViewModel(
            reportGateway = FakeChartReportGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onResultDisplayModeChange(ReportResultDisplayMode.CHART)
        advanceUntilIdle()

        val state = viewModel.uiState
        assertEquals(2, state.chartPoints.size)
        assertEquals(4500L, state.chartAverageDurationSeconds)
        assertEquals(9000L, state.chartTotalDurationSeconds)
        assertEquals(2, state.chartActiveDays)
        assertEquals(2, state.chartRangeDays)
        assertEquals(true, state.chartUsesLegacyStatsFallback)
        assertNotNull(state.chartLastTrace)
        assertEquals(false, state.chartLastTrace?.cacheHit)
    }
}

private class FakeChartReportGateway : ReportGateway {
    override suspend fun reportMarkdown(request: TemporalReportQueryRequest): ReportCallResult =
        ReportCallResult(
            initialized = true,
            operationOk = true,
            outputText = "",
            rawResponse = ""
        )
}

private class FakeChartQueryGateway(
    private val includeCoreStats: Boolean = true
) : QueryGateway {
    var chartQueryCount: Int = 0
    var lastChartParams: ReportChartQueryParams? = null

    override suspend fun queryActivitySuggestions(
        lookbackDays: Int,
        topN: Int
    ): ActivitySuggestionResult = ActivitySuggestionResult(
        ok = true,
        suggestions = emptyList(),
        message = "ok"
    )

    override suspend fun queryDayDurations(params: DataDurationQueryParams): DataQueryTextResult =
        DataQueryTextResult(ok = true, outputText = "", message = "ok")

    override suspend fun queryDayDurationStats(params: DataDurationQueryParams): DataQueryTextResult =
        DataQueryTextResult(ok = true, outputText = "", message = "ok")

    override suspend fun queryProjectTree(params: DataTreeQueryParams): TreeQueryResult =
        TreeQueryResult(ok = true, found = false, message = "ok")

    override suspend fun queryProjectTreeText(params: DataTreeQueryParams): DataQueryTextResult =
        DataQueryTextResult(ok = true, outputText = "", message = "ok")

    override suspend fun queryReportChart(params: ReportChartQueryParams): ReportChartQueryResult {
        chartQueryCount += 1
        lastChartParams = params
        val chartData = ReportChartData(
            roots = listOf("sleep", "study"),
            selectedRoot = params.root.orEmpty(),
            lookbackDays = params.lookbackDays,
            points = listOf(
                ReportChartPoint(date = "2026-02-13", durationSeconds = 3600L),
                ReportChartPoint(date = "2026-02-14", durationSeconds = 5400L)
            ),
            usesLegacyStatsFallback = !includeCoreStats
        )
        val resolvedChartData = if (includeCoreStats) {
            chartData.copy(
                averageDurationSeconds = 4500L,
                totalDurationSeconds = 9000L,
                activeDays = 2,
                rangeDays = 2,
                usesLegacyStatsFallback = false
            )
        } else {
            chartData
        }
        return ReportChartQueryResult(
            ok = true,
            data = resolvedChartData,
            message = "ok"
        )
    }

    override suspend fun listActivityMappingNames(): ActivityMappingNamesResult =
        ActivityMappingNamesResult(ok = true, names = emptyList(), message = "ok")
}
