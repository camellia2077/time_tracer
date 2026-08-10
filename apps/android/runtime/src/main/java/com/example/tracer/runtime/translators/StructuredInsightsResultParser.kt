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
            StructuredInsightsCallResult(
                initialized = result.initialized,
                operationOk = true,
                insights = translator.translate(decoder.decode(result.rawResponse)),
                rawResponse = result.rawResponse,
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
