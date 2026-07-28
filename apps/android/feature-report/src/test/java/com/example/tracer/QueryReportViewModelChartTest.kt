package com.example.tracer

import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.CompletableDeferred
import kotlinx.coroutines.test.advanceUntilIdle
import kotlinx.coroutines.test.runCurrent
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
        viewModel.selectTrendChart()

        viewModel.onChartRootChange("study")
        viewModel.onReportModeChange(ReportMode.RECENT)
        viewModel.onReportRecentDaysChange("14")
        viewModel.onResultDisplayModeChange(ReportResultDisplayMode.CHART)
        advanceUntilIdle()

        assertEquals(1, fakeQueryGateway.chartQueryCount)
        assertEquals(0, fakeQueryGateway.compositionQueryCount)
        assertEquals("study", fakeQueryGateway.lastChartParams?.root)
        assertEquals(14, fakeQueryGateway.lastChartParams?.lookbackDays)
        assertEquals(null, fakeQueryGateway.lastChartParams?.fromDateIso)
        assertEquals(null, fakeQueryGateway.lastChartParams?.toDateIso)

        val state = viewModel.uiState
        assertEquals(ReportResultDisplayMode.CHART, state.resultDisplayMode)
        assertEquals(ReportChartSemanticMode.TREND, state.chartSemanticMode)
        assertTrue(!state.trendChartLoading)
        assertTrue(state.trendChartError.isEmpty())
        assertEquals(listOf("sleep", "study"), state.trendChartRoots)
        assertEquals("study", state.trendChartSelectedRoot)
        assertEquals(2, state.trendChartPoints.size)
        assertEquals("2026-02-13", state.trendChartPoints[0].date)
        assertEquals(4500L, state.trendChartAverageDurationSeconds)
        assertEquals(9000L, state.trendChartTotalDurationSeconds)
        assertEquals(2, state.trendChartActiveDays)
        assertEquals(2, state.trendChartRangeDays)
        assertEquals(false, state.trendChartUsesLegacyStatsFallback)
    }

    @Test
    fun switchToChart_invalidRecentDays_doesNotQueryGateway() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = FakeChartReportGateway(),
            queryGateway = fakeQueryGateway
        )
        viewModel.selectTrendChart()

        viewModel.onReportModeChange(ReportMode.RECENT)
        viewModel.onReportRecentDaysChange("0")
        viewModel.onResultDisplayModeChange(ReportResultDisplayMode.CHART)
        advanceUntilIdle()

        assertEquals(0, fakeQueryGateway.chartQueryCount)
        assertTrue(viewModel.uiState.trendChartError.isNotBlank())
    }

    @Test
    fun switchToChart_withRange_usesSharedRangeParams() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = FakeChartReportGateway(),
            queryGateway = fakeQueryGateway
        )
        viewModel.selectTrendChart()

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
        assertTrue(viewModel.uiState.trendChartError.isEmpty())
    }

    @Test
    fun switchToChart_withWeek_usesIsoWeekDateWindow() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = FakeChartReportGateway(),
            queryGateway = fakeQueryGateway
        )
        viewModel.selectTrendChart()

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
        viewModel.selectTrendChart()

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
        assertTrue(viewModel.uiState.trendChartError.isEmpty())
    }

    @Test
    fun changingReportParamsOutsideChart_invalidatesPreviousChartBeforeNextLoad() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = FakeChartReportGateway(),
            queryGateway = fakeQueryGateway
        )
        viewModel.selectTrendChart()

        viewModel.onReportModeChange(ReportMode.WEEK)
        viewModel.onReportWeekChange("202615")
        viewModel.onResultDisplayModeChange(ReportResultDisplayMode.CHART)
        advanceUntilIdle()
        assertEquals(1, fakeQueryGateway.chartQueryCount)
        assertTrue(viewModel.uiState.trendChartPoints.isNotEmpty())

        viewModel.onResultDisplayModeChange(ReportResultDisplayMode.TEXT)
        viewModel.onReportModeChange(ReportMode.DAY)
        viewModel.onReportDateChange("20260413")
        advanceUntilIdle()

        assertEquals(1, fakeQueryGateway.chartQueryCount)
        assertEquals(null, viewModel.uiState.trendChartRenderModel)
        assertTrue(viewModel.uiState.trendChartPoints.isEmpty())
        assertEquals(null, viewModel.uiState.compositionChartRenderModel)

        viewModel.onResultDisplayModeChange(ReportResultDisplayMode.CHART)
        advanceUntilIdle()

        assertEquals(1, fakeQueryGateway.chartQueryCount)
        assertEquals(1, fakeQueryGateway.compositionQueryCount)
        assertEquals("2026-04-13", fakeQueryGateway.lastCompositionParams?.fromDateIso)
        assertEquals("2026-04-13", fakeQueryGateway.lastCompositionParams?.toDateIso)
        assertEquals(1, fakeQueryGateway.lastCompositionParams?.lookbackDays)
        assertEquals(ReportChartSemanticMode.COMPOSITION, viewModel.uiState.chartSemanticMode)
    }

    @Test
    fun onChartRootChange_autoRefreshes_whenInChartMode() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = FakeChartReportGateway(),
            queryGateway = fakeQueryGateway
        )
        viewModel.selectTrendChart()

        viewModel.onReportModeChange(ReportMode.RECENT)
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
    fun delayedTrendResult_cannotOverwriteNewerDateSelection() = runTest {
        val firstResponse = CompletableDeferred<Unit>()
        val fakeQueryGateway = FakeChartQueryGateway(
            chartResponder = { params, requestNumber ->
                if (requestNumber == 1) {
                    firstResponse.await()
                    ReportChartQueryResult(
                        ok = true,
                        data = ReportChartData(
                            roots = listOf("sleep"),
                            selectedRoot = params.root.orEmpty(),
                            lookbackDays = params.lookbackDays,
                            points = emptyList()
                        ),
                        message = "no chart data"
                    )
                } else {
                    ReportChartQueryResult(
                        ok = true,
                        data = ReportChartData(
                            roots = listOf("study"),
                            selectedRoot = params.root.orEmpty(),
                            lookbackDays = params.lookbackDays,
                            points = listOf(
                                ReportChartPoint(
                                    date = "2026-04-13",
                                    durationSeconds = 3600L
                                )
                            )
                        ),
                        message = "ok"
                    )
                }
            }
        )
        val viewModel = QueryReportViewModel(
            reportGateway = FakeChartReportGateway(),
            queryGateway = fakeQueryGateway
        )
        viewModel.selectTrendChart()
        viewModel.onReportModeChange(ReportMode.WEEK)
        viewModel.onReportWeekChange("202615")
        viewModel.onResultDisplayModeChange(ReportResultDisplayMode.CHART)
        runCurrent()

        viewModel.onReportWeekChange("202616")
        runCurrent()
        assertEquals(2, fakeQueryGateway.chartQueryCount)
        assertEquals("2026-04-13", viewModel.uiState.trendChartPoints.single().date)

        firstResponse.complete(Unit)
        advanceUntilIdle()

        assertEquals("2026-04-13", viewModel.uiState.trendChartPoints.single().date)
        assertTrue(viewModel.uiState.trendChartError.isEmpty())
    }

    @Test
    fun changingReportModeInChartMode_doesNotStartTreeThatCanOverwriteChart() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = FakeChartReportGateway(),
            queryGateway = fakeQueryGateway
        )
        viewModel.applyPersistedReportPresentation(
            reportMode = ReportMode.WEEK,
            chartSemanticMode = ReportChartSemanticMode.TREND,
            resultDisplayMode = ReportResultDisplayMode.CHART,
            parameterSection = ReportParameterSection.ACTIVITY_HIERARCHY
        )
        viewModel.onReportWeekChange("202615")
        advanceUntilIdle()

        viewModel.onReportModeChange(ReportMode.MONTH)
        advanceUntilIdle()

        assertEquals(0, fakeQueryGateway.treeQueryCount)
        assertEquals(3, fakeQueryGateway.chartQueryCount)
        assertEquals(ReportMode.MONTH, viewModel.uiState.reportMode)
        assertTrue(!viewModel.uiState.trendChartLoading)
        assertEquals(2, viewModel.uiState.trendChartPoints.size)
    }

    @Test
    fun switchToChart_withoutCoreStats_usesDerivedStatsFromPipeline() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway(includeCoreStats = false)
        val viewModel = QueryReportViewModel(
            reportGateway = FakeChartReportGateway(),
            queryGateway = fakeQueryGateway
        )
        viewModel.selectTrendChart()

        viewModel.onReportModeChange(ReportMode.RECENT)
        viewModel.onResultDisplayModeChange(ReportResultDisplayMode.CHART)
        advanceUntilIdle()

        val state = viewModel.uiState
        assertEquals(2, state.trendChartPoints.size)
        assertEquals(4500L, state.trendChartAverageDurationSeconds)
        assertEquals(9000L, state.trendChartTotalDurationSeconds)
        assertEquals(2, state.trendChartActiveDays)
        assertEquals(2, state.trendChartRangeDays)
        assertEquals(true, state.trendChartUsesLegacyStatsFallback)
        assertNotNull(state.trendChartLastTrace)
        assertEquals(false, state.trendChartLastTrace?.cacheHit)
    }

    @Test
    fun switchToChart_withDay_usesCompositionQuery() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = FakeChartReportGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onReportModeChange(ReportMode.DAY)
        viewModel.onReportDateChange("20260413")
        viewModel.onResultDisplayModeChange(ReportResultDisplayMode.CHART)
        advanceUntilIdle()

        assertEquals(0, fakeQueryGateway.chartQueryCount)
        assertEquals(1, fakeQueryGateway.compositionQueryCount)
        assertEquals("2026-04-13", fakeQueryGateway.lastCompositionParams?.fromDateIso)
        assertEquals("2026-04-13", fakeQueryGateway.lastCompositionParams?.toDateIso)
        assertEquals(ReportChartSemanticMode.COMPOSITION, viewModel.uiState.chartSemanticMode)
        assertEquals(
            ReportCompositionVisualMode.HORIZONTAL_BAR,
            viewModel.uiState.compositionVisualMode
        )
        assertNotNull(viewModel.uiState.compositionChartRenderModel)
    }

    @Test
    fun coldStart_appliesPersistedChartSelectionBeforeGeneratingAReport() = runTest {
        val reportGateway = FakeChartReportGateway()
        val queryGateway = FakeChartQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = reportGateway,
            queryGateway = queryGateway
        )

        // The tab-enter callback runs before preference hydration on a cold start.
        viewModel.onReportTabEntered()
        // App language is also restored before the report presentation preference group.
        viewModel.onReportLocaleChange("ja")
        advanceUntilIdle()
        assertEquals(0, reportGateway.reportQueryCount)
        assertEquals(0, queryGateway.compositionQueryCount)

        viewModel.applyPersistedReportPresentation(
            reportMode = ReportMode.DAY,
            chartSemanticMode = ReportChartSemanticMode.COMPOSITION,
            resultDisplayMode = ReportResultDisplayMode.CHART,
            parameterSection = ReportParameterSection.DAY
        )
        advanceUntilIdle()

        assertEquals(0, reportGateway.reportQueryCount)
        assertEquals(1, queryGateway.compositionQueryCount)
        assertNotNull(viewModel.uiState.compositionChartRenderModel)
    }

    @Test
    fun chartAutoLoadsCompositionAndTrendForEverySupportedReportPeriod() = runTest {
        ReportMode.entries.forEach { reportMode ->
            val fakeQueryGateway = FakeChartQueryGateway()
            val viewModel = QueryReportViewModel(
                reportGateway = FakeChartReportGateway(),
                queryGateway = fakeQueryGateway
            )
            viewModel.configureValidChartPeriod(reportMode)

            viewModel.onResultDisplayModeChange(ReportResultDisplayMode.CHART)
            advanceUntilIdle()

            assertEquals(
                "composition should load automatically for $reportMode",
                1,
                fakeQueryGateway.compositionQueryCount
            )
            assertNotNull(viewModel.uiState.compositionChartRenderModel)

            if (reportMode != ReportMode.DAY) {
                // This is the path used when DataStore emits a persisted chart semantic
                // preference after the Chart display is already visible.
                viewModel.onPersistedChartSemanticModeChange(ReportChartSemanticMode.TREND)
                advanceUntilIdle()

                assertEquals(
                    "trend should load automatically for $reportMode",
                    1,
                    fakeQueryGateway.chartQueryCount
                )
                assertNotNull(viewModel.uiState.trendChartRenderModel)
            }
        }
    }

    @Test
    fun returningToReportChart_refreshesEveryReportPeriod() = runTest {
        ReportMode.entries.forEach { reportMode ->
            val fakeQueryGateway = FakeChartQueryGateway()
            val viewModel = QueryReportViewModel(
                reportGateway = FakeChartReportGateway(),
                queryGateway = fakeQueryGateway
            )
            viewModel.applyPersistedReportPresentation(
                reportMode = reportMode,
                chartSemanticMode = ReportChartSemanticMode.COMPOSITION,
                resultDisplayMode = ReportResultDisplayMode.TEXT,
                parameterSection = ReportParameterSection.DAY
            )
            advanceUntilIdle()
            viewModel.configureValidChartPeriod(reportMode)
            viewModel.onResultDisplayModeChange(ReportResultDisplayMode.CHART)
            advanceUntilIdle()

            if (reportMode != ReportMode.DAY) {
                viewModel.onPersistedChartSemanticModeChange(ReportChartSemanticMode.TREND)
                advanceUntilIdle()
            }
            viewModel.refreshReportDayDefault()
            advanceUntilIdle()

            if (reportMode == ReportMode.DAY) {
                assertEquals(
                    "returning to Report should regenerate $reportMode composition",
                    2,
                    fakeQueryGateway.compositionQueryCount
                )
            } else {
                assertEquals(
                    "returning to Report should regenerate $reportMode trend",
                    2,
                    fakeQueryGateway.chartQueryCount
                )
            }
        }
    }

    @Test
    fun switchingSemanticToComposition_queriesCompositionForNonDayModes() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = FakeChartReportGateway(),
            queryGateway = fakeQueryGateway
        )
        viewModel.selectTrendChart()

        viewModel.onReportModeChange(ReportMode.WEEK)
        viewModel.onReportWeekChange("202615")
        viewModel.onResultDisplayModeChange(ReportResultDisplayMode.CHART)
        advanceUntilIdle()
        assertEquals(1, fakeQueryGateway.chartQueryCount)

        viewModel.onChartSemanticModeChange(ReportChartSemanticMode.COMPOSITION)
        advanceUntilIdle()

        assertEquals(1, fakeQueryGateway.chartQueryCount)
        assertEquals(1, fakeQueryGateway.compositionQueryCount)
        assertEquals("2026-04-06", fakeQueryGateway.lastCompositionParams?.fromDateIso)
        assertEquals("2026-04-12", fakeQueryGateway.lastCompositionParams?.toDateIso)
        assertEquals(ReportChartSemanticMode.COMPOSITION, viewModel.uiState.chartSemanticMode)
    }

    @Test
    fun switchingCompositionVisualMode_doesNotReloadCompositionQuery() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = FakeChartReportGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onReportModeChange(ReportMode.DAY)
        viewModel.onReportDateChange("20260413")
        viewModel.onResultDisplayModeChange(ReportResultDisplayMode.CHART)
        advanceUntilIdle()
        assertEquals(1, fakeQueryGateway.compositionQueryCount)

        viewModel.onCompositionVisualModeChange(ReportCompositionVisualMode.PIE)
        advanceUntilIdle()

        assertEquals(1, fakeQueryGateway.compositionQueryCount)
        assertEquals(ReportCompositionVisualMode.PIE, viewModel.uiState.compositionVisualMode)
    }

    @Test
    fun dayComposition_canSwitchToTreemapWithoutReloadingCompositionQuery() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = FakeChartReportGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onReportModeChange(ReportMode.DAY)
        viewModel.onReportDateChange("20260413")
        viewModel.onResultDisplayModeChange(ReportResultDisplayMode.CHART)
        advanceUntilIdle()
        assertEquals(1, fakeQueryGateway.compositionQueryCount)

        viewModel.onCompositionVisualModeChange(ReportCompositionVisualMode.TREEMAP)
        advanceUntilIdle()

        assertEquals(1, fakeQueryGateway.compositionQueryCount)
        assertEquals(ReportCompositionVisualMode.TREEMAP, viewModel.uiState.compositionVisualMode)
    }

    @Test
    fun changingReportParamsInChartMode_preservesCompositionVisualModeWhileReloading() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = FakeChartReportGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onReportModeChange(ReportMode.DAY)
        viewModel.onReportDateChange("20260413")
        viewModel.onResultDisplayModeChange(ReportResultDisplayMode.CHART)
        advanceUntilIdle()
        viewModel.onCompositionVisualModeChange(ReportCompositionVisualMode.PIE)

        viewModel.onReportDateChange("20260414")
        advanceUntilIdle()

        assertEquals(2, fakeQueryGateway.compositionQueryCount)
        assertEquals("2026-04-14", fakeQueryGateway.lastCompositionParams?.fromDateIso)
        assertEquals("2026-04-14", fakeQueryGateway.lastCompositionParams?.toDateIso)
        assertEquals(ReportCompositionVisualMode.PIE, viewModel.uiState.compositionVisualMode)
    }

    @Test
    fun nonDayComposition_canUseHorizontalBarWithoutReloadingQuery() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = FakeChartReportGateway(),
            queryGateway = fakeQueryGateway
        )
        viewModel.selectTrendChart()

        viewModel.onReportModeChange(ReportMode.WEEK)
        viewModel.onReportWeekChange("202615")
        viewModel.onResultDisplayModeChange(ReportResultDisplayMode.CHART)
        advanceUntilIdle()
        assertEquals(1, fakeQueryGateway.chartQueryCount)

        viewModel.onChartSemanticModeChange(ReportChartSemanticMode.COMPOSITION)
        advanceUntilIdle()
        assertEquals(1, fakeQueryGateway.compositionQueryCount)

        viewModel.onCompositionVisualModeChange(ReportCompositionVisualMode.HORIZONTAL_BAR)
        advanceUntilIdle()

        assertEquals(1, fakeQueryGateway.compositionQueryCount)
        assertEquals(
            ReportCompositionVisualMode.HORIZONTAL_BAR,
            viewModel.uiState.compositionVisualMode
        )
    }

    @Test
    fun changingReportPeriod_preservesTreemapForComposition() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = FakeChartReportGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onReportModeChange(ReportMode.DAY)
        viewModel.onResultDisplayModeChange(ReportResultDisplayMode.CHART)
        advanceUntilIdle()
        viewModel.onCompositionVisualModeChange(ReportCompositionVisualMode.TREEMAP)

        listOf(
            ReportMode.WEEK,
            ReportMode.MONTH,
            ReportMode.YEAR,
            ReportMode.RANGE,
            ReportMode.RECENT
        ).forEach { reportMode ->
            viewModel.onReportModeChange(reportMode)

            assertEquals(ReportCompositionVisualMode.TREEMAP, viewModel.uiState.compositionVisualMode)
        }
    }
}

private fun QueryReportViewModel.selectTrendChart() {
    onPersistedChartSemanticModeChange(ReportChartSemanticMode.TREND)
}

private fun QueryReportViewModel.configureValidChartPeriod(reportMode: ReportMode) {
    onReportModeChange(reportMode)
    when (reportMode) {
        ReportMode.DAY -> onReportDateChange("20260413")
        ReportMode.WEEK -> onReportWeekChange("202615")
        ReportMode.MONTH -> onReportMonthChange("202604")
        ReportMode.YEAR -> onReportYearChange("2026")
        ReportMode.RANGE -> {
            onReportRangeStartDateChange("20260410")
            onReportRangeEndDateChange("20260413")
        }
        ReportMode.RECENT -> onReportRecentDaysChange("7")
    }
}

private class FakeChartReportGateway : ReportGateway {
    var reportQueryCount: Int = 0

    override suspend fun reportMarkdown(request: TemporalReportQueryRequest): ReportCallResult =
        ReportCallResult(
            initialized = true,
            operationOk = true,
            outputText = "",
            rawResponse = ""
        ).also { reportQueryCount += 1 }
}

private class FakeChartQueryGateway(
    private val includeCoreStats: Boolean = true,
    private val chartResponder: (suspend (ReportChartQueryParams, Int) -> ReportChartQueryResult)? =
        null
) : QueryGateway {
    var chartQueryCount: Int = 0
    var lastChartParams: ReportChartQueryParams? = null
    var compositionQueryCount: Int = 0
    var lastCompositionParams: ReportCompositionQueryParams? = null
    var treeQueryCount: Int = 0

    override suspend fun queryActivitySuggestions(
        lookbackDays: Int,
        topN: Int,
        anchorDateIso: String?
    ): ActivitySuggestionResult = ActivitySuggestionResult(
        ok = true,
        suggestions = emptyList(),
        message = "ok"
    )

    override suspend fun queryDayDurations(params: DataDurationQueryParams): DataQueryTextResult =
        DataQueryTextResult(ok = true, outputText = "", message = "ok")

    override suspend fun queryDayDurationStats(params: DataDurationQueryParams): DataQueryTextResult =
        DataQueryTextResult(ok = true, outputText = "", message = "ok")

    override suspend fun queryProjectTree(params: DataTreeQueryParams): TreeQueryResult {
        treeQueryCount += 1
        return TreeQueryResult(ok = true, found = false, message = "ok")
    }

    override suspend fun queryReportChart(params: ReportChartQueryParams): ReportChartQueryResult {
        chartQueryCount += 1
        lastChartParams = params
        chartResponder?.let { responder ->
            return responder(params, chartQueryCount)
        }
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

    override suspend fun queryReportComposition(
        params: ReportCompositionQueryParams
    ): ReportCompositionQueryResult {
        compositionQueryCount += 1
        lastCompositionParams = params
        return ReportCompositionQueryResult(
            ok = true,
            data = ReportCompositionData(
                tree = listOf(
                    TreeNode(name = "study", durationSeconds = 5400L),
                    TreeNode(name = "sleep", durationSeconds = 3600L)
                ),
                totalDurationSeconds = 9000L,
                activeRootCount = 2,
                rangeDays = params.lookbackDays
            ),
            message = "ok"
        )
    }

    override suspend fun listActivityMappingNames(): ActivityMappingNamesResult =
        ActivityMappingNamesResult(ok = true, names = emptyList(), message = "ok")
}
