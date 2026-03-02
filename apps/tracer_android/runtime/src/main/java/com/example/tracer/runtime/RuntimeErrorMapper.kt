package com.example.tracer

internal class RuntimeErrorMapper {
    fun formatFailure(prefix: String, error: Exception): String =
        formatNativeFailure(prefix, error)

    fun appendContext(
        message: String,
        operationId: String = "",
        errorLogPath: String = ""
    ): String = appendFailureContext(
        message = message,
        operationId = operationId,
        errorLogPath = errorLogPath
    )
}
