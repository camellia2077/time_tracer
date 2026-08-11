package com.example.tracer

import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertTrue
import org.junit.Test

class QueryInsightsCompositionPipelineTest {
    @Test
    fun execute_reusesCache_andProducesCompositionTrace() = runTest {
        val gateway = FakeCompositionPipelineQueryGateway()
        var now = 5_000L
        val useCase = QueryInsightsCompositionUseCase(
            queryGateway = gateway,
            inputValidator = QueryInputValidator(),
            textProvider = DefaultQueryInsightsTextProvider,
            nowMs = {
                val current = now
                now += 10
                current
            }
        )
        val inputState = QueryInsightsUiState(
            insightsMode = InsightsMode.DAY,
            insightsDate = "20260413",
            chartSemanticMode = InsightsChartSemanticMode.COMPOSITION
        )

        val first = useCase.execute(currentState = inputState, emit = {})
        val second = useCase.execute(currentState = inputState, emit = {})

        assertEquals(1, gateway.compositionQueryCount)
        assertNotNull(first.compositionChartRenderModel)
        assertNotNull(second.compositionChartRenderModel)
        assertEquals(false, first.compositionChartLastTrace?.cacheHit)
        assertEquals(true, second.compositionChartLastTrace?.cacheHit)
        assertTrue(second.statusText.contains("cache=true"))
        assertEquals(2, second.compositionChartRenderModel?.tree?.size)
        assertEquals(3L, second.compositionChartRenderModel?.tree?.first()?.occurrenceCount)
    }
}

private class FakeCompositionPipelineQueryGateway : QueryGateway {
    var compositionQueryCount: Int = 0

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
        InsightsChartQueryResult(ok = true, data = null, message = "unused")

    override suspend fun queryInsightsComposition(
        params: InsightsCompositionQueryParams
    ): InsightsCompositionQueryResult {
        compositionQueryCount += 1
        return InsightsCompositionQueryResult(
            ok = true,
            data = InsightsCompositionData(
                tree = listOf(
                    TreeNode(name = "study", durationSeconds = 5400L, occurrenceCount = 3L),
                    TreeNode(name = "sleep", durationSeconds = 3600L, occurrenceCount = 2L)
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
