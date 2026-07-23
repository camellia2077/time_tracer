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

    fun parseActivityNameConversion(
        response: String,
        fallbackContent: String
    ): TxtActivityNameConversionResult =
        try {
            val json = JSONObject(response)
            TxtActivityNameConversionResult(
                ok = json.optBoolean("ok", false),
                convertedContent = json.optString("converted_content", fallbackContent),
                message = json.optString("error_message", "")
            )
        } catch (_: Exception) {
            TxtActivityNameConversionResult(
                ok = false,
                convertedContent = fallbackContent,
                message = "Invalid native TXT response."
            )
        }

    fun parseCanonicalActivityReplacement(
        response: String,
        fallbackContent: String
    ): TxtCanonicalActivityReplacementResult =
        try {
            val json = JSONObject(response)
            TxtCanonicalActivityReplacementResult(
                ok = json.optBoolean("ok", false),
                updatedContent = json.optString("updated_content", fallbackContent),
                message = json.optString("error_message", "")
            )
        } catch (_: Exception) {
            TxtCanonicalActivityReplacementResult(
                ok = false,
                updatedContent = fallbackContent,
                message = "Invalid native TXT response."
            )
        }
}
