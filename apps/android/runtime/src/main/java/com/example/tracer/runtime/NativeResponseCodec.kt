package com.example.tracer

import org.json.JSONObject

internal class NativeResponseCodec {
    fun parse(response: String): NativeResponsePayload {
        return try {
            val json = JSONObject(response)
            NativeResponsePayload(
                ok = json.optBoolean("ok", false),
                content = json.optString("content", ""),
                errorMessage = json.optString("error_message", ""),
                insightsHashSha256 = json.optString("insights_hash_sha256", ""),
                errorContract = parseErrorContract(json),
                insightsWindowMetadata = parseInsightsWindowMetadata(json)
            )
        } catch (_: Exception) {
            NativeResponsePayload(
                ok = false,
                content = "",
                errorMessage = "Invalid native response.",
                insightsHashSha256 = ""
            )
        }
    }

    private fun parseErrorContract(json: JSONObject): InsightsErrorContract? {
        val errorCode = json.optString("error_code", "")
        val errorCategory = json.optString("error_category", "")
        val hints = mutableListOf<String>()
        val rawHints = json.optJSONArray("hints")
        if (rawHints != null) {
            for (index in 0 until rawHints.length()) {
                val hint = rawHints.optString(index, "")
                if (hint.isNotBlank()) {
                    hints += hint
                }
            }
        }
        if (errorCode.isBlank() && errorCategory.isBlank() && hints.isEmpty()) {
            return null
        }
        return InsightsErrorContract(
            errorCode = errorCode,
            errorCategory = errorCategory,
            hints = hints
        )
    }

    private fun parseInsightsWindowMetadata(json: JSONObject): InsightsWindowMetadata? {
        val hasAnyWindowField = json.has("has_records") ||
            json.has("matched_day_count") ||
            json.has("matched_record_count") ||
            json.has("start_date") ||
            json.has("end_date") ||
            json.has("requested_days")
        if (!hasAnyWindowField) {
            return null
        }
        return InsightsWindowMetadata(
            hasRecords = json.optBoolean("has_records", false),
            matchedDayCount = json.optInt("matched_day_count", 0),
            matchedRecordCount = json.optInt("matched_record_count", 0),
            startDate = json.optString("start_date", ""),
            endDate = json.optString("end_date", ""),
            requestedDays = json.optInt("requested_days", 0)
        )
    }
}
