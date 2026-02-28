package com.example.tracer

data class DomainNativeEnvelope(
    val initialized: Boolean,
    val operationOk: Boolean,
    val rawResponse: String,
    val outputText: String = "",
    val errorLogPath: String = "",
    val operationId: String = ""
)

fun DomainResult<DomainNativeEnvelope>.toLegacyNativeCallResult(
    failureRawResponse: (DomainError) -> String = { error -> error.legacyMessage() }
): NativeCallResult {
    return fold(
        onSuccess = { envelope ->
            NativeCallResult(
                initialized = envelope.initialized,
                operationOk = envelope.operationOk,
                rawResponse = envelope.rawResponse,
                errorLogPath = envelope.errorLogPath,
                operationId = envelope.operationId
            )
        },
        onFailure = { error ->
            NativeCallResult(
                initialized = false,
                operationOk = false,
                rawResponse = failureRawResponse(error),
                errorLogPath = error.errorLogPath,
                operationId = error.operationId
            )
        }
    )
}

fun DomainResult<DomainNativeEnvelope>.toLegacyReportCallResult(
    failureRawResponse: (DomainError) -> String = { error -> error.legacyMessage() },
    failureOutput: (DomainError) -> String = { error -> error.legacyMessage() }
): ReportCallResult {
    return fold(
        onSuccess = { envelope ->
            ReportCallResult(
                initialized = envelope.initialized,
                operationOk = envelope.operationOk,
                outputText = envelope.outputText,
                rawResponse = envelope.rawResponse,
                errorLogPath = envelope.errorLogPath,
                operationId = envelope.operationId
            )
        },
        onFailure = { error ->
            ReportCallResult(
                initialized = false,
                operationOk = false,
                outputText = failureOutput(error),
                rawResponse = failureRawResponse(error),
                errorLogPath = error.errorLogPath,
                operationId = error.operationId
            )
        }
    )
}

fun DomainResult<String>.toLegacyDataQueryTextResult(
    successMessage: (String) -> String = { "query ok" },
    failureOutputText: String = ""
): DataQueryTextResult {
    return fold(
        onSuccess = { outputText ->
            DataQueryTextResult(
                ok = true,
                outputText = outputText,
                message = successMessage(outputText)
            )
        },
        onFailure = { error ->
            DataQueryTextResult(
                ok = false,
                outputText = failureOutputText,
                message = error.legacyMessage(),
                operationId = error.operationId
            )
        }
    )
}

fun TreeQueryResult.toLegacyDataQueryTextResult(): DataQueryTextResult {
    val output = if (legacyText.isNotBlank()) {
        legacyText
    } else {
        ""
    }
    return DataQueryTextResult(
        ok = ok,
        outputText = output,
        message = message,
        operationId = operationId
    )
}
