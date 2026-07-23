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
class QueryReportViewModelReportSyncTest {
    @get:Rule
    val mainDispatcherRule = MainDispatcherRule()

    @Test
    fun validDateChange_automaticallyGeneratesMarkdownReport() = runTest {
        val fakeReportGateway = FakeStructuredReportGateway().apply {
            dayResult = ReportCallResult(
                initialized = true,
                operationOk = true,
                outputText = "# Automatically generated",
                rawResponse = """{"ok":true}"""
            )
        }
        val viewModel = QueryReportViewModel(
            reportGateway = fakeReportGateway,
            queryGateway = FakeReportSyncQueryGateway()
        )

        viewModel.onReportDateChange("20260214")
        advanceUntilIdle()

        assertEquals(
            "# Automatically generated",
            (viewModel.uiState.activeResult as QueryResult.Report).text
        )
    }

    @Test
    fun reportDay_missingTarget_isPresentedAsNormalNoData() = runTest {
        val fakeReportGateway = FakeStructuredReportGateway().apply {
            dayResult = ReportCallResult(
                initialized = true,
                operationOk = false,
                outputText = "runtime report failed. [op=day-missing]",
                rawResponse = """{"ok":false}""",
                errorContract = ReportErrorContract(
                    errorCode = "reporting.target.not_found",
                    errorCategory = "reporting",
                    hints = listOf("Try another date.")
                )
            )
        }
        val viewModel = QueryReportViewModel(
            reportGateway = fakeReportGateway,
            queryGateway = FakeReportSyncQueryGateway()
        )

        viewModel.onReportDateChange("20260214")
        viewModel.reportDay()
        advanceUntilIdle()

        val state = viewModel.uiState
        assertNull(state.activeResult)
        assertTrue(DataTreePeriod.DAY !in state.reportResultsByPeriod)
        assertTrue(DataTreePeriod.DAY !in state.reportErrorsByPeriod)
        val summary = state.reportSummariesByPeriod[DataTreePeriod.DAY]
        assertTrue(summary is ReportSummary.NoData)
        assertEquals(DataTreePeriod.DAY, (summary as ReportSummary.NoData).period)
        assertTrue(state.statusText.contains("No records for this day"))
    }

    @Test
    fun reportRecent_emptyWindow_keepsMarkdownAndExposesWindowSummary() = runTest {
        val metadata = ReportWindowMetadata(
            hasRecords = false,
            matchedDayCount = 0,
            matchedRecordCount = 0,
            startDate = "2026-02-01",
            endDate = "2026-02-07",
            requestedDays = 7
        )
        val fakeReportGateway = FakeStructuredReportGateway().apply {
            recentResult = ReportCallResult(
                initialized = true,
                operationOk = true,
                outputText = "## Recent Report\n\nNo rows.",
                rawResponse = """{"ok":true}""",
                reportWindowMetadata = metadata
            )
        }
        val viewModel = QueryReportViewModel(
            reportGateway = fakeReportGateway,
            queryGateway = FakeReportSyncQueryGateway()
        )

        viewModel.onReportRecentDaysChange("7")
        viewModel.reportRecent()
        advanceUntilIdle()

        val state = viewModel.uiState
        assertTrue(state.activeResult is QueryResult.Report)
        val activeResult = state.activeResult as QueryResult.Report
        assertEquals("## Recent Report\n\nNo rows.", activeResult.text)
        assertTrue(activeResult.summary is ReportSummary.WindowMetadata)
        val summary = activeResult.summary as ReportSummary.WindowMetadata
        assertEquals(false, summary.metadata.hasRecords)
        assertEquals(7, summary.metadata.requestedDays)
        assertTrue(DataTreePeriod.RECENT !in state.reportErrorsByPeriod)
        assertEquals(activeResult, state.reportResultsByPeriod[DataTreePeriod.RECENT])
        assertTrue(state.statusText.contains("empty window"))
    }

    @Test
    fun reportResults_cacheSummaryPerPeriod() = runTest {
        val fakeReportGateway = FakeStructuredReportGateway().apply {
            dayResult = ReportCallResult(
                initialized = true,
                operationOk = true,
                outputText = "# Day Report",
                rawResponse = """{"ok":true}"""
            )
            recentResult = ReportCallResult(
                initialized = true,
                operationOk = true,
                outputText = "# Recent Report",
                rawResponse = """{"ok":true}""",
                reportWindowMetadata = ReportWindowMetadata(
                    hasRecords = true,
                    matchedDayCount = 3,
                    matchedRecordCount = 9,
                    startDate = "2026-02-01",
                    endDate = "2026-02-07",
                    requestedDays = 7
                )
            )
        }
        val viewModel = QueryReportViewModel(
            reportGateway = fakeReportGateway,
            queryGateway = FakeReportSyncQueryGateway()
        )

        viewModel.onReportDateChange("20260214")
        viewModel.reportDay()
        advanceUntilIdle()

        viewModel.onReportRecentDaysChange("7")
        viewModel.reportRecent()
        advanceUntilIdle()

        assertEquals("# Day Report", viewModel.uiState.reportResultsByPeriod[DataTreePeriod.DAY]?.text)
        assertEquals(
            null,
            viewModel.uiState.reportResultsByPeriod[DataTreePeriod.DAY]?.summary
        )
        assertTrue(
            viewModel.uiState.reportResultsByPeriod[DataTreePeriod.RECENT]?.summary is
                ReportSummary.WindowMetadata
        )
        assertTrue(
            viewModel.uiState.reportSummariesByPeriod[DataTreePeriod.RECENT] is
                ReportSummary.WindowMetadata
        )
    }

    @Test
    fun reportDay_keepsStructuredActivityTimelineForDayText() = runTest {
        val fakeReportGateway = FakeStructuredReportGateway().apply {
            dayResult = ReportCallResult(
                initialized = true,
                operationOk = true,
                outputText = "# Day Report",
                rawResponse = """{"ok":true}"""
            )
            dayStructuredResult = StructuredReportCallResult(
                initialized = true,
                operationOk = true,
                report = StructuredDailyReport(
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
        val viewModel = QueryReportViewModel(
            reportGateway = fakeReportGateway,
            queryGateway = FakeReportSyncQueryGateway()
        )

        viewModel.onReportDateChange("20260214")
        viewModel.reportDay()
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
    fun selectingMarkdownOrTimeline_onDayAutomaticallyGeneratesTheReport() = runTest {
        val fakeReportGateway = FakeStructuredReportGateway().apply {
            dayResult = ReportCallResult(
                initialized = true,
                operationOk = true,
                outputText = "# Day Report",
                rawResponse = """{"ok":true}"""
            )
        }
        val viewModel = QueryReportViewModel(
            reportGateway = fakeReportGateway,
            queryGateway = FakeReportSyncQueryGateway()
        )

        viewModel.onParameterSectionChange(ReportParameterSection.TIMELINE)
        advanceUntilIdle()
        assertEquals("# Day Report", (viewModel.uiState.activeResult as QueryResult.Report).text)

        viewModel.onParameterSectionChange(ReportParameterSection.DAY)
        advanceUntilIdle()
        assertEquals("# Day Report", (viewModel.uiState.activeResult as QueryResult.Report).text)
    }

    @Test
    fun reportMonth_usesTemporalQueryRequestWithDateRangeSelection() = runTest {
        val fakeReportGateway = FakeStructuredReportGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = fakeReportGateway,
            queryGateway = FakeReportSyncQueryGateway()
        )

        viewModel.onReportMonthChange("202602")
        viewModel.reportMonth()
        advanceUntilIdle()

        val request = fakeReportGateway.lastTemporalRequest
        assertNotNull(request)
        assertEquals(ReportDisplayMode.MONTH, request?.displayMode)
        assertEquals(TemporalSelectionKind.DATE_RANGE, request?.selection?.kind)
        assertEquals("2026-02-01", request?.selection?.startDate)
        assertEquals("2026-02-28", request?.selection?.endDate)
    }

    @Test
    fun refreshReportDayDefault_beforeCutoff_uses_previous_logical_day() = runTest {
        val viewModel = QueryReportViewModel(
            reportGateway = FakeStructuredReportGateway(),
            queryGateway = FakeReportSyncQueryGateway(),
            clock = fixedClock("2026-03-29T21:30:00Z", "Asia/Shanghai")
        )

        assertEquals("20260329", viewModel.uiState.reportDate)
    }

    @Test
    fun refreshReportDayDefault_discards_previous_day_selection_when_reentering_report() = runTest {
        val viewModel = QueryReportViewModel(
            reportGateway = FakeStructuredReportGateway(),
            queryGateway = FakeReportSyncQueryGateway(),
            clock = fixedClock("2026-03-30T02:30:00Z", "Asia/Shanghai")
        )

        viewModel.onReportDateChange("20260320")
        viewModel.refreshReportDayDefault()

        assertEquals("20260330", viewModel.uiState.reportDate)
    }

    @Test
    fun refreshReportDayDefault_requeries_even_when_logical_day_did_not_change() = runTest {
        val fakeReportGateway = FakeStructuredReportGateway().apply {
            dayResult = ReportCallResult(
                initialized = true,
                operationOk = true,
                outputText = "# Refreshed Day Report",
                rawResponse = """{"ok":true}"""
            )
            dayStructuredResult = StructuredReportCallResult(
                initialized = true,
                operationOk = true,
                report = StructuredDailyReport(
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
        val viewModel = QueryReportViewModel(
            reportGateway = fakeReportGateway,
            queryGateway = FakeReportSyncQueryGateway(),
            clock = fixedClock("2026-03-30T12:30:00Z", "Asia/Shanghai")
        )

        viewModel.refreshReportDayDefault()
        advanceUntilIdle()

        assertEquals("# Refreshed Day Report", (viewModel.uiState.activeResult as QueryResult.Report).text)
        assertEquals("new_activity", viewModel.uiState.dayTimeline?.activities?.single()?.activityName)
    }

    @Test
    fun timelineSection_isAvailableForDay_andResetsForOtherPeriods() {
        val viewModel = QueryReportViewModel(
            reportGateway = FakeStructuredReportGateway(),
            queryGateway = FakeReportSyncQueryGateway()
        )

        viewModel.onParameterSectionChange(ReportParameterSection.TIMELINE)
        assertEquals(ReportParameterSection.TIMELINE, viewModel.uiState.parameterSection)

        viewModel.onReportModeChange(ReportMode.WEEK)

        assertEquals(ReportParameterSection.DAY, viewModel.uiState.parameterSection)
    }
}

private fun fixedClock(instantIso: String, zoneId: String): Clock =
    Clock.fixed(Instant.parse(instantIso), ZoneId.of(zoneId))

private class FakeStructuredReportGateway : ReportGateway {
    var dayResult: ReportCallResult = successResult()
    var monthResult: ReportCallResult = successResult()
    var yearResult: ReportCallResult = successResult()
    var weekResult: ReportCallResult = successResult()
    var recentResult: ReportCallResult = successResult()
    var rangeResult: ReportCallResult = successResult()
    var dayStructuredResult: StructuredReportCallResult = failedStructuredResult()
    var lastTemporalRequest: TemporalReportQueryRequest? = null

    override suspend fun reportMarkdown(request: TemporalReportQueryRequest): ReportCallResult {
        lastTemporalRequest = request
        return when (request.displayMode) {
            ReportDisplayMode.DAY -> dayResult
            ReportDisplayMode.MONTH -> monthResult
            ReportDisplayMode.YEAR -> yearResult
            ReportDisplayMode.WEEK -> weekResult
            ReportDisplayMode.RECENT -> recentResult
            ReportDisplayMode.RANGE -> rangeResult
        }
    }

    override suspend fun reportStructured(
        request: TemporalReportQueryRequest
    ): StructuredReportCallResult = if (request.displayMode == ReportDisplayMode.DAY) {
        dayStructuredResult
    } else {
        failedStructuredResult()
    }

    private fun successResult(): ReportCallResult = ReportCallResult(
        initialized = true,
        operationOk = true,
        outputText = "",
        rawResponse = ""
    )

    private fun failedStructuredResult(): StructuredReportCallResult =
        StructuredReportCallResult(
            initialized = true,
            operationOk = false,
            report = null,
            rawResponse = ""
        )
}

private class FakeReportSyncQueryGateway : QueryGateway {
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

    override suspend fun queryProjectTree(params: DataTreeQueryParams): TreeQueryResult =
        TreeQueryResult(ok = true, found = false, message = "ok")

    override suspend fun queryReportChart(params: ReportChartQueryParams): ReportChartQueryResult =
        ReportChartQueryResult(
            ok = true,
            data = ReportChartData(
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
