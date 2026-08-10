package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

internal class RuntimeInsightsDelegate(
    private val executeInsightsAfterInit: (
        operationName: String,
        action: (RuntimePaths) -> String
    ) -> InsightsCallResult,
    private val nativeInsightsJson: (String) -> String,
    private val requestCodec: TemporalInsightsRequestJsonCodec = TemporalInsightsRequestJsonCodec(),
    private val structuredInsightsResultParser: StructuredInsightsResultParser =
        StructuredInsightsResultParser()
) {
    suspend fun insightsMarkdown(request: TemporalInsightsQueryRequest): InsightsCallResult =
        withContext(Dispatchers.IO) {
            val requestJson = requestCodec.encodeQuery(request)
            executeInsightsAfterInit(buildInsightsOperationName(request.displayMode)) {
                nativeInsightsJson(requestJson)
            }
        }

    suspend fun insightsStructured(
        request: TemporalInsightsQueryRequest
    ): StructuredInsightsCallResult = withContext(Dispatchers.IO) {
        val requestJson = requestCodec.encodeStructuredQuery(request)
        val result = executeInsightsAfterInit("structured_${request.displayMode.wireValue}") {
            nativeInsightsJson(requestJson)
        }
        structuredInsightsResultParser.parse(result)
    }

    private fun buildInsightsOperationName(displayMode: InsightsDisplayMode): String {
        val suffix = when (displayMode) {
            InsightsDisplayMode.DAY -> "day"
            InsightsDisplayMode.MONTH -> "month"
            InsightsDisplayMode.YEAR -> "year"
            InsightsDisplayMode.WEEK -> "week"
            InsightsDisplayMode.RECENT -> "recent"
            InsightsDisplayMode.RANGE -> "range"
        }
        return "native_insights_$suffix"
    }

}
