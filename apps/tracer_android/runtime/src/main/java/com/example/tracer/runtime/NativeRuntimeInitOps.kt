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
    return JSONObject()
        .put("ok", false)
        .put("content", "")
        .put("error_message", errorMessage)
        .toString()
}

internal fun buildNativeCallFailure(prefix: String, error: Exception): NativeCallResult {
    val message = formatNativeFailure(prefix, error)
    return NativeCallResult(
        initialized = false,
        operationOk = false,
        rawResponse = buildNativeErrorResponseJson(message)
    )
}

internal fun buildRecordActionFailure(prefix: String, error: Exception): RecordActionResult {
    return RecordActionResult(
        ok = false,
        message = formatNativeFailure(prefix, error)
    )
}
