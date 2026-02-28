package com.example.tracer

import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.test.advanceUntilIdle
import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Rule
import org.junit.Test

@OptIn(ExperimentalCoroutinesApi::class)
class QueryReportTreePipelineTest {
    @get:Rule
    val mainDispatcherRule = MainDispatcherRule()

    @Test
    fun loadTree_usesStructuredNodes_andPassesNormalizedArgument() = runTest {
        val fakeQueryGateway = FakeTreeQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = FakeTreeReportGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onReportWeekChange("202607")
        viewModel.loadTree(period = DataTreePeriod.WEEK, level = 1)
        advanceUntilIdle()

        val request = fakeQueryGateway.lastTreeRequest
        assertTrue(request != null)
        assertEquals(DataTreePeriod.WEEK, request?.period)
        assertEquals("2026-W07", request?.periodArgument)
        assertEquals(1, request?.level)

        val activeResult = viewModel.uiState.activeResult
        assertTrue(activeResult is QueryResult.Tree)
        val treeResult = activeResult as QueryResult.Tree
        assertEquals(false, treeResult.usesTextFallback)
        assertEquals(true, treeResult.found)
        assertEquals(1, treeResult.nodes.size)
        assertEquals("study", treeResult.nodes[0].name)
    }

    @Test
    fun loadTree_whenStructuredResultFallsBackToText_keepsFallbackText() = runTest {
        val fakeQueryGateway = FakeTreeQueryGateway().apply {
            treeResult = TreeQueryResult(
                ok = true,
                found = true,
                roots = emptyList(),
                nodes = emptyList(),
                message = "Loaded tree via legacy text fallback.",
                legacyText = "study\n└── math",
                usesTextFallback = true
            )
        }
        val viewModel = QueryReportViewModel(
            reportGateway = FakeTreeReportGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onReportRecentDaysChange("7")
        viewModel.loadTree(period = DataTreePeriod.RECENT, level = -1)
        advanceUntilIdle()

        val activeResult = viewModel.uiState.activeResult
        assertTrue(activeResult is QueryResult.Tree)
        val treeResult = activeResult as QueryResult.Tree
        assertEquals(true, treeResult.usesTextFallback)
        assertTrue(treeResult.fallbackText.contains("study"))
    }
}

private class FakeTreeReportGateway : ReportGateway {
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

private class FakeTreeQueryGateway : QueryGateway {
    var lastTreeRequest: DataTreeQueryParams? = null
    var treeResult: TreeQueryResult = TreeQueryResult(
        ok = true,
        found = true,
        roots = listOf("study"),
        nodes = listOf(
            TreeNode(
                name = "study",
                path = "study",
                durationSeconds = 7200L,
                children = listOf(
                    TreeNode(
                        name = "math",
                        path = "study_math",
                        durationSeconds = 3600L
                    )
                )
            )
        ),
        message = "Loaded 2 tree node(s)."
    )

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

    override suspend fun queryProjectTree(params: DataTreeQueryParams): TreeQueryResult {
        lastTreeRequest = params
        return treeResult
    }

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

