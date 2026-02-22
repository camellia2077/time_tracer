package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.test.StandardTestDispatcher
import kotlinx.coroutines.test.TestDispatcher
import kotlinx.coroutines.test.advanceUntilIdle
import kotlinx.coroutines.test.resetMain
import kotlinx.coroutines.test.runTest
import kotlinx.coroutines.test.setMain
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Rule
import org.junit.Test
import org.junit.rules.TestWatcher
import org.junit.runner.Description

@OptIn(ExperimentalCoroutinesApi::class)
class QueryReportViewModelStatsTest {
    @get:Rule
    val mainDispatcherRule = MainDispatcherRule()

    @Test
    fun loadDayStats_keepsMarkdownOutput_andCoversAllPeriods() = runTest {
        val fakeQueryGateway = FakeQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = FakeReportGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onReportDateChange("20260214")
        viewModel.onReportMonthChange("202602")
        viewModel.onReportYearChange("2026")
        viewModel.onReportWeekChange("202607")
        viewModel.onReportRangeStartDateChange("20260201")
        viewModel.onReportRangeEndDateChange("20260214")
        viewModel.onReportRecentDaysChange("7")

        val expectedArguments = mapOf(
            DataTreePeriod.DAY to "20260214",
            DataTreePeriod.WEEK to "2026-W07",
            DataTreePeriod.MONTH to "202602",
            DataTreePeriod.YEAR to "2026",
            DataTreePeriod.RECENT to "7",
            DataTreePeriod.RANGE to "2026-02-01|2026-02-14"
        )

        for ((period, expectedArgument) in expectedArguments) {
            fakeQueryGateway.nextOutput = "## Day Duration Stats\n\nstats-md-marker-${period.wireValue}"
            viewModel.loadDayStats(period)
            advanceUntilIdle()

            val lastRequest = fakeQueryGateway.lastStatsRequest
            assertTrue("Expected stats request to be captured", lastRequest != null)
            assertEquals(period, lastRequest?.period)
            assertEquals(expectedArgument, lastRequest?.periodArgument)

            assertEquals(period, viewModel.uiState.statsPeriod)
            assertEquals("", viewModel.uiState.analysisError)
            val activeResult = viewModel.uiState.activeResult
            assertTrue("Expected activeResult to be Stats", activeResult is QueryResult.Stats)
            val statsText = (activeResult as QueryResult.Stats).text
            assertTrue(
                "Expected markdown marker in stats output for period=${period.wireValue}",
                statsText.contains("stats-md-marker-${period.wireValue}")
            )
        }
    }
}

@OptIn(ExperimentalCoroutinesApi::class)
class MainDispatcherRule(
    private val dispatcher: TestDispatcher = StandardTestDispatcher()
) : TestWatcher() {
    override fun starting(description: Description) {
        Dispatchers.setMain(dispatcher)
    }

    override fun finished(description: Description) {
        Dispatchers.resetMain()
    }
}

private class FakeReportGateway : ReportGateway {
    override suspend fun reportDayMarkdown(date: String): ReportCallResult =
        ReportCallResult(
            initialized = true,
            operationOk = true,
            outputText = "",
            rawResponse = ""
        )

    override suspend fun reportMonthMarkdown(month: String): ReportCallResult =
        reportDayMarkdown(month)

    override suspend fun reportYearMarkdown(year: String): ReportCallResult =
        reportDayMarkdown(year)

    override suspend fun reportWeekMarkdown(week: String): ReportCallResult =
        reportDayMarkdown(week)

    override suspend fun reportRecentMarkdown(days: String): ReportCallResult =
        reportDayMarkdown(days)

    override suspend fun reportRange(startDate: String, endDate: String): ReportCallResult =
        reportDayMarkdown("$startDate|$endDate")
}

private class FakeQueryGateway : QueryGateway {
    var nextOutput: String = "## Day Duration Stats"
    var lastStatsRequest: DataDurationQueryParams? = null

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

    override suspend fun queryDayDurationStats(params: DataDurationQueryParams): DataQueryTextResult {
        lastStatsRequest = params
        return DataQueryTextResult(
            ok = true,
            outputText = nextOutput,
            message = "ok"
        )
    }

    override suspend fun queryProjectTree(params: DataTreeQueryParams): DataQueryTextResult =
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
