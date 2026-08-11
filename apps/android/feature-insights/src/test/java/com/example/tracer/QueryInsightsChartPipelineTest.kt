package com.example.tracer

import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertTrue
import org.junit.Test

class QueryInsightsChartPipelineTest {
    @Test
    fun execute_withInvalidLookback_returnsValidationErrorWithoutQuery() = runTest {
        val gateway = FakePipelineQueryGateway()
        val useCase = QueryInsightsChartUseCase(
            queryGateway = gateway,
            inputValidator = QueryInputValidator(),
            textProvider = DefaultQueryInsightsTextProvider
        )

        val result = useCase.execute(
            currentState = QueryInsightsUiState(
                insightsMode = InsightsMode.RECENT,
                insightsRecentDays = "0"
            ),
            emit = {}
        )

        assertEquals(0, gateway.chartQueryCount)
        assertTrue(result.trendChartError.isNotBlank())
        assertEquals(InsightsResultDisplayMode.CHART, result.resultDisplayMode)
    }

    @Test
    fun execute_reusesCache_andProducesTrace() = runTest {
        val gateway = FakePipelineQueryGateway()
        var now = 1_000L
        val useCase = QueryInsightsChartUseCase(
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
            trendChartSelectedRoot = "study",
            insightsMode = InsightsMode.RECENT,
            insightsRecentDays = "7"
        )

        val first = useCase.execute(
            currentState = inputState,
            emit = {}
        )
        val second = useCase.execute(
            currentState = inputState,
            emit = {}
        )

        assertEquals(1, gateway.chartQueryCount)
        assertNotNull(first.trendChartRenderModel)
        assertNotNull(second.trendChartRenderModel)
        assertEquals(false, first.trendChartLastTrace?.cacheHit)
        assertEquals(true, second.trendChartLastTrace?.cacheHit)
        assertTrue(second.statusText.contains("cache=true"))
        assertEquals(2, second.trendChartRenderModel?.points?.size)
    }

    @Test
    fun execute_withMonthMode_mapsToInclusiveDateRange() = runTest {
        val gateway = FakePipelineQueryGateway()
        val useCase = QueryInsightsChartUseCase(
            queryGateway = gateway,
            inputValidator = QueryInputValidator(),
            textProvider = DefaultQueryInsightsTextProvider
        )

        val result = useCase.execute(
            currentState = QueryInsightsUiState(
                insightsMode = InsightsMode.MONTH,
                insightsMonth = "202602"
            ),
            emit = {}
        )

        assertEquals(1, gateway.chartQueryCount)
        assertEquals("2026-02-01", gateway.lastChartParams?.fromDateIso)
        assertEquals("2026-02-28", gateway.lastChartParams?.toDateIso)
        assertEquals(28, gateway.lastChartParams?.lookbackDays)
        assertTrue(result.trendChartError.isEmpty())
    }

    @Test
    fun mapCorePayloadToDomainModel_legacyStats_usesDerivedFallbackValues() {
        val domain = mapCorePayloadToDomainModel(
            InsightsChartData(
                roots = listOf("study"),
                selectedRoot = "study",
                lookbackDays = 7,
                points = listOf(
                    InsightsChartPoint("2026-02-11", 3600L, epochDay = 20495L),
                    InsightsChartPoint("2026-02-10", 0L, epochDay = 20494L)
                ),
                averageDurationSeconds = null,
                totalDurationSeconds = null,
                activeDays = null,
                rangeDays = null,
                usesLegacyStatsFallback = true
            )
        )

        assertEquals(listOf("2026-02-10", "2026-02-11"), domain.points.map { it.date })
        assertEquals(3600L, domain.totalDurationSeconds)
        assertEquals(1, domain.activeDays)
        assertEquals(2, domain.rangeDays)
        assertEquals(3600L, domain.averageDurationSeconds)
        assertEquals(true, domain.usesLegacyStatsFallback)
    }

    @Test
    fun mapCorePayloadToCompositionRenderModel_preservesAverageOccurrenceCount() {
        val model = mapCorePayloadToCompositionRenderModel(
            InsightsCompositionData(
                tree = listOf(
                    TreeNode(
                        name = "study",
                        durationSeconds = 3_600L,
                        occurrenceCount = 4L,
                        averageOccurrenceCount = 2.0
                    )
                ),
                totalDurationSeconds = 3_600L,
                activeRootCount = 1,
                activeDays = 2,
                rangeDays = 7
            )
        )

        assertEquals(2.0, model.tree.single().averageOccurrenceCount ?: -1.0, 0.0)
    }
}

private class FakePipelineQueryGateway : QueryGateway {
    var chartQueryCount: Int = 0
    var lastChartParams: InsightsChartQueryParams? = null

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

    override suspend fun queryInsightsChart(params: InsightsChartQueryParams): InsightsChartQueryResult {
        chartQueryCount += 1
        lastChartParams = params
        return InsightsChartQueryResult(
            ok = true,
            data = InsightsChartData(
                roots = listOf("sleep", "study"),
                selectedRoot = params.root.orEmpty(),
                lookbackDays = params.lookbackDays,
                points = listOf(
                    InsightsChartPoint("2026-02-12", 3600L, epochDay = 20496L),
                    InsightsChartPoint("2026-02-13", 5400L, epochDay = 20497L)
                ),
                averageDurationSeconds = 4500L,
                totalDurationSeconds = 9000L,
                activeDays = 2,
                rangeDays = 2,
                usesLegacyStatsFallback = false,
                schemaVersion = 1,
                usesSchemaVersionFallback = false
            ),
            message = "ok"
        )
    }

    override suspend fun queryInsightsComposition(
        params: InsightsCompositionQueryParams
    ): InsightsCompositionQueryResult = InsightsCompositionQueryResult(
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

    override suspend fun listActivityMappingNames(): ActivityMappingNamesResult =
        ActivityMappingNamesResult(ok = true, names = emptyList(), message = "ok")
}
