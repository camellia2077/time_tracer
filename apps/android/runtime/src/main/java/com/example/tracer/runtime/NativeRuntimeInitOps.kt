package com.example.tracer

import kotlinx.coroutines.CancellationException
import org.json.JSONObject

internal fun formatNativeFailure(prefix: String, error: Exception): String {
    if (error is CancellationException) {
        throw error
    }
    val details = error.message?.takeIf { it.isNotBlank() } ?: (error::class.simpleName ?: "Unknown error")
    return "$prefix: $details"
}

internal fun buildNativeErrorResponseJson(errorMessage: String): String {
    return buildNativeErrorResponseJson(
        errorMessage = errorMessage,
        operationId = "",
        errorLogPath = ""
    )
}

internal fun buildNativeErrorResponseJson(
    errorMessage: String,
    operationId: String = "",
    errorLogPath: String = ""
): String {
    val messageWithContext = appendFailureContext(
        message = errorMessage,
        operationId = operationId,
        errorLogPath = errorLogPath
    )
    val contentJson = if (errorLogPath.isNotBlank()) {
        JSONObject().put("error_log_path", errorLogPath).toString()
    } else {
        ""
    }

    return JSONObject()
        .put("ok", false)
        .put("content", contentJson)
        .put("error_message", messageWithContext)
        .put("operation_id", operationId)
        .toString()
}

internal fun buildNativeCallFailure(
    prefix: String,
    error: Exception,
    operationId: String = "",
    errorLogPath: String = ""
): NativeCallResult {
    val message = formatNativeFailure(prefix, error)
    return NativeCallResult(
        initialized = false,
        operationOk = false,
        rawResponse = buildNativeErrorResponseJson(
            errorMessage = message,
            operationId = operationId,
            errorLogPath = errorLogPath
        ),
        errorLogPath = errorLogPath,
        operationId = operationId
    )
}

internal fun appendFailureContext(
    message: String,
    operationId: String = "",
    errorLogPath: String = ""
): String {
    val details = buildList {
        if (operationId.isNotBlank()) {
            add("op=$operationId")
        }
        if (errorLogPath.isNotBlank()) {
            add("log=$errorLogPath")
        }
    }
    if (details.isEmpty()) {
        return message
    }
    return "$message [${details.joinToString(", ")}]"
}

internal fun buildRecordActionFailure(prefix: String, error: Exception): RecordActionResult {
    return RecordActionResult(
        ok = false,
        message = formatNativeFailure(prefix, error)
    )
}
