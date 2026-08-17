package com.example.tracer

import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.CompletableDeferred
import kotlinx.coroutines.test.advanceUntilIdle
import kotlinx.coroutines.test.runCurrent
import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertTrue
import org.junit.Rule
import org.junit.Test

@OptIn(ExperimentalCoroutinesApi::class)
class QueryInsightsViewModelChartTest {
    @get:Rule
    val mainDispatcherRule = MainDispatcherRule()

    @Test
    fun switchToChart_autoLoads_withCurrentRootAndRecentMode() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeChartInsightsGateway(),
            queryGateway = fakeQueryGateway
        )
        viewModel.selectTrendChart()

        viewModel.onChartRootChange("study")
        viewModel.onInsightsModeChange(InsightsMode.RECENT)
        viewModel.onInsightsRecentDaysChange("14")
        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
        advanceUntilIdle()

        assertEquals(1, fakeQueryGateway.chartQueryCount)
        assertEquals(0, fakeQueryGateway.compositionQueryCount)
        assertEquals("study", fakeQueryGateway.lastChartParams?.root)
        assertEquals(14, fakeQueryGateway.lastChartParams?.lookbackDays)
        assertEquals(null, fakeQueryGateway.lastChartParams?.fromDateIso)
        assertEquals(null, fakeQueryGateway.lastChartParams?.toDateIso)

        val state = viewModel.uiState
        assertEquals(InsightsResultDisplayMode.CHART, state.resultDisplayMode)
        assertEquals(InsightsChartSemanticMode.TREND, state.chartSemanticMode)
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
    }

    @Test
    fun switchToChart_invalidRecentDays_doesNotQueryGateway() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeChartInsightsGateway(),
            queryGateway = fakeQueryGateway
        )
        viewModel.selectTrendChart()

        viewModel.onInsightsModeChange(InsightsMode.RECENT)
        viewModel.onInsightsRecentDaysChange("0")
        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
        advanceUntilIdle()

        assertEquals(0, fakeQueryGateway.chartQueryCount)
        assertTrue(viewModel.uiState.trendChartError.isNotBlank())
    }

    @Test
    fun switchToChart_withRange_usesSharedRangeParams() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeChartInsightsGateway(),
            queryGateway = fakeQueryGateway
        )
        viewModel.selectTrendChart()

        viewModel.onInsightsModeChange(InsightsMode.RANGE)
        viewModel.onInsightsRangeStartDateChange("20260210")
        viewModel.onInsightsRangeEndDateChange("20260214")
        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
        advanceUntilIdle()

        assertEquals(1, fakeQueryGateway.chartQueryCount)
        assertEquals("2026-02-10", fakeQueryGateway.lastChartParams?.fromDateIso)
        assertEquals("2026-02-14", fakeQueryGateway.lastChartParams?.toDateIso)
        assertEquals("20260210", viewModel.uiState.insightsRangeStartDate)
        assertEquals("20260214", viewModel.uiState.insightsRangeEndDate)
        assertTrue(viewModel.uiState.trendChartError.isEmpty())
    }

    @Test
    fun switchToChart_withWeek_usesIsoWeekDateWindow() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeChartInsightsGateway(),
            queryGateway = fakeQueryGateway
        )
        viewModel.selectTrendChart()

        viewModel.onInsightsModeChange(InsightsMode.WEEK)
        viewModel.onInsightsWeekChange("202615")
        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
        advanceUntilIdle()

        assertEquals(1, fakeQueryGateway.chartQueryCount)
        assertEquals("2026-04-06", fakeQueryGateway.lastChartParams?.fromDateIso)
        assertEquals("2026-04-12", fakeQueryGateway.lastChartParams?.toDateIso)
        assertEquals(7, fakeQueryGateway.lastChartParams?.lookbackDays)
    }

    @Test
    fun changingInsightsModeInChartMode_invalidatesAndReloadsChart() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeChartInsightsGateway(),
            queryGateway = fakeQueryGateway
        )
        viewModel.selectTrendChart()

        viewModel.onInsightsMonthChange("202602")
        viewModel.onInsightsModeChange(InsightsMode.WEEK)
        viewModel.onInsightsWeekChange("202615")
        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
        advanceUntilIdle()

        assertEquals(1, fakeQueryGateway.chartQueryCount)
        assertEquals("2026-04-06", fakeQueryGateway.lastChartParams?.fromDateIso)
        assertEquals("2026-04-12", fakeQueryGateway.lastChartParams?.toDateIso)

        viewModel.onInsightsModeChange(InsightsMode.MONTH)
        advanceUntilIdle()

        assertEquals(2, fakeQueryGateway.chartQueryCount)
        assertEquals("2026-02-01", fakeQueryGateway.lastChartParams?.fromDateIso)
        assertEquals("2026-02-28", fakeQueryGateway.lastChartParams?.toDateIso)
        assertEquals(28, fakeQueryGateway.lastChartParams?.lookbackDays)
        assertTrue(viewModel.uiState.trendChartError.isEmpty())
    }

    @Test
    fun changingInsightsParamsOutsideChart_invalidatesPreviousChartBeforeNextLoad() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeChartInsightsGateway(),
            queryGateway = fakeQueryGateway
        )
        viewModel.selectTrendChart()

        viewModel.onInsightsModeChange(InsightsMode.WEEK)
        viewModel.onInsightsWeekChange("202615")
        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
        advanceUntilIdle()
        assertEquals(1, fakeQueryGateway.chartQueryCount)
        assertTrue(viewModel.uiState.trendChartPoints.isNotEmpty())

        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.DETAILS)
        viewModel.onInsightsModeChange(InsightsMode.DAY)
        viewModel.onInsightsDateChange("20260413")
        advanceUntilIdle()

        assertEquals(1, fakeQueryGateway.chartQueryCount)
        assertEquals(null, viewModel.uiState.trendChartRenderModel)
        assertTrue(viewModel.uiState.trendChartPoints.isEmpty())
        assertEquals(null, viewModel.uiState.compositionChartRenderModel)

        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
        advanceUntilIdle()

        assertEquals(1, fakeQueryGateway.chartQueryCount)
        assertEquals(1, fakeQueryGateway.compositionQueryCount)
        assertEquals("2026-04-13", fakeQueryGateway.lastCompositionParams?.fromDateIso)
        assertEquals("2026-04-13", fakeQueryGateway.lastCompositionParams?.toDateIso)
        assertEquals(1, fakeQueryGateway.lastCompositionParams?.lookbackDays)
        assertEquals(InsightsChartSemanticMode.COMPOSITION, viewModel.uiState.chartSemanticMode)
    }

    @Test
    fun onChartRootChange_autoRefreshes_whenInChartMode() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeChartInsightsGateway(),
            queryGateway = fakeQueryGateway
        )
        viewModel.selectTrendChart()

        viewModel.onInsightsModeChange(InsightsMode.RECENT)
        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
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
                    InsightsChartQueryResult(
                        ok = true,
                        data = InsightsChartData(
                            roots = listOf("sleep"),
                            selectedRoot = params.root.orEmpty(),
                            lookbackDays = params.lookbackDays,
                            points = emptyList()
                        ),
                        message = "no chart data"
                    )
                } else {
                    InsightsChartQueryResult(
                        ok = true,
                        data = InsightsChartData(
                            roots = listOf("study"),
                            selectedRoot = params.root.orEmpty(),
                            lookbackDays = params.lookbackDays,
                            points = listOf(
                                InsightsChartPoint(
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
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeChartInsightsGateway(),
            queryGateway = fakeQueryGateway
        )
        viewModel.selectTrendChart()
        viewModel.onInsightsModeChange(InsightsMode.WEEK)
        viewModel.onInsightsWeekChange("202615")
        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
        runCurrent()

        viewModel.onInsightsWeekChange("202616")
        runCurrent()
        assertEquals(2, fakeQueryGateway.chartQueryCount)
        assertEquals("2026-04-13", viewModel.uiState.trendChartPoints.single().date)

        firstResponse.complete(Unit)
        advanceUntilIdle()

        assertEquals("2026-04-13", viewModel.uiState.trendChartPoints.single().date)
        assertTrue(viewModel.uiState.trendChartError.isEmpty())
    }

    @Test
    fun changingInsightsModeInChartMode_doesNotStartTreeThatCanOverwriteChart() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeChartInsightsGateway(),
            queryGateway = fakeQueryGateway
        )
        viewModel.applyPersistedInsightsPresentation(
            insightsMode = InsightsMode.WEEK,
            chartSemanticMode = InsightsChartSemanticMode.TREND,
            resultDisplayMode = InsightsResultDisplayMode.CHART,
            parameterSection = InsightsParameterSection.ACTIVITY_HIERARCHY
        )
        viewModel.onInsightsWeekChange("202615")
        advanceUntilIdle()

        viewModel.onInsightsModeChange(InsightsMode.MONTH)
        advanceUntilIdle()

        assertEquals(0, fakeQueryGateway.treeQueryCount)
        assertEquals(3, fakeQueryGateway.chartQueryCount)
        assertEquals(InsightsMode.MONTH, viewModel.uiState.insightsMode)
        assertTrue(!viewModel.uiState.trendChartLoading)
        assertEquals(2, viewModel.uiState.trendChartPoints.size)
    }

    @Test
    fun switchToChart_withDay_usesCompositionQuery() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeChartInsightsGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onInsightsModeChange(InsightsMode.DAY)
        viewModel.onInsightsDateChange("20260413")
        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
        advanceUntilIdle()

        assertEquals(0, fakeQueryGateway.chartQueryCount)
        assertEquals(1, fakeQueryGateway.compositionQueryCount)
        assertEquals("2026-04-13", fakeQueryGateway.lastCompositionParams?.fromDateIso)
        assertEquals("2026-04-13", fakeQueryGateway.lastCompositionParams?.toDateIso)
        assertEquals(InsightsChartSemanticMode.COMPOSITION, viewModel.uiState.chartSemanticMode)
        assertEquals(
            InsightsCompositionVisualMode.HORIZONTAL_BAR,
            viewModel.uiState.compositionVisualMode
        )
        assertNotNull(viewModel.uiState.compositionChartRenderModel)
    }

    @Test
    fun coldStart_appliesPersistedChartSelectionBeforeGeneratingAInsights() = runTest {
        val insightsGateway = FakeChartInsightsGateway()
        val queryGateway = FakeChartQueryGateway()
        val viewModel = QueryInsightsViewModel(
            insightsGateway = insightsGateway,
            queryGateway = queryGateway
        )
        assertFalse(viewModel.uiState.isPresentationRestored)

        // The tab-enter callback runs before preference hydration on a cold start.
        viewModel.onInsightsTabEntered()
        // App language is also restored before the insights presentation preference group.
        viewModel.onInsightsLocaleChange("ja")
        advanceUntilIdle()
        assertEquals(0, insightsGateway.insightsQueryCount)
        assertEquals(0, queryGateway.compositionQueryCount)

        viewModel.applyPersistedInsightsPresentation(
            insightsMode = InsightsMode.DAY,
            chartSemanticMode = InsightsChartSemanticMode.COMPOSITION,
            resultDisplayMode = InsightsResultDisplayMode.CHART,
            parameterSection = InsightsParameterSection.DAY
        )
        advanceUntilIdle()

        assertTrue(viewModel.uiState.isPresentationRestored)
        assertEquals(0, insightsGateway.insightsQueryCount)
        assertEquals(1, queryGateway.compositionQueryCount)
        assertNotNull(viewModel.uiState.compositionChartRenderModel)

        viewModel.onInsightsModeChange(InsightsMode.MONTH)
        assertTrue(viewModel.uiState.isPresentationRestored)
    }

    @Test
    fun chartAutoLoadsCompositionAndTrendForEverySupportedInsightsPeriod() = runTest {
        InsightsMode.entries.forEach { insightsMode ->
            val fakeQueryGateway = FakeChartQueryGateway()
            val viewModel = QueryInsightsViewModel(
                insightsGateway = FakeChartInsightsGateway(),
                queryGateway = fakeQueryGateway
            )
            viewModel.configureValidChartPeriod(insightsMode)

            viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
            advanceUntilIdle()

            assertEquals(
                "composition should load automatically for $insightsMode",
                1,
                fakeQueryGateway.compositionQueryCount
            )
            assertNotNull(viewModel.uiState.compositionChartRenderModel)

            if (insightsMode != InsightsMode.DAY) {
                // This is the path used when DataStore emits a persisted chart semantic
                // preference after the Chart display is already visible.
                viewModel.onPersistedChartSemanticModeChange(InsightsChartSemanticMode.TREND)
                advanceUntilIdle()

                assertEquals(
                    "trend should load automatically for $insightsMode",
                    1,
                    fakeQueryGateway.chartQueryCount
                )
                assertNotNull(viewModel.uiState.trendChartRenderModel)
            }
        }
    }

    @Test
    fun returningToInsightsChart_refreshesEveryInsightsPeriod() = runTest {
        InsightsMode.entries.forEach { insightsMode ->
            val fakeQueryGateway = FakeChartQueryGateway()
            val viewModel = QueryInsightsViewModel(
                insightsGateway = FakeChartInsightsGateway(),
                queryGateway = fakeQueryGateway
            )
            viewModel.applyPersistedInsightsPresentation(
                insightsMode = insightsMode,
                chartSemanticMode = InsightsChartSemanticMode.COMPOSITION,
                resultDisplayMode = InsightsResultDisplayMode.DETAILS,
                parameterSection = InsightsParameterSection.DAY
            )
            advanceUntilIdle()
            viewModel.configureValidChartPeriod(insightsMode)
            viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
            advanceUntilIdle()

            if (insightsMode != InsightsMode.DAY) {
                viewModel.onPersistedChartSemanticModeChange(InsightsChartSemanticMode.TREND)
                advanceUntilIdle()
            }
            viewModel.refreshInsightsDayDefault()
            advanceUntilIdle()

            if (insightsMode == InsightsMode.DAY) {
                assertEquals(
                    "returning to Insights should regenerate $insightsMode composition",
                    2,
                    fakeQueryGateway.compositionQueryCount
                )
            } else {
                assertEquals(
                    "returning to Insights should regenerate $insightsMode trend",
                    2,
                    fakeQueryGateway.chartQueryCount
                )
            }
        }
    }

    @Test
    fun switchingSemanticToComposition_queriesCompositionForNonDayModes() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeChartInsightsGateway(),
            queryGateway = fakeQueryGateway
        )
        viewModel.selectTrendChart()

        viewModel.onInsightsModeChange(InsightsMode.WEEK)
        viewModel.onInsightsWeekChange("202615")
        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
        advanceUntilIdle()
        assertEquals(1, fakeQueryGateway.chartQueryCount)

        viewModel.onChartSemanticModeChange(InsightsChartSemanticMode.COMPOSITION)
        advanceUntilIdle()

        assertEquals(1, fakeQueryGateway.chartQueryCount)
        assertEquals(1, fakeQueryGateway.compositionQueryCount)
        assertEquals("2026-04-06", fakeQueryGateway.lastCompositionParams?.fromDateIso)
        assertEquals("2026-04-12", fakeQueryGateway.lastCompositionParams?.toDateIso)
        assertEquals(InsightsChartSemanticMode.COMPOSITION, viewModel.uiState.chartSemanticMode)
    }

    @Test
    fun switchingCompositionVisualMode_doesNotReloadCompositionQuery() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeChartInsightsGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onInsightsModeChange(InsightsMode.DAY)
        viewModel.onInsightsDateChange("20260413")
        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
        advanceUntilIdle()
        assertEquals(1, fakeQueryGateway.compositionQueryCount)

        viewModel.onCompositionVisualModeChange(InsightsCompositionVisualMode.PIE)
        advanceUntilIdle()

        assertEquals(1, fakeQueryGateway.compositionQueryCount)
        assertEquals(InsightsCompositionVisualMode.PIE, viewModel.uiState.compositionVisualMode)
    }

    @Test
    fun dayComposition_canSwitchToTreemapWithoutReloadingCompositionQuery() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeChartInsightsGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onInsightsModeChange(InsightsMode.DAY)
        viewModel.onInsightsDateChange("20260413")
        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
        advanceUntilIdle()
        assertEquals(1, fakeQueryGateway.compositionQueryCount)

        viewModel.onCompositionVisualModeChange(InsightsCompositionVisualMode.TREEMAP)
        advanceUntilIdle()

        assertEquals(1, fakeQueryGateway.compositionQueryCount)
        assertEquals(InsightsCompositionVisualMode.TREEMAP, viewModel.uiState.compositionVisualMode)
    }

    @Test
    fun changingInsightsParamsInChartMode_preservesCompositionVisualModeWhileReloading() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeChartInsightsGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onInsightsModeChange(InsightsMode.DAY)
        viewModel.onInsightsDateChange("20260413")
        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
        advanceUntilIdle()
        viewModel.onCompositionVisualModeChange(InsightsCompositionVisualMode.PIE)

        viewModel.onInsightsDateChange("20260414")
        advanceUntilIdle()

        assertEquals(2, fakeQueryGateway.compositionQueryCount)
        assertEquals("2026-04-14", fakeQueryGateway.lastCompositionParams?.fromDateIso)
        assertEquals("2026-04-14", fakeQueryGateway.lastCompositionParams?.toDateIso)
        assertEquals(InsightsCompositionVisualMode.PIE, viewModel.uiState.compositionVisualMode)
    }

    @Test
    fun nonDayComposition_canUseHorizontalBarWithoutReloadingQuery() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeChartInsightsGateway(),
            queryGateway = fakeQueryGateway
        )
        viewModel.selectTrendChart()

        viewModel.onInsightsModeChange(InsightsMode.WEEK)
        viewModel.onInsightsWeekChange("202615")
        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
        advanceUntilIdle()
        assertEquals(1, fakeQueryGateway.chartQueryCount)

        viewModel.onChartSemanticModeChange(InsightsChartSemanticMode.COMPOSITION)
        advanceUntilIdle()
        assertEquals(1, fakeQueryGateway.compositionQueryCount)

        viewModel.onCompositionVisualModeChange(InsightsCompositionVisualMode.HORIZONTAL_BAR)
        advanceUntilIdle()

        assertEquals(1, fakeQueryGateway.compositionQueryCount)
        assertEquals(
            InsightsCompositionVisualMode.HORIZONTAL_BAR,
            viewModel.uiState.compositionVisualMode
        )
    }

    @Test
    fun changingInsightsPeriod_preservesTreemapForComposition() = runTest {
        val fakeQueryGateway = FakeChartQueryGateway()
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeChartInsightsGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onInsightsModeChange(InsightsMode.DAY)
        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
        advanceUntilIdle()
        viewModel.onCompositionVisualModeChange(InsightsCompositionVisualMode.TREEMAP)

        listOf(
            InsightsMode.WEEK,
            InsightsMode.MONTH,
            InsightsMode.YEAR,
            InsightsMode.RANGE,
            InsightsMode.RECENT
        ).forEach { insightsMode ->
            viewModel.onInsightsModeChange(insightsMode)

            assertEquals(InsightsCompositionVisualMode.TREEMAP, viewModel.uiState.compositionVisualMode)
        }
    }
}

private fun QueryInsightsViewModel.selectTrendChart() {
    onPersistedChartSemanticModeChange(InsightsChartSemanticMode.TREND)
}

private fun QueryInsightsViewModel.configureValidChartPeriod(insightsMode: InsightsMode) {
    onInsightsModeChange(insightsMode)
    when (insightsMode) {
        InsightsMode.DAY -> onInsightsDateChange("20260413")
        InsightsMode.WEEK -> onInsightsWeekChange("202615")
        InsightsMode.MONTH -> onInsightsMonthChange("202604")
        InsightsMode.YEAR -> onInsightsYearChange("2026")
        InsightsMode.RANGE -> {
            onInsightsRangeStartDateChange("20260410")
            onInsightsRangeEndDateChange("20260413")
        }
        InsightsMode.RECENT -> onInsightsRecentDaysChange("7")
    }
}

private class FakeChartInsightsGateway : InsightsGateway {
    var insightsQueryCount: Int = 0

    override suspend fun insightsMarkdown(request: TemporalInsightsQueryRequest): InsightsCallResult =
        InsightsCallResult(
            initialized = true,
            operationOk = true,
            outputText = "",
            rawResponse = ""
        ).also { insightsQueryCount += 1 }
}

private class FakeChartQueryGateway(
    private val chartResponder: (suspend (InsightsChartQueryParams, Int) -> InsightsChartQueryResult)? =
        null
) : QueryGateway {
    var chartQueryCount: Int = 0
    var lastChartParams: InsightsChartQueryParams? = null
    var compositionQueryCount: Int = 0
    var lastCompositionParams: InsightsCompositionQueryParams? = null
    var treeQueryCount: Int = 0

    override suspend fun queryFrequentActivities(
        lookbackDays: Int,
        topN: Int,
        anchorDateIso: String?
    ): ActivityFrequentResult = ActivityFrequentResult(
        ok = true,
        frequentActivities = emptyList(),
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

    override suspend fun queryInsightsChart(params: InsightsChartQueryParams): InsightsChartQueryResult {
        chartQueryCount += 1
        lastChartParams = params
        chartResponder?.let { responder ->
            return responder(params, chartQueryCount)
        }
        val chartData = InsightsChartData(
            roots = listOf("sleep", "study"),
            selectedRoot = params.root.orEmpty(),
            lookbackDays = params.lookbackDays,
            points = listOf(
                InsightsChartPoint(date = "2026-02-13", durationSeconds = 3600L),
                InsightsChartPoint(date = "2026-02-14", durationSeconds = 5400L)
            ),
            averageDurationSeconds = 4500L,
            totalDurationSeconds = 9000L,
            activeDays = 2,
            rangeDays = 2
        )
        return InsightsChartQueryResult(
            ok = true,
            data = chartData,
            message = "ok"
        )
    }

    override suspend fun queryInsightsComposition(
        params: InsightsCompositionQueryParams
    ): InsightsCompositionQueryResult {
        compositionQueryCount += 1
        lastCompositionParams = params
        return InsightsCompositionQueryResult(
            ok = true,
            data = InsightsCompositionData(
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
