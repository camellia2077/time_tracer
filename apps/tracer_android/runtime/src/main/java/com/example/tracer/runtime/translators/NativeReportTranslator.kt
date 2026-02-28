package com.example.tracer

internal class NativeReportTranslator(
    private val responseCodec: NativeResponseCodec
) {
    fun fromInitFailure(initResult: NativeCallResult): ReportCallResult {
        val initPayload = responseCodec.parse(initResult.rawResponse)
        val outputText = appendFailureContext(
            message = initPayload.errorMessage.ifEmpty { "native init failed." },
            operationId = initResult.operationId,
            errorLogPath = initResult.errorLogPath
        )
        return DomainResult.Success(
            DomainNativeEnvelope(
                initialized = false,
                operationOk = false,
                outputText = outputText,
                rawResponse = initResult.rawResponse,
                errorLogPath = initResult.errorLogPath,
                operationId = initResult.operationId
            )
        ).toLegacyReportCallResult()
    }

    fun fromRuntimePathsMissing(operationId: String): ReportCallResult {
        return DomainResult.Success(
            DomainNativeEnvelope(
                initialized = false,
                operationOk = false,
                outputText = appendFailureContext(
                    message = "Runtime paths are not initialized.",
                    operationId = operationId
                ),
                rawResponse = "",
                operationId = operationId
            )
        ).toLegacyReportCallResult()
    }

    fun fromExecutionFailure(
        message: String,
        rawResponse: String,
        operationId: String
    ): ReportCallResult {
        return DomainResult.Success(
            DomainNativeEnvelope(
                initialized = true,
                operationOk = false,
                outputText = appendFailureContext(
                    message = message,
                    operationId = operationId
                ),
                rawResponse = rawResponse,
                operationId = operationId
            )
        ).toLegacyReportCallResult()
    }

    fun fromNativeResponse(
        response: String,
        operationId: String
    ): ReportCallResult {
        val payload = responseCodec.parse(response)
        val errorLogPath = if (payload.ok) {
            ""
        } else {
            extractErrorLogPath(payload.content)
        }
        val outputText = if (payload.ok) {
            ReportOutputPolicy.preserveRaw(payload.content)
        } else {
            appendFailureContext(
                message = payload.errorMessage.ifEmpty { "runtime report failed." },
                operationId = operationId,
                errorLogPath = errorLogPath
            )
        }
        return DomainResult.Success(
            DomainNativeEnvelope(
                initialized = true,
                operationOk = payload.ok,
                outputText = outputText,
                rawResponse = response,
                errorLogPath = errorLogPath,
                operationId = operationId
            )
        ).toLegacyReportCallResult()
    }

    private fun extractErrorLogPath(content: String): String {
        if (content.isBlank()) {
            return ""
        }
        return runCatching {
            org.json.JSONObject(content).optString("error_log_path", "")
        }.getOrDefault("")
    }
}
