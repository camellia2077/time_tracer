package com.example.tracer

internal class NativeRecordTranslator(
    private val responseCodec: NativeResponseCodec
) {
    fun extractStageFailure(result: NativeCallResult, stage: String): String? {
        val failureMessage = when {
            !result.initialized -> {
                CoreError(
                    userMessage = "native init failed.",
                    debugMessage = result.rawResponse,
                    errorLogPath = result.errorLogPath,
                    operationId = result.operationId
                ).legacyMessage()
            }

            result.operationOk -> {
                null
            }

            else -> {
                val payload = responseCodec.parse(result.rawResponse)
                CoreError(
                    userMessage = payload.errorMessage.ifEmpty { result.rawResponse },
                    debugMessage = result.rawResponse,
                    errorLogPath = result.errorLogPath,
                    operationId = result.operationId
                ).legacyMessage()
            }
        } ?: return null

        return "$stage failed: $failureMessage"
    }
}
