package com.example.tracer

import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.test.advanceUntilIdle
import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Rule
import org.junit.Test
import java.time.Clock
import java.time.Instant
import java.time.ZoneId

@OptIn(ExperimentalCoroutinesApi::class)
class QueryInsightsViewModelInsightsSyncTest {
    @get:Rule
    val mainDispatcherRule = MainDispatcherRule()

    @Test
    fun validDateChange_automaticallyGeneratesMarkdownInsights() = runTest {
        val fakeInsightsGateway = FakeStructuredInsightsGateway().apply {
            dayResult = InsightsCallResult(
                initialized = true,
                operationOk = true,
                outputText = "# Automatically generated",
                rawResponse = """{"ok":true}"""
            )
        }
        val viewModel = QueryInsightsViewModel(
            insightsGateway = fakeInsightsGateway,
            queryGateway = FakeInsightsSyncQueryGateway()
        )

        viewModel.onInsightsDateChange("20260214")
        advanceUntilIdle()

        assertEquals(
            "# Automatically generated",
            (viewModel.uiState.activeResult as QueryResult.Insights).text
        )
    }

    @Test
    fun switchingBackToText_requeriesCurrentWeekInsights() = runTest {
        val fakeInsightsGateway = FakeStructuredInsightsGateway()
        val viewModel = QueryInsightsViewModel(
            insightsGateway = fakeInsightsGateway,
            queryGateway = FakeInsightsSyncQueryGateway()
        )

        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
        viewModel.onInsightsModeChange(InsightsMode.WEEK)
        viewModel.onInsightsWeekChange("202615")
        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.TEXT)
        advanceUntilIdle()

        assertEquals(InsightsDisplayMode.WEEK, fakeInsightsGateway.lastTemporalRequest?.displayMode)
    }

    @Test
    fun insightsDay_missingTarget_isPresentedAsNormalNoData() = runTest {
        val fakeInsightsGateway = FakeStructuredInsightsGateway().apply {
            dayResult = InsightsCallResult(
                initialized = true,
                operationOk = false,
                outputText = "runtime insights failed. [op=day-missing]",
                rawResponse = """{"ok":false}""",
                errorContract = InsightsErrorContract(
                    errorCode = "insights.target.not_found",
                    errorCategory = "insights",
                    hints = listOf("Try another date.")
                )
            )
        }
        val viewModel = QueryInsightsViewModel(
            insightsGateway = fakeInsightsGateway,
            queryGateway = FakeInsightsSyncQueryGateway()
        )

        viewModel.onInsightsDateChange("20260214")
        viewModel.insightsDay()
        advanceUntilIdle()

        val state = viewModel.uiState
        assertNull(state.activeResult)
        assertTrue(DataTreePeriod.DAY !in state.insightsResultsByPeriod)
        assertTrue(DataTreePeriod.DAY !in state.insightsErrorsByPeriod)
        val summary = state.insightsSummariesByPeriod[DataTreePeriod.DAY]
        assertTrue(summary is InsightsSummary.NoData)
        assertEquals(DataTreePeriod.DAY, (summary as InsightsSummary.NoData).period)
        assertTrue(state.statusText.contains("No records for this day"))
    }

    @Test
    fun insightsRecent_emptyWindow_keepsMarkdownAndExposesWindowSummary() = runTest {
        val metadata = InsightsWindowMetadata(
            hasRecords = false,
            matchedDayCount = 0,
            matchedRecordCount = 0,
            startDate = "2026-02-01",
            endDate = "2026-02-07",
            requestedDays = 7
        )
        val fakeInsightsGateway = FakeStructuredInsightsGateway().apply {
            recentResult = InsightsCallResult(
                initialized = true,
                operationOk = true,
                outputText = "## Recent Insights\n\nNo rows.",
                rawResponse = """{"ok":true}""",
                insightsWindowMetadata = metadata
            )
        }
        val viewModel = QueryInsightsViewModel(
            insightsGateway = fakeInsightsGateway,
            queryGateway = FakeInsightsSyncQueryGateway()
        )

        viewModel.onInsightsRecentDaysChange("7")
        viewModel.insightsRecent()
        advanceUntilIdle()

        val state = viewModel.uiState
        assertTrue(state.activeResult is QueryResult.Insights)
        val activeResult = state.activeResult as QueryResult.Insights
        assertEquals("## Recent Insights\n\nNo rows.", activeResult.text)
        assertTrue(activeResult.summary is InsightsSummary.WindowMetadata)
        val summary = activeResult.summary as InsightsSummary.WindowMetadata
        assertEquals(false, summary.metadata.hasRecords)
        assertEquals(7, summary.metadata.requestedDays)
        assertTrue(DataTreePeriod.RECENT !in state.insightsErrorsByPeriod)
        assertEquals(activeResult, state.insightsResultsByPeriod[DataTreePeriod.RECENT])
        assertTrue(state.statusText.contains("empty window"))
    }

    @Test
    fun insightsResults_cacheSummaryPerPeriod() = runTest {
        val fakeInsightsGateway = FakeStructuredInsightsGateway().apply {
            dayResult = InsightsCallResult(
                initialized = true,
                operationOk = true,
                outputText = "# Day Insights",
                rawResponse = """{"ok":true}"""
            )
            recentResult = InsightsCallResult(
                initialized = true,
                operationOk = true,
                outputText = "# Recent Insights",
                rawResponse = """{"ok":true}""",
                insightsWindowMetadata = InsightsWindowMetadata(
                    hasRecords = true,
                    matchedDayCount = 3,
                    matchedRecordCount = 9,
                    startDate = "2026-02-01",
                    endDate = "2026-02-07",
                    requestedDays = 7
                )
            )
        }
        val viewModel = QueryInsightsViewModel(
            insightsGateway = fakeInsightsGateway,
            queryGateway = FakeInsightsSyncQueryGateway()
        )

        viewModel.onInsightsDateChange("20260214")
        viewModel.insightsDay()
        advanceUntilIdle()

        viewModel.onInsightsRecentDaysChange("7")
        viewModel.insightsRecent()
        advanceUntilIdle()

        assertEquals("# Day Insights", viewModel.uiState.insightsResultsByPeriod[DataTreePeriod.DAY]?.text)
        assertEquals(
            null,
            viewModel.uiState.insightsResultsByPeriod[DataTreePeriod.DAY]?.summary
        )
        assertTrue(
            viewModel.uiState.insightsResultsByPeriod[DataTreePeriod.RECENT]?.summary is
                InsightsSummary.WindowMetadata
        )
        assertTrue(
            viewModel.uiState.insightsSummariesByPeriod[DataTreePeriod.RECENT] is
                InsightsSummary.WindowMetadata
        )
    }

    @Test
    fun insightsDay_keepsStructuredActivityTimelineForDayText() = runTest {
        val fakeInsightsGateway = FakeStructuredInsightsGateway().apply {
            dayResult = InsightsCallResult(
                initialized = true,
                operationOk = true,
                outputText = "# Day Insights",
                rawResponse = """{"ok":true}"""
            )
            dayStructuredResult = StructuredInsightsCallResult(
                initialized = true,
                operationOk = true,
                insights = StructuredDailyInsights(
                    date = "2026-02-14",
                    totalDurationSeconds = 3600,
                    activities = listOf(
                        ActivityTimelineItem(
                            startTime = "09:00",
                            endTime = "10:00",
                            activityName = "study_math",
                            durationSeconds = 3600,
                            remark = "整理错题"
                        )
                    )
                ),
                rawResponse = """{"ok":true}"""
            )
        }
        val viewModel = QueryInsightsViewModel(
            insightsGateway = fakeInsightsGateway,
            queryGateway = FakeInsightsSyncQueryGateway()
        )

        viewModel.onInsightsDateChange("20260214")
        viewModel.insightsDay()
        advanceUntilIdle()

        assertEquals(
            "study_math",
            viewModel.uiState.dayTimeline?.activities?.single()?.activityName
        )
        assertEquals(
            "整理错题",
            viewModel.uiState.dayTimeline?.activities?.single()?.remark
        )
    }

    @Test
    fun selectingMarkdownOrTimeline_onDayAutomaticallyGeneratesTheInsights() = runTest {
        val fakeInsightsGateway = FakeStructuredInsightsGateway().apply {
            dayResult = InsightsCallResult(
                initialized = true,
                operationOk = true,
                outputText = "# Day Insights",
                rawResponse = """{"ok":true}"""
            )
        }
        val viewModel = QueryInsightsViewModel(
            insightsGateway = fakeInsightsGateway,
            queryGateway = FakeInsightsSyncQueryGateway()
        )

        viewModel.onParameterSectionChange(InsightsParameterSection.TIMELINE)
        advanceUntilIdle()
        assertEquals("# Day Insights", (viewModel.uiState.activeResult as QueryResult.Insights).text)

        viewModel.onParameterSectionChange(InsightsParameterSection.DAY)
        advanceUntilIdle()
        assertEquals("# Day Insights", (viewModel.uiState.activeResult as QueryResult.Insights).text)
    }

    @Test
    fun insightsMonth_usesTemporalQueryRequestWithDateRangeSelection() = runTest {
        val fakeInsightsGateway = FakeStructuredInsightsGateway()
        val viewModel = QueryInsightsViewModel(
            insightsGateway = fakeInsightsGateway,
            queryGateway = FakeInsightsSyncQueryGateway()
        )

        viewModel.onInsightsMonthChange("202602")
        viewModel.insightsMonth()
        advanceUntilIdle()

        val request = fakeInsightsGateway.lastTemporalRequest
        assertNotNull(request)
        assertEquals(InsightsDisplayMode.MONTH, request?.displayMode)
        assertEquals(TemporalSelectionKind.DATE_RANGE, request?.selection?.kind)
        assertEquals("2026-02-01", request?.selection?.startDate)
        assertEquals("2026-02-28", request?.selection?.endDate)
    }

    @Test
    fun insightsWeek_usesStructuredPeriodStatusValues() = runTest {
        val fakeInsightsGateway = FakeStructuredInsightsGateway().apply {
            periodStructuredResult = StructuredInsightsCallResult(
                initialized = true,
                operationOk = true,
                insights = null,
                rawResponse = """{"ok":true}""",
                statuses = listOf(
                    InsightsStatusValue(id = "study", label = "Study", occurrenceCount = 3, totalDurationSeconds = 7200)
                )
            )
        }
        val viewModel = QueryInsightsViewModel(
            insightsGateway = fakeInsightsGateway,
            queryGateway = FakeInsightsSyncQueryGateway()
        )

        viewModel.onInsightsWeekChange("202607")
        viewModel.insightsWeek()
        advanceUntilIdle()

        assertEquals(
            listOf(InsightsStatusValue(id = "study", label = "Study", occurrenceCount = 3, totalDurationSeconds = 7200)),
            viewModel.uiState.statusValues
        )
    }

    @Test
    fun changingInsightsLocale_requeriesCurrentMarkdownWithNewLocale() = runTest {
        val fakeInsightsGateway = FakeStructuredInsightsGateway()
        val viewModel = QueryInsightsViewModel(
            insightsGateway = fakeInsightsGateway,
            queryGateway = FakeInsightsSyncQueryGateway(),
            clock = fixedClock("2026-02-14T12:00:00Z", "Asia/Shanghai")
        )
        viewModel.applyPersistedInsightsPresentation(
            insightsMode = InsightsMode.DAY,
            chartSemanticMode = InsightsChartSemanticMode.COMPOSITION,
            resultDisplayMode = InsightsResultDisplayMode.TEXT,
            parameterSection = InsightsParameterSection.DAY
        )
        advanceUntilIdle()

        // Exercise an actual transition regardless of the host JVM's default locale.
        viewModel.onInsightsLocaleChange("en")
        advanceUntilIdle()
        viewModel.onInsightsLocaleChange("zh")
        advanceUntilIdle()

        assertEquals("zh", fakeInsightsGateway.lastTemporalRequest?.locale)
    }

    @Test
    fun refreshInsightsDayDefault_beforeCutoff_uses_previous_logical_day() = runTest {
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeStructuredInsightsGateway(),
            queryGateway = FakeInsightsSyncQueryGateway(),
            clock = fixedClock("2026-03-29T21:30:00Z", "Asia/Shanghai")
        )

        assertEquals("20260329", viewModel.uiState.insightsDate)
    }

    @Test
    fun refreshInsightsDayDefault_discards_previous_day_selection_when_reentering_insights() = runTest {
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeStructuredInsightsGateway(),
            queryGateway = FakeInsightsSyncQueryGateway(),
            clock = fixedClock("2026-03-30T02:30:00Z", "Asia/Shanghai")
        )

        viewModel.onInsightsDateChange("20260320")
        viewModel.refreshInsightsDayDefault()

        assertEquals("20260330", viewModel.uiState.insightsDate)
    }

    @Test
    fun refreshInsightsDayDefault_requeries_even_when_logical_day_did_not_change() = runTest {
        val fakeInsightsGateway = FakeStructuredInsightsGateway().apply {
            dayResult = InsightsCallResult(
                initialized = true,
                operationOk = true,
                outputText = "# Refreshed Day Insights",
                rawResponse = """{"ok":true}"""
            )
            dayStructuredResult = StructuredInsightsCallResult(
                initialized = true,
                operationOk = true,
                insights = StructuredDailyInsights(
                    date = "2026-03-30",
                    totalDurationSeconds = 60,
                    activities = listOf(
                        ActivityTimelineItem(
                            startTime = "09:00",
                            endTime = "09:01",
                            activityName = "new_activity",
                            durationSeconds = 60
                        )
                    )
                ),
                rawResponse = """{"ok":true}"""
            )
        }
        val viewModel = QueryInsightsViewModel(
            insightsGateway = fakeInsightsGateway,
            queryGateway = FakeInsightsSyncQueryGateway(),
            clock = fixedClock("2026-03-30T12:30:00Z", "Asia/Shanghai")
        )

        viewModel.refreshInsightsDayDefault()
        advanceUntilIdle()

        assertEquals("# Refreshed Day Insights", (viewModel.uiState.activeResult as QueryResult.Insights).text)
        assertEquals("new_activity", viewModel.uiState.dayTimeline?.activities?.single()?.activityName)
    }

    @Test
    fun timelineSection_isAvailableForDay_andResetsForOtherPeriods() {
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeStructuredInsightsGateway(),
            queryGateway = FakeInsightsSyncQueryGateway()
        )

        viewModel.onParameterSectionChange(InsightsParameterSection.TIMELINE)
        assertEquals(InsightsParameterSection.TIMELINE, viewModel.uiState.parameterSection)

        viewModel.onInsightsModeChange(InsightsMode.WEEK)

        assertEquals(InsightsParameterSection.DAY, viewModel.uiState.parameterSection)
    }
}

private fun fixedClock(instantIso: String, zoneId: String): Clock =
    Clock.fixed(Instant.parse(instantIso), ZoneId.of(zoneId))

private class FakeStructuredInsightsGateway : InsightsGateway {
    var dayResult: InsightsCallResult = successResult()
    var monthResult: InsightsCallResult = successResult()
    var yearResult: InsightsCallResult = successResult()
    var weekResult: InsightsCallResult = successResult()
    var recentResult: InsightsCallResult = successResult()
    var rangeResult: InsightsCallResult = successResult()
    var dayStructuredResult: StructuredInsightsCallResult = failedStructuredResult()
    var periodStructuredResult: StructuredInsightsCallResult = failedStructuredResult()
    var lastTemporalRequest: TemporalInsightsQueryRequest? = null

    override suspend fun insightsMarkdown(request: TemporalInsightsQueryRequest): InsightsCallResult {
        lastTemporalRequest = request
        return when (request.displayMode) {
            InsightsDisplayMode.DAY -> dayResult
            InsightsDisplayMode.MONTH -> monthResult
            InsightsDisplayMode.YEAR -> yearResult
            InsightsDisplayMode.WEEK -> weekResult
            InsightsDisplayMode.RECENT -> recentResult
            InsightsDisplayMode.RANGE -> rangeResult
        }
    }

    override suspend fun insightsStructured(
        request: TemporalInsightsQueryRequest
    ): StructuredInsightsCallResult = if (request.displayMode == InsightsDisplayMode.DAY) {
        dayStructuredResult
    } else {
        periodStructuredResult
    }

    private fun successResult(): InsightsCallResult = InsightsCallResult(
        initialized = true,
        operationOk = true,
        outputText = "",
        rawResponse = ""
    )

    private fun failedStructuredResult(): StructuredInsightsCallResult =
        StructuredInsightsCallResult(
            initialized = true,
            operationOk = false,
            insights = null,
            rawResponse = ""
        )
}

private class FakeInsightsSyncQueryGateway : QueryGateway {
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

    override suspend fun queryProjectTree(params: DataTreeQueryParams): TreeQueryResult =
        TreeQueryResult(ok = true, found = false, message = "ok")

    override suspend fun queryInsightsChart(params: InsightsChartQueryParams): InsightsChartQueryResult =
        InsightsChartQueryResult(
            ok = true,
            data = InsightsChartData(
                roots = emptyList(),
                selectedRoot = "",
                lookbackDays = params.lookbackDays,
                points = emptyList()
            ),
            message = "ok"
        )

    override suspend fun listActivityMappingNames(): ActivityMappingNamesResult =
        ActivityMappingNamesResult(ok = true, names = emptyList(), message = "ok")
}
