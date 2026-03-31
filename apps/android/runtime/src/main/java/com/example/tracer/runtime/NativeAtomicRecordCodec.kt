package com.example.tracer

import org.json.JSONArray
import org.json.JSONObject

internal data class NativeAtomicRecordPayload(
    val ok: Boolean,
    val message: String,
    val operationId: String,
    val warnings: List<String>,
    val rollbackFailed: Boolean,
    val retainedTransactionRoot: String
)

internal class NativeAtomicRecordCodec {
    fun parse(content: String): NativeAtomicRecordPayload? {
        if (content.isBlank()) {
            return null
        }
        return runCatching {
            val json = JSONObject(content)
            NativeAtomicRecordPayload(
                ok = json.optBoolean("ok", false),
                message = json.optString("message", ""),
                operationId = json.optString("operation_id", ""),
                warnings = json.optJSONArray("warnings").toStringList(),
                rollbackFailed = json.optBoolean("rollback_failed", false),
                retainedTransactionRoot = json.optString("retained_transaction_root", "")
            )
        }.getOrNull()
    }

    private fun JSONArray?.toStringList(): List<String> {
        if (this == null) {
            return emptyList()
        }
        val items = mutableListOf<String>()
        for (index in 0 until length()) {
            items += optString(index, "")
        }
        return items
    }
}
