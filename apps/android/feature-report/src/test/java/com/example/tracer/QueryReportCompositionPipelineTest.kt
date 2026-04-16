package com.example.tracer

import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertTrue
import org.junit.Test

class QueryReportCompositionPipelineTest {
    @Test
    fun execute_reusesCache_andProducesCompositionTrace() = runTest {
        val gateway = FakeCompositionPipelineQueryGateway()
        var now = 5_000L
        val useCase = QueryReportCompositionUseCase(
            queryGateway = gateway,
            inputValidator = QueryInputValidator(),
            textProvider = DefaultQueryReportTextProvider,
            nowMs = {
                val current = now
                now += 10
                current
            }
        )
        val inputState = QueryReportUiState(
            reportMode = ReportMode.DAY,
            reportDate = "20260413",
            chartSemanticMode = ReportChartSemanticMode.COMPOSITION
        )

        val first = useCase.execute(currentState = inputState, emit = {})
        val second = useCase.execute(currentState = inputState, emit = {})

        assertEquals(1, gateway.compositionQueryCount)
        assertNotNull(first.compositionChartRenderModel)
        assertNotNull(second.compositionChartRenderModel)
        assertEquals(false, first.compositionChartLastTrace?.cacheHit)
        assertEquals(true, second.compositionChartLastTrace?.cacheHit)
        assertTrue(second.statusText.contains("cache=true"))
        assertEquals(2, second.compositionChartRenderModel?.slices?.size)
    }
}

private class FakeCompositionPipelineQueryGateway : QueryGateway {
    var compositionQueryCount: Int = 0

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
        ReportChartQueryResult(ok = true, data = null, message = "unused")

    override suspend fun queryReportComposition(
        params: ReportCompositionQueryParams
    ): ReportCompositionQueryResult {
        compositionQueryCount += 1
        return ReportCompositionQueryResult(
            ok = true,
            data = ReportCompositionData(
                slices = listOf(
                    ReportCompositionSlice("study", 5400L, 60f),
                    ReportCompositionSlice("sleep", 3600L, 40f)
                ),
                totalDurationSeconds = 9000L,
                activeRootCount = 2,
                rangeDays = 1
            ),
            message = "ok"
        )
    }

    override suspend fun listActivityMappingNames(): ActivityMappingNamesResult =
        ActivityMappingNamesResult(ok = true, names = emptyList(), message = "ok")
}
