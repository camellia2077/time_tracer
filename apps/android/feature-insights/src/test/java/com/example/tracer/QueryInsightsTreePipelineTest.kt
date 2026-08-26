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
class QueryInsightsTreePipelineTest {
    @get:Rule
    val mainDispatcherRule = MainDispatcherRule()

    @Test
    fun loadTree_usesStructuredNodes_andPassesNormalizedArgument() = runTest {
        val fakeQueryGateway = FakeTreeQueryGateway()
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeTreeInsightsGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onInsightsWeekChange("202607")
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
    fun selectingHierarchy_automaticallyLoadsTheCurrentInsightsDate() = runTest {
        val fakeQueryGateway = FakeTreeQueryGateway()
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeTreeInsightsGateway(),
            queryGateway = fakeQueryGateway,
            clock = Clock.fixed(
                Instant.parse("2026-04-13T12:00:00Z"),
                ZoneId.of("UTC")
            )
        )

        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
        viewModel.onChartSemanticModeChange(InsightsChartSemanticMode.HIERARCHY)
        advanceUntilIdle()

        val request = fakeQueryGateway.lastTreeRequest
        assertTrue(request != null)
        assertEquals(DataTreePeriod.DAY, request?.period)
        assertEquals("20260413", request?.periodArgument)
        assertEquals(-1, request?.level)
    }

    @Test
    fun selectingHierarchy_loadsTheCurrentActivityTree() = runTest {
        val fakeQueryGateway = FakeTreeQueryGateway()
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeTreeInsightsGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
        viewModel.onChartSemanticModeChange(InsightsChartSemanticMode.HIERARCHY)
        advanceUntilIdle()

        assertEquals(InsightsResultDisplayMode.CHART, viewModel.uiState.resultDisplayMode)
        assertEquals(InsightsChartSemanticMode.HIERARCHY, viewModel.uiState.chartSemanticMode)
        assertEquals(DataTreePeriod.DAY, fakeQueryGateway.lastTreeRequest?.period)
        assertTrue(viewModel.uiState.activeResult is QueryResult.Tree)
    }

    @Test
    fun selectingHierarchyAgain_rehydratesMissingResult() = runTest {
        val fakeQueryGateway = FakeTreeQueryGateway().apply {
            treeResult = TreeQueryResult(
                ok = false,
                found = false,
                message = "temporary failure"
            )
        }
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeTreeInsightsGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
        viewModel.onChartSemanticModeChange(InsightsChartSemanticMode.HIERARCHY)
        advanceUntilIdle()
        assertTrue(viewModel.uiState.activeResult == null)

        fakeQueryGateway.treeResult = TreeQueryResult(
            ok = true,
            found = true,
            nodes = listOf(TreeNode(name = "study", path = "study")),
            message = "ok"
        )
        // The selected display mode has not changed, but its result is still missing.
        // This is the state that can occur after a theme-driven recomposition.
        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
        viewModel.onChartSemanticModeChange(InsightsChartSemanticMode.HIERARCHY)
        advanceUntilIdle()

        assertTrue(viewModel.uiState.activeResult is QueryResult.Tree)
        assertEquals(2, fakeQueryGateway.treeRequestCount)
    }

    @Test
    fun selectingHierarchyAsChartSubview_loadsTheCurrentActivityTree() = runTest {
        val fakeQueryGateway = FakeTreeQueryGateway()
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeTreeInsightsGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
        viewModel.onChartSemanticModeChange(InsightsChartSemanticMode.HIERARCHY)
        advanceUntilIdle()

        assertEquals(InsightsResultDisplayMode.CHART, viewModel.uiState.resultDisplayMode)
        assertEquals(InsightsChartSemanticMode.HIERARCHY, viewModel.uiState.chartSemanticMode)
        assertEquals(DataTreePeriod.DAY, fakeQueryGateway.lastTreeRequest?.period)
        assertTrue(viewModel.uiState.activeResult is QueryResult.Tree)
    }

    @Test
    fun changingTreeDate_reloadsTreeUsingSelectedLevel() = runTest {
        val fakeQueryGateway = FakeTreeQueryGateway()
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeTreeInsightsGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
        viewModel.onChartSemanticModeChange(InsightsChartSemanticMode.HIERARCHY)
        advanceUntilIdle()
        viewModel.onTreeLevelChange(1)
        advanceUntilIdle()

        viewModel.onInsightsDateChange("20260412")
        advanceUntilIdle()

        val request = fakeQueryGateway.lastTreeRequest
        assertTrue(request != null)
        assertEquals(DataTreePeriod.DAY, request?.period)
        assertEquals("20260412", request?.periodArgument)
        assertEquals(1, request?.level)
    }

    @Test
    fun changingInsightsMode_whileHierarchySelected_reloadsTreeForNewPeriod() = runTest {
        val fakeQueryGateway = FakeTreeQueryGateway()
        val viewModel = QueryInsightsViewModel(
            insightsGateway = FakeTreeInsightsGateway(),
            queryGateway = fakeQueryGateway
        )

        viewModel.onResultDisplayModeChange(InsightsResultDisplayMode.CHART)
        viewModel.onChartSemanticModeChange(InsightsChartSemanticMode.HIERARCHY)
        advanceUntilIdle()
        viewModel.onTreeLevelChange(1)
        advanceUntilIdle()

        viewModel.onInsightsModeChange(InsightsMode.WEEK)
        advanceUntilIdle()

        val request = fakeQueryGateway.lastTreeRequest
        assertTrue(request != null)
        assertEquals(DataTreePeriod.WEEK, request?.period)
        assertEquals(1, request?.level)
    }

    @Test
    fun hierarchyMode_resolvesTreeWhileDetailsResolvesMarkdownOrActivities() {
        val markdown = QueryResult.Insights(text = "markdown")
        val tree = QueryResult.Tree(
            period = DataTreePeriod.DAY,
            nodes = emptyList(),
            found = true
        )
        val state = initialQueryInsightsUiState().copy(
            activeResult = tree,
            chartSemanticMode = InsightsChartSemanticMode.HIERARCHY,
            insightsResultsByPeriod = mapOf(DataTreePeriod.DAY to markdown)
        )

        assertEquals(
            markdown,
            resolveDisplayResult(
                state,
                DataTreePeriod.DAY,
                InsightsResultDisplayMode.DETAILS,
                InsightsParameterSection.DAY
            )
        )
        assertEquals(
            markdown,
            resolveDisplayResult(
                state,
                DataTreePeriod.DAY,
                InsightsResultDisplayMode.DETAILS,
                InsightsParameterSection.ACTIVITIES
            )
        )
        assertEquals(
            tree,
            resolveDisplayResult(
                state,
                DataTreePeriod.DAY,
                InsightsResultDisplayMode.CHART,
                InsightsParameterSection.DAY
            )
        )
    }
}

private class FakeTreeInsightsGateway : InsightsGateway {
    override suspend fun insightsMarkdown(request: TemporalInsightsQueryRequest): InsightsCallResult =
        InsightsCallResult(
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
        lastTreeRequest = params
        treeRequestCount += 1
        return treeResult
    }

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

