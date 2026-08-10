package com.example.tracer

internal class NativeInsightsTranslator(
    private val responseCodec: NativeResponseCodec
) {
    fun fromInitFailure(initResult: NativeCallResult): InsightsCallResult {
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
                operationId = initResult.operationId,
                errorContract = initPayload.errorContract
            )
        ).toLegacyInsightsCallResult()
    }

    fun fromRuntimePathsMissing(operationId: String): InsightsCallResult {
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
        ).toLegacyInsightsCallResult()
    }

    fun fromExecutionFailure(
        message: String,
        rawResponse: String,
        operationId: String
    ): InsightsCallResult {
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
        ).toLegacyInsightsCallResult()
    }

    fun fromNativeResponse(
        response: String,
        operationId: String
    ): InsightsCallResult {
        val payload = responseCodec.parse(response)
        val errorLogPath = if (payload.ok) {
            ""
        } else {
            extractErrorLogPath(payload.content)
        }
        val outputText = if (payload.ok) {
            InsightsOutputPolicy.preserveRaw(payload.content)
        } else {
            appendFailureContext(
                message = payload.errorMessage.ifEmpty { "runtime insights failed." },
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
                operationId = operationId,
                errorContract = payload.errorContract,
                insightsWindowMetadata = payload.insightsWindowMetadata
            )
        ).toLegacyInsightsCallResult()
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
