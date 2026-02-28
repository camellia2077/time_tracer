package com.example.tracer

import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertTrue
import org.junit.Test

class QueryReportChartPipelineTest {
    @Test
    fun execute_withInvalidLookback_returnsValidationErrorWithoutQuery() = runTest {
        val gateway = FakePipelineQueryGateway()
        val useCase = QueryReportChartUseCase(
            queryGateway = gateway,
            inputValidator = QueryInputValidator(),
            textProvider = DefaultQueryReportTextProvider
        )

        val result = useCase.execute(
            currentState = QueryReportUiState(
                chartDateInputMode = ChartDateInputMode.LOOKBACK,
                chartLookbackDays = "0"
            ),
            emit = {}
        )

        assertEquals(0, gateway.chartQueryCount)
        assertTrue(result.chartError.isNotBlank())
        assertEquals(ReportResultDisplayMode.CHART, result.resultDisplayMode)
    }

    @Test
    fun execute_reusesCache_andProducesTrace() = runTest {
        val gateway = FakePipelineQueryGateway()
        var now = 1_000L
        val useCase = QueryReportChartUseCase(
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
            chartSelectedRoot = "study",
            chartLookbackDays = "7",
            chartDateInputMode = ChartDateInputMode.LOOKBACK
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
        assertNotNull(first.chartRenderModel)
        assertNotNull(second.chartRenderModel)
        assertEquals(false, first.chartLastTrace?.cacheHit)
        assertEquals(true, second.chartLastTrace?.cacheHit)
        assertTrue(second.statusText.contains("cache=true"))
        assertEquals(2, second.chartRenderModel?.points?.size)
    }

    @Test
    fun mapCorePayloadToDomainModel_legacyStats_usesDerivedFallbackValues() {
        val domain = mapCorePayloadToDomainModel(
            ReportChartData(
                roots = listOf("study"),
                selectedRoot = "study",
                lookbackDays = 7,
                points = listOf(
                    ReportChartPoint("2026-02-11", 3600L, epochDay = 20495L),
                    ReportChartPoint("2026-02-10", 0L, epochDay = 20494L)
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
        assertEquals(1800L, domain.averageDurationSeconds)
        assertEquals(true, domain.usesLegacyStatsFallback)
    }
}

private class FakePipelineQueryGateway : QueryGateway {
    var chartQueryCount: Int = 0

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

    override suspend fun queryReportChart(params: ReportChartQueryParams): ReportChartQueryResult {
        chartQueryCount += 1
        return ReportChartQueryResult(
            ok = true,
            data = ReportChartData(
                roots = listOf("sleep", "study"),
                selectedRoot = params.root.orEmpty(),
                lookbackDays = params.lookbackDays,
                points = listOf(
                    ReportChartPoint("2026-02-12", 3600L, epochDay = 20496L),
                    ReportChartPoint("2026-02-13", 5400L, epochDay = 20497L)
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

    override suspend fun listActivityMappingNames(): ActivityMappingNamesResult =
        ActivityMappingNamesResult(ok = true, names = emptyList(), message = "ok")
}
