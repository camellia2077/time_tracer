package com.example.tracer

import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.test.advanceUntilIdle
import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Rule
import org.junit.Test
import java.time.Clock
import java.time.Instant
import java.time.ZoneId

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
        assertEquals(true, treeResult.found)
        assertEquals(1, treeResult.nodes.size)
        assertEquals("study", treeResult.nodes[0].name)
    }

    @Test
    fun selectingTree_automaticallyLoadsTheCurrentReportDate() = runTest {
        val fakeQueryGateway = FakeTreeQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = FakeTreeReportGateway(),
            queryGateway = fakeQueryGateway,
            clock = Clock.fixed(
                Instant.parse("2026-04-13T12:00:00Z"),
                ZoneId.of("UTC")
            )
        )

        viewModel.onParameterSectionChange(ReportParameterSection.ACTIVITY_HIERARCHY)
        advanceUntilIdle()

        val request = fakeQueryGateway.lastTreeRequest
        assertTrue(request != null)
        assertEquals(DataTreePeriod.DAY, request?.period)
        assertEquals("20260413", request?.periodArgument)
        assertEquals(-1, request?.level)
    }

    @Test
    fun selectingTreeAgain_rehydratesMissingResult() = runTest {
        val fakeQueryGateway = FakeTreeQueryGateway().apply {
            treeResult = TreeQueryResult(
                ok = false,
                found = false,
                message = "temporary failure"
            )
        }
        val viewModel = QueryReportViewModel(
            reportGateway = FakeTreeReportGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onParameterSectionChange(ReportParameterSection.ACTIVITY_HIERARCHY)
        advanceUntilIdle()
        assertTrue(viewModel.uiState.activeResult == null)

        fakeQueryGateway.treeResult = TreeQueryResult(
            ok = true,
            found = true,
            nodes = listOf(TreeNode(name = "study", path = "study")),
            message = "ok"
        )
        // The selected section has not changed, but its result is still missing.
        // This is the state that can occur after a theme-driven recomposition.
        viewModel.onParameterSectionChange(ReportParameterSection.ACTIVITY_HIERARCHY)
        advanceUntilIdle()

        assertTrue(viewModel.uiState.activeResult is QueryResult.Tree)
        assertEquals(2, fakeQueryGateway.treeRequestCount)
    }

    @Test
    fun changingTreeDate_reloadsTreeUsingSelectedLevel() = runTest {
        val fakeQueryGateway = FakeTreeQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = FakeTreeReportGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onParameterSectionChange(ReportParameterSection.ACTIVITY_HIERARCHY)
        advanceUntilIdle()
        viewModel.onTreeLevelChange(1)
        advanceUntilIdle()

        viewModel.onReportDateChange("20260412")
        advanceUntilIdle()

        val request = fakeQueryGateway.lastTreeRequest
        assertTrue(request != null)
        assertEquals(DataTreePeriod.DAY, request?.period)
        assertEquals("20260412", request?.periodArgument)
        assertEquals(1, request?.level)
    }

    @Test
    fun changingReportMode_whileTreeSelected_reloadsTreeForNewPeriod() = runTest {
        val fakeQueryGateway = FakeTreeQueryGateway()
        val viewModel = QueryReportViewModel(
            reportGateway = FakeTreeReportGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onParameterSectionChange(ReportParameterSection.ACTIVITY_HIERARCHY)
        advanceUntilIdle()
        viewModel.onTreeLevelChange(1)
        advanceUntilIdle()

        viewModel.onReportModeChange(ReportMode.WEEK)
        advanceUntilIdle()

        val request = fakeQueryGateway.lastTreeRequest
        assertTrue(request != null)
        assertEquals(DataTreePeriod.WEEK, request?.period)
        assertEquals(1, request?.level)
    }

    @Test
    fun switchingAwayFromTree_resolvesMarkdownOrTimelineInsteadOfTree() {
        val markdown = QueryResult.Report(text = "markdown")
        val tree = QueryResult.Tree(
            period = DataTreePeriod.DAY,
            nodes = emptyList(),
            found = true
        )
        val state = initialQueryReportUiState().copy(
            activeResult = tree,
            reportResultsByPeriod = mapOf(DataTreePeriod.DAY to markdown)
        )

        assertEquals(
            markdown,
            resolveDisplayResult(state, DataTreePeriod.DAY, ReportParameterSection.DAY)
        )
        assertEquals(
            markdown,
            resolveDisplayResult(state, DataTreePeriod.DAY, ReportParameterSection.TIMELINE)
        )
        assertEquals(
            tree,
            resolveDisplayResult(state, DataTreePeriod.DAY, ReportParameterSection.ACTIVITY_HIERARCHY)
        )
    }
}

private class FakeTreeReportGateway : ReportGateway {
    override suspend fun reportMarkdown(request: TemporalReportQueryRequest): ReportCallResult =
        ReportCallResult(
            initialized = true,
            operationOk = true,
            outputText = "",
            rawResponse = ""
        )
}

private class FakeTreeQueryGateway : QueryGateway {
    var lastTreeRequest: DataTreeQueryParams? = null
    var treeRequestCount: Int = 0
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
                        durationSeconds = 3600L,
                        parentDurationPercent = 50f
                    )
                )
            )
        ),
        message = "Loaded 2 tree node(s)."
    )

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
        lastTreeRequest = params
        treeRequestCount += 1
        return treeResult
    }

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

