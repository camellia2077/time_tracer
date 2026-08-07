package com.example.tracer

import android.util.Log
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

private const val MAPPING_QUERY_RECORD_TAG = "TimeTracerRecord"

internal class RuntimeMappingQueryDelegate(
    private val runDataQuery: (DataQueryRequest) -> DataQueryTextResult
) {
    suspend fun listActivityMappingNames(): ActivityMappingNamesResult =
        withContext(Dispatchers.IO) {
            try {
                queryActivityMappingNamesFromCore()
            } catch (error: Exception) {
                ActivityMappingNamesResult(
                    ok = false,
                    names = emptyList(),
                    message = formatNativeFailure("list activity mapping names failed", error)
                )
            }
        }

    suspend fun listActivityHierarchyLeafKeys(): ActivityMappingNamesResult =
        withContext(Dispatchers.IO) {
            try {
                queryActivityHierarchyLeafKeysFromCore()
            } catch (error: Exception) {
                ActivityMappingNamesResult(
                    ok = false,
                    names = emptyList(),
                    message = formatNativeFailure("list activity alias keys failed", error)
                )
            }
        }

    suspend fun listActivityHierarchyLeafMappings(): ActivityHierarchyLeafMappingListResult =
        withContext(Dispatchers.IO) {
            try {
                queryActivityHierarchyLeafMappingsFromCore()
            } catch (error: Exception) {
                ActivityHierarchyLeafMappingListResult(
                    ok = false,
                    entries = emptyList(),
                    message = formatNativeFailure("list activity alias mappings failed", error)
                )
            }
        }

    suspend fun listWakeKeywords(): ActivityMappingNamesResult =
        withContext(Dispatchers.IO) {
            try {
                queryWakeKeywordsFromCore()
            } catch (error: Exception) {
                ActivityMappingNamesResult(
                    ok = false,
                    names = emptyList(),
                    message = formatNativeFailure("list wake keywords failed", error)
                )
            }
        }

    suspend fun listAuthorableEventTokens(): ActivityMappingNamesResult =
        withContext(Dispatchers.IO) {
            try {
                queryAuthorableEventTokensFromCore()
            } catch (error: Exception) {
                ActivityMappingNamesResult(
                    ok = false,
                    names = emptyList(),
                    message = formatNativeFailure("list authorable event tokens failed", error)
                )
            }
        }

    fun queryAuthorableEventTokensFromCore(): ActivityMappingNamesResult {
        return queryNamedMappingSet(
            action = NativeBridge.QUERY_ACTION_AUTHORABLE_EVENT_TOKENS,
            failurePrefix = "authorable event tokens query failed",
            emptyNamesMessage = "authorable event tokens query failed: empty authorable token set.",
            successMessageTemplate = "Loaded %d authorable event tokens."
        )
    }

    private fun queryActivityMappingNamesFromCore(): ActivityMappingNamesResult {
        val queryResult = runDataQuery(
            DataQueryRequest(
                action = NativeBridge.QUERY_ACTION_MAPPING_NAMES
            )
        )
        if (!queryResult.ok) {
            return ActivityMappingNamesResult(
                ok = false,
                names = emptyList(),
                message = appendFailureContext(
                    message = "mapping names query failed: ${queryResult.message}",
                    operationId = queryResult.operationId
                ),
                operationId = queryResult.operationId
            )
        }

        val names = parseMappingNamesContent(queryResult.outputText).sorted()
        if (names.isEmpty()) {
            return ActivityMappingNamesResult(
                ok = false,
                names = emptyList(),
                message = appendFailureContext(
                    message = "mapping names query failed: empty names.",
                    operationId = queryResult.operationId
                ),
                operationId = queryResult.operationId
            )
        }

        return ActivityMappingNamesResult(
            ok = true,
            names = names,
            message = "Loaded ${names.size} mapping names.",
            operationId = queryResult.operationId
        )
    }

    private fun queryActivityHierarchyLeafMappingsFromCore(): ActivityHierarchyLeafMappingListResult {
        val queryResult = runDataQuery(
            DataQueryRequest(
                action = NativeBridge.QUERY_ACTION_ACTIVITY_ALIAS_MAPPINGS,
                outputMode = DataQueryOutputMode.SEMANTIC_JSON
            )
        )
        if (!queryResult.ok) {
            return ActivityHierarchyLeafMappingListResult(
                ok = false,
                entries = emptyList(),
                message = appendFailureContext(
                    message = "activity alias mappings query failed: ${queryResult.message}",
                    operationId = queryResult.operationId
                ),
                operationId = queryResult.operationId
            )
        }

        val entries = parseActivityHierarchyLeafMappingsContent(queryResult.outputText)
        if (entries.isEmpty()) {
            return ActivityHierarchyLeafMappingListResult(
                ok = false,
                entries = emptyList(),
                message = appendFailureContext(
                    message = "activity alias mappings query failed: empty mappings.",
                    operationId = queryResult.operationId
                ),
                operationId = queryResult.operationId
            )
        }

        return ActivityHierarchyLeafMappingListResult(
            ok = true,
            entries = entries,
            message = "Loaded ${entries.size} activity alias mapping entries.",
            operationId = queryResult.operationId
        )
    }

    private fun queryActivityHierarchyLeafKeysFromCore(): ActivityMappingNamesResult {
        return queryNamedMappingSet(
            action = NativeBridge.QUERY_ACTION_MAPPING_ALIAS_KEYS,
            failurePrefix = "mapping alias keys query failed",
            emptyNamesMessage = "mapping alias keys query failed: empty alias keys.",
            successMessageTemplate = "Loaded %d mapping alias keys.",
            allowEmptyNames = true
        )
    }

    private fun queryWakeKeywordsFromCore(): ActivityMappingNamesResult {
        return queryNamedMappingSet(
            action = NativeBridge.QUERY_ACTION_WAKE_KEYWORDS,
            failurePrefix = "wake keywords query failed",
            emptyNamesMessage = "wake keywords query failed: empty wake keywords.",
            successMessageTemplate = "Loaded %d wake keywords."
        )
    }

    private fun queryNamedMappingSet(
        action: Int,
        failurePrefix: String,
        emptyNamesMessage: String,
        successMessageTemplate: String,
        allowEmptyNames: Boolean = false
    ): ActivityMappingNamesResult {
        Log.i(
            MAPPING_QUERY_RECORD_TAG,
            "mapping.query.start action=$action outputMode=${DataQueryOutputMode.SEMANTIC_JSON}"
        )
        val queryResult = runDataQuery(
            DataQueryRequest(
                action = action,
                // Core's mapping-name parser returns JSON. Without this mode
                // Core renders the names as newline-delimited text, which
                // makes wake_keywords appear empty to Android.
                outputMode = DataQueryOutputMode.SEMANTIC_JSON
            )
        )
        Log.i(
            MAPPING_QUERY_RECORD_TAG,
            "mapping.query.response action=$action ok=${queryResult.ok} " +
                "outputLength=${queryResult.outputText.length} message=${queryResult.message}"
        )
        if (!queryResult.ok) {
            return ActivityMappingNamesResult(
                ok = false,
                names = emptyList(),
                message = appendFailureContext(
                    message = "$failurePrefix: ${queryResult.message}",
                    operationId = queryResult.operationId
                ),
                operationId = queryResult.operationId
            )
        }

        val names = parseMappingNamesContent(queryResult.outputText).sorted()
        Log.i(
            MAPPING_QUERY_RECORD_TAG,
            "mapping.query.parsed action=$action count=${names.size} " +
                "containsW=${names.contains("w")} sample=${names.take(8).joinToString(",")}"
        )
        if (names.isEmpty() && !allowEmptyNames) {
            return ActivityMappingNamesResult(
                ok = false,
                names = emptyList(),
                message = appendFailureContext(
                    message = emptyNamesMessage,
                    operationId = queryResult.operationId
                ),
                operationId = queryResult.operationId
            )
        }

        return ActivityMappingNamesResult(
            ok = true,
            names = names,
            message = successMessageTemplate.format(names.size),
            operationId = queryResult.operationId
        )
    }
}
