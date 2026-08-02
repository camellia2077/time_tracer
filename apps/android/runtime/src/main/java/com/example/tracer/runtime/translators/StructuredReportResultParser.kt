package com.example.tracer

internal class StructuredReportResultParser(
    private val decoder: StructuredReportJsonDecoder = StructuredReportJsonDecoder(),
    private val translator: StructuredReportModelTranslator = StructuredReportModelTranslator()
) {
    fun parse(result: ReportCallResult): StructuredReportCallResult {
        if (!result.operationOk) {
            return StructuredReportCallResult(
                initialized = result.initialized,
                operationOk = false,
                report = null,
                rawResponse = result.rawResponse,
                errorMessage = result.outputText,
                operationId = result.operationId
            )
        }
        return runCatching {
            StructuredReportCallResult(
                initialized = result.initialized,
                operationOk = true,
                report = translator.translate(decoder.decode(result.rawResponse)),
                rawResponse = result.rawResponse,
                operationId = result.operationId
            )
        }.getOrElse { error ->
            StructuredReportCallResult(
                initialized = result.initialized,
                operationOk = false,
                report = null,
                rawResponse = result.rawResponse,
                errorMessage = error.message ?: "Invalid structured report payload.",
                operationId = result.operationId
            )
        }
    }
}
