package com.example.tracer

interface InsightsGateway {
    suspend fun insightsMarkdown(request: TemporalInsightsQueryRequest): InsightsCallResult

    suspend fun insightsStructured(
        request: TemporalInsightsQueryRequest
    ): StructuredInsightsCallResult = StructuredInsightsCallResult(
        initialized = false,
        operationOk = false,
        insights = null,
        rawResponse = ""
    )
}
