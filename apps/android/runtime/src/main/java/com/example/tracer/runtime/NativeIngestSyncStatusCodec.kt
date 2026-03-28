package com.example.tracer

import org.json.JSONArray
import org.json.JSONObject

internal data class NativeTxtIngestSyncStatusEntry(
    val monthKey: String,
    val txtRelativePath: String,
    val txtContentHashSha256: String,
    val ingestedAtUnixMs: Long
)

internal data class NativeTxtIngestSyncStatusResult(
    val ok: Boolean,
    val items: List<NativeTxtIngestSyncStatusEntry>,
    val errorMessage: String
)

internal class NativeIngestSyncStatusCodec {
    fun buildRequest(months: List<String>): String {
        if (months.isEmpty()) {
            return "{}"
        }

        val values = JSONArray()
        months.forEach(values::put)
        return JSONObject()
            .put("months", values)
            .toString()
    }

    fun parse(response: String): NativeTxtIngestSyncStatusResult {
        return try {
            val json = JSONObject(response)
            val itemsJson = json.optJSONArray("items") ?: JSONArray()
            val items = buildList(itemsJson.length()) {
                for (index in 0 until itemsJson.length()) {
                    val item = itemsJson.optJSONObject(index) ?: continue
                    add(
                        NativeTxtIngestSyncStatusEntry(
                            monthKey = item.optString("month_key", ""),
                            txtRelativePath = item.optString("txt_relative_path", ""),
                            txtContentHashSha256 = item.optString("txt_content_hash_sha256", ""),
                            ingestedAtUnixMs = item.optLong("ingested_at_unix_ms", 0L)
                        )
                    )
                }
            }
            NativeTxtIngestSyncStatusResult(
                ok = json.optBoolean("ok", false),
                items = items,
                errorMessage = json.optString("error_message", "")
            )
        } catch (_: Exception) {
            NativeTxtIngestSyncStatusResult(
                ok = false,
                items = emptyList(),
                errorMessage = "Invalid native ingest sync status response."
            )
        }
    }
}
