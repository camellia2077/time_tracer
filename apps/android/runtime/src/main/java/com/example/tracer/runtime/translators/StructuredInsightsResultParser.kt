package com.example.tracer

internal class StructuredInsightsResultParser(
    private val decoder: StructuredInsightsJsonDecoder = StructuredInsightsJsonDecoder(),
    private val translator: StructuredInsightsModelTranslator = StructuredInsightsModelTranslator()
) {
    fun parse(result: InsightsCallResult): StructuredInsightsCallResult {
        if (!result.operationOk) {
            return StructuredInsightsCallResult(
                initialized = result.initialized,
                operationOk = false,
                insights = null,
                rawResponse = result.rawResponse,
                errorMessage = result.outputText,
                operationId = result.operationId
            )
        }
        return runCatching {
            val payload = decoder.decode(result.rawResponse)
            StructuredInsightsCallResult(
                initialized = result.initialized,
                operationOk = true,
                insights = if (payload.isDaily) translator.translate(payload) else null,
                rawResponse = result.rawResponse,
                statuses = payload.statuses,
                operationId = result.operationId
            )
        }.getOrElse { error ->
            StructuredInsightsCallResult(
                initialized = result.initialized,
                operationOk = false,
                insights = null,
                rawResponse = result.rawResponse,
                errorMessage = error.message ?: "Invalid structured insights payload.",
                operationId = result.operationId
            )
        }
    }
}
