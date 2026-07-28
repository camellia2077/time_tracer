package com.example.tracer

import org.json.JSONArray
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

    fun parseAliasCanonicalRename(response: String): AliasCanonicalRenameResult =
        try {
            val json = JSONObject(response)
            val replacementsJson = json.optJSONArray("replacements") ?: JSONArray()
            val replacements = buildList {
                for (index in 0 until replacementsJson.length()) {
                    val item = replacementsJson.optJSONObject(index) ?: continue
                    add(
                        CanonicalActivityNameReplacement(
                            oldCanonical = item.optString("old_canonical", ""),
                            newCanonical = item.optString("new_canonical", "")
                        )
                    )
                }
            }
            AliasCanonicalRenameResult(
                ok = json.optBoolean("ok", false),
                updatedTomlContent = json.optString("updated_toml_content", ""),
                replacements = replacements,
                message = json.optString("error_message", "")
            )
        } catch (_: Exception) {
            AliasCanonicalRenameResult(
                ok = false,
                updatedTomlContent = "",
                replacements = emptyList(),
                message = "Invalid native alias canonical response."
            )
        }

    fun parseAliasHierarchyOperation(
        response: String,
        fallbackTomlContent: String
    ): AliasHierarchyOperationResult = try {
        val json = JSONObject(response)
        AliasHierarchyOperationResult(
            ok = json.optBoolean("ok", false),
            updatedTomlContent = json.optString("updated_toml_content", fallbackTomlContent),
            replacements = parseReplacements(json.optJSONArray("replacements")),
            aliasReplacements = parseAliasReplacements(json.optJSONArray("alias_replacements")),
            hierarchy = json.optJSONObject("hierarchy")?.let(::parseHierarchy),
            message = json.optString("error_message", "")
        )
    } catch (_: Exception) {
        AliasHierarchyOperationResult(
            ok = false,
            updatedTomlContent = fallbackTomlContent,
            replacements = emptyList(),
            message = "Invalid native alias hierarchy response."
        )
    }

    fun parseAliasHierarchyDescribe(response: String): AliasHierarchyDescribeResult = try {
        val json = JSONObject(response)
        AliasHierarchyDescribeResult(
            ok = json.optBoolean("ok", false),
            hierarchy = json.optJSONObject("hierarchy")?.let(::parseHierarchy),
            message = json.optString("error_message", "")
        )
    } catch (_: Exception) {
        AliasHierarchyDescribeResult(
            ok = false,
            message = "Invalid native alias hierarchy response."
        )
    }

    fun parseAliasHierarchyCreate(response: String): AliasHierarchyCreateResult = try {
        val json = JSONObject(response)
        AliasHierarchyCreateResult(json.optBoolean("ok", false), json.optString("toml_content", ""), json.optString("error_message", ""))
    } catch (_: Exception) {
        AliasHierarchyCreateResult(false, message = "Invalid native alias hierarchy response.")
    }

    fun parseAliasHierarchyValidation(response: String): AliasHierarchyValidationResult = try {
        val json = JSONObject(response)
        AliasHierarchyValidationResult(
            ok = json.optBoolean("ok", false),
            message = json.optString("error_message", "")
        )
    } catch (_: Exception) {
        AliasHierarchyValidationResult(false, "Invalid native alias hierarchy response.")
    }

    private fun parseReplacements(values: JSONArray?): List<CanonicalActivityNameReplacement> = buildList {
        for (index in 0 until (values?.length() ?: 0)) {
            val item = values?.optJSONObject(index) ?: continue
            add(CanonicalActivityNameReplacement(
                oldCanonical = item.optString("old_canonical", ""),
                newCanonical = item.optString("new_canonical", "")
            ))
        }
    }

    private fun parseAliasReplacements(values: JSONArray?): List<AliasKeyReplacement> = buildList {
        for (index in 0 until (values?.length() ?: 0)) {
            val item = values?.optJSONObject(index) ?: continue
            add(AliasKeyReplacement(
                oldAlias = item.optString("old_alias", ""),
                newAlias = item.optString("new_alias", "")
            ))
        }
    }

    private fun parseHierarchy(snapshot: JSONObject): AliasHierarchySnapshot = AliasHierarchySnapshot(
        parent = snapshot.optString("parent", ""),
        nodes = parseHierarchyNodes(snapshot.optJSONArray("nodes"))
    )

    private fun parseHierarchyNodes(values: JSONArray?): List<AliasHierarchyNode> = buildList {
        for (index in 0 until (values?.length() ?: 0)) {
            val item = values?.optJSONObject(index) ?: continue
            val aliases = buildList {
                val aliasValues = item.optJSONArray("aliases")
                for (aliasIndex in 0 until (aliasValues?.length() ?: 0)) {
                    add(aliasValues?.optString(aliasIndex).orEmpty())
                }
            }
            add(AliasHierarchyNode(
                canonicalKey = item.optString("canonical_key", ""),
                path = item.optString("path", ""),
                isGroup = item.optBoolean("is_group", false),
                aliases = aliases,
                children = parseHierarchyNodes(item.optJSONArray("children"))
            ))
        }
    }
}
