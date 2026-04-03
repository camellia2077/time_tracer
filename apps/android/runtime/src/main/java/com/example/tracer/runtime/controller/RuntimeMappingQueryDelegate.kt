package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

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

    suspend fun listActivityAliasKeys(): ActivityMappingNamesResult =
        withContext(Dispatchers.IO) {
            try {
                queryActivityAliasKeysFromCore()
            } catch (error: Exception) {
                ActivityMappingNamesResult(
                    ok = false,
                    names = emptyList(),
                    message = formatNativeFailure("list activity alias keys failed", error)
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

    private fun queryActivityAliasKeysFromCore(): ActivityMappingNamesResult {
        return queryNamedMappingSet(
            action = NativeBridge.QUERY_ACTION_MAPPING_ALIAS_KEYS,
            failurePrefix = "mapping alias keys query failed",
            emptyNamesMessage = "mapping alias keys query failed: empty alias keys.",
            successMessageTemplate = "Loaded %d mapping alias keys."
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
        successMessageTemplate: String
    ): ActivityMappingNamesResult {
        val queryResult = runDataQuery(
            DataQueryRequest(
                action = action
            )
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
        if (names.isEmpty()) {
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

