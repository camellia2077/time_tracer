package com.example.tracer

import org.json.JSONObject

internal class NativeResponseCodec {
    fun parse(response: String): NativeResponsePayload {
        return try {
            val json = JSONObject(response)
            NativeResponsePayload(
                ok = json.optBoolean("ok", false),
                content = json.optString("content", ""),
                errorMessage = json.optString("error_message", "")
            )
        } catch (_: Exception) {
            NativeResponsePayload(
                ok = false,
                content = "",
                errorMessage = "Invalid native response."
            )
        }
    }
}
