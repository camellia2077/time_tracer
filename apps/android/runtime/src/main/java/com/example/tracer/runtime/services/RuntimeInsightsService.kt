package com.example.tracer

internal class RuntimeInsightsService(
    private val insightsDelegate: RuntimeInsightsDelegate
) {
    suspend fun insightsMarkdown(request: TemporalInsightsQueryRequest): InsightsCallResult =
        insightsDelegate.insightsMarkdown(request)

    suspend fun insightsStructured(
        request: TemporalInsightsQueryRequest
    ): StructuredInsightsCallResult = insightsDelegate.insightsStructured(request)
}
