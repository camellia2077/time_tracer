package com.example.tracer

import org.json.JSONObject

internal class NativeTxtRuntimeCodec {
    fun parseDayMarker(response: String): TxtDayMarkerResult =
        try {
            val json = JSONObject(response)
            TxtDayMarkerResult(
                ok = json.optBoolean("ok", false),
                normalizedDayMarker = json.optString("normalized_day_marker", ""),
                message = json.optString("error_message", "")
            )
        } catch (_: Exception) {
            TxtDayMarkerResult(
                ok = false,
                normalizedDayMarker = "",
                message = "Invalid native TXT response."
            )
        }

    fun parseResolve(response: String): TxtDayBlockResolveResult =
        try {
            val json = JSONObject(response)
            TxtDayBlockResolveResult(
                ok = json.optBoolean("ok", false),
                normalizedDayMarker = json.optString("normalized_day_marker", ""),
                found = json.optBoolean("found", false),
                isMarkerValid = json.optBoolean("is_marker_valid", false),
                canSave = json.optBoolean("can_save", false),
                dayBody = json.optString("day_body", ""),
                dayContentIsoDate = json.optString("day_content_iso_date", "")
                    .takeIf { it.isNotBlank() },
                message = json.optString("error_message", "")
            )
        } catch (_: Exception) {
            TxtDayBlockResolveResult(
                ok = false,
                normalizedDayMarker = "",
                found = false,
                isMarkerValid = false,
                canSave = false,
                dayBody = "",
                dayContentIsoDate = null,
                message = "Invalid native TXT response."
            )
        }

    fun parseReplace(response: String): TxtDayBlockReplaceResult =
        try {
            val json = JSONObject(response)
            TxtDayBlockReplaceResult(
                ok = json.optBoolean("ok", false),
                normalizedDayMarker = json.optString("normalized_day_marker", ""),
                found = json.optBoolean("found", false),
                isMarkerValid = json.optBoolean("is_marker_valid", false),
                updatedContent = json.optString("updated_content", ""),
                message = json.optString("error_message", "")
            )
        } catch (_: Exception) {
            TxtDayBlockReplaceResult(
                ok = false,
                normalizedDayMarker = "",
                found = false,
                isMarkerValid = false,
                updatedContent = "",
                message = "Invalid native TXT response."
            )
        }
}
