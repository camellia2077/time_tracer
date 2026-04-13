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

@OptIn(ExperimentalCoroutinesApi::class)
class QueryReportViewModelReportSyncTest {
    @get:Rule
    val mainDispatcherRule = MainDispatcherRule()

    @Test
    fun reportDay_missingTarget_setsSummaryAndClearsCachedMarkdown() = runTest {
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
        assertTrue(summary is ReportSummary.MissingTarget)
        summary as ReportSummary.MissingTarget
        assertEquals("reporting.target.not_found", summary.errorCode)
        assertEquals(listOf("Try another date."), summary.hints)
        assertTrue(state.statusText.contains("target not found"))
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
}

private class FakeStructuredReportGateway : ReportGateway {
    var dayResult: ReportCallResult = successResult()
    var monthResult: ReportCallResult = successResult()
    var yearResult: ReportCallResult = successResult()
    var weekResult: ReportCallResult = successResult()
    var recentResult: ReportCallResult = successResult()
    var rangeResult: ReportCallResult = successResult()
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

    private fun successResult(): ReportCallResult = ReportCallResult(
        initialized = true,
        operationOk = true,
        outputText = "",
        rawResponse = ""
    )
}

private class FakeReportSyncQueryGateway : QueryGateway {
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
