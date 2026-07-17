package com.example.tracer

import android.util.Log
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

internal class RuntimeQueryDelegate(
    private val queryTranslator: NativeQueryTranslator,
    private val executeNativeDataQuery: (
        request: DataQueryRequest,
        onRuntimePaths: ((RuntimePaths) -> Unit)?
    ) -> NativeCallResult,
    private val ensureConfigTomlStorage: () -> ConfigTomlStorage,
    private val diagnosticsRecorder: RuntimeDiagnosticsRecorder? = null,
    private val nextOperationId: ((String) -> String)? = null
) {
    private val dataDelegate = RuntimeDataQueryDelegate(
        queryTranslator = queryTranslator,
        executeNativeDataQuery = executeNativeDataQuery
    )
    private val mappingDelegate = RuntimeMappingQueryDelegate(
        runDataQuery = dataDelegate::runDataQuery
    )
    private val canonicalCatalogDelegate = RuntimeCanonicalCatalogQueryDelegate(
        ensureConfigTomlStorage = ensureConfigTomlStorage
    )

    suspend fun queryActivitySuggestions(
        lookbackDays: Int,
        topN: Int,
        anchorDateIso: String? = null
    ): ActivitySuggestionResult = withContext(Dispatchers.IO) {
        val validationFailure = validateSuggestionQueryParams(
            lookbackDays = lookbackDays,
            topN = topN
        )
        if (validationFailure != null) {
            recordActivitySuggestionDiagnostics(
                operationId = nextDiagnosticOperationId(),
                ok = false,
                lookbackDays = lookbackDays,
                topN = topN,
                anchorDateIso = anchorDateIso,
                message = validationFailure.message
            )
            return@withContext validationFailure
        }
        if (lookbackDays == 0 || topN == 0) {
            // `0` is a valid business configuration for suggestions: it means
            // "do not query any suggestion data" and also keeps the UI friendly
            // when users clear the numeric fields before typing replacement values.
            val message = if (lookbackDays == 0) {
                buildSuggestionResultMessage(emptyList(), lookbackDays)
            } else {
                "Suggestion query skipped because topN=0."
            }
            recordActivitySuggestionDiagnostics(
                operationId = nextDiagnosticOperationId(),
                ok = true,
                lookbackDays = lookbackDays,
                topN = topN,
                anchorDateIso = anchorDateIso,
                nativeOk = true,
                suggestions = emptyList(),
                message = message
            )
            return@withContext ActivitySuggestionResult(
                ok = true,
                suggestions = emptyList(),
                message = message
            )
        }

        try {
            val queryResult = executeNativeDataQuery(
                DataQueryRequest(
                    action = NativeBridge.QUERY_ACTION_ACTIVITY_SUGGEST,
                    topN = topN,
                    lookbackDays = lookbackDays,
                    anchorDateIso = anchorDateIso
                ),
                null
            )
            val contentResult = queryTranslator.toContentResult(
                queryResult = queryResult,
                defaultFailureMessage = "query activity suggestions failed."
            )
            val rawActivities = when (contentResult) {
                is DomainResult.Success -> parseSuggestedActivities(contentResult.value)
                is DomainResult.Failure -> {
                    recordActivitySuggestionDiagnostics(
                        operationId = contentResult.error.operationId.ifBlank {
                            queryResult.operationId
                        },
                        ok = false,
                        lookbackDays = lookbackDays,
                        topN = topN,
                        anchorDateIso = anchorDateIso,
                        nativeOk = false,
                        message = contentResult.error.legacyMessage()
                    )
                    return@withContext ActivitySuggestionResult(
                        ok = false,
                        suggestions = emptyList(),
                        message = contentResult.error.legacyMessage(),
                        operationId = contentResult.error.operationId
                    )
                }
            }
            val suggestions = normalizeSuggestedActivities(
                activities = rawActivities,
                validActivityNames = emptySet(),
                maxItems = topN
            )
            val authorableTokensResult = mappingDelegate.queryAuthorableEventTokensFromCore()
            recordActivitySuggestionDiagnostics(
                operationId = queryResult.operationId,
                ok = true,
                lookbackDays = lookbackDays,
                topN = topN,
                anchorDateIso = anchorDateIso,
                nativeOk = true,
                rawActivities = rawActivities,
                authorableOk = authorableTokensResult.ok,
                authorableNames = authorableTokensResult.names,
                suggestions = suggestions,
                message = buildSuggestionResultMessage(suggestions, lookbackDays)
            )

            ActivitySuggestionResult(
                ok = true,
                suggestions = suggestions,
                message = buildSuggestionResultMessage(suggestions, lookbackDays),
                operationId = queryResult.operationId
            )
        } catch (error: Exception) {
            ActivitySuggestionResult(
                ok = false,
                suggestions = emptyList(),
                message = formatNativeFailure("query activity suggestions failed", error)
            ).also {
                recordActivitySuggestionDiagnostics(
                    operationId = nextDiagnosticOperationId(),
                    ok = false,
                    lookbackDays = lookbackDays,
                    topN = topN,
                    anchorDateIso = anchorDateIso,
                    message = it.message
                )
            }
        }
    }

    suspend fun queryDayDurations(params: DataDurationQueryParams): DataQueryTextResult =
        dataDelegate.queryDayDurations(params)

    suspend fun queryDayDurationStats(params: DataDurationQueryParams): DataQueryTextResult =
        dataDelegate.queryDayDurationStats(params)

    suspend fun queryProjectTree(params: DataTreeQueryParams): TreeQueryResult =
        dataDelegate.queryProjectTree(params)

    suspend fun queryReportChart(params: ReportChartQueryParams): ReportChartQueryResult =
        dataDelegate.queryReportChart(params)

    suspend fun queryReportComposition(
        params: ReportCompositionQueryParams
    ): ReportCompositionQueryResult =
        dataDelegate.queryReportComposition(params)

    suspend fun listActivityMappingNames(): ActivityMappingNamesResult =
        mappingDelegate.listActivityMappingNames()

    suspend fun listActivityAliasMappings(): ActivityAliasMappingListResult =
        mappingDelegate.listActivityAliasMappings()

    suspend fun listCanonicalCatalog(): CanonicalCatalogResult =
        canonicalCatalogDelegate.listCanonicalCatalog()

    suspend fun listActivityAliasKeys(): ActivityMappingNamesResult =
        mappingDelegate.listActivityAliasKeys()

    suspend fun listWakeKeywords(): ActivityMappingNamesResult =
        mappingDelegate.listWakeKeywords()

    suspend fun listAuthorableEventTokens(): ActivityMappingNamesResult =
        mappingDelegate.listAuthorableEventTokens()

    private fun nextDiagnosticOperationId(): String =
        nextOperationId?.invoke("query_activity_suggestions") ?: ""

    private fun recordActivitySuggestionDiagnostics(
        operationId: String,
        ok: Boolean,
        lookbackDays: Int,
        topN: Int,
        anchorDateIso: String?,
        message: String,
        nativeOk: Boolean? = null,
        rawActivities: List<String> = emptyList(),
        authorableOk: Boolean? = null,
        authorableNames: List<String> = emptyList(),
        suggestions: List<String> = emptyList()
    ) {
        val diagnosticMessage = buildActivitySuggestionDiagnosticMessage(
            lookbackDays = lookbackDays,
            topN = topN,
            anchorDateIso = anchorDateIso,
            nativeOk = nativeOk,
            rawActivities = rawActivities,
            authorableOk = authorableOk,
            authorableNames = authorableNames,
            suggestions = suggestions,
            statusMessage = message
        )
        logActivitySuggestionDiagnostics(
            operationId = operationId,
            ok = ok,
            message = diagnosticMessage
        )
        diagnosticsRecorder?.record(
            RuntimeDiagnosticRecord(
                timestampEpochMs = System.currentTimeMillis(),
                operationId = operationId.ifBlank { "query_activity_suggestions" },
                stage = "query.activity_suggestions",
                ok = ok,
                initialized = null,
                message = diagnosticMessage
            )
        )
    }

    private fun logActivitySuggestionDiagnostics(
        operationId: String,
        ok: Boolean,
        message: String
    ) {
        try {
            Log.i(
                DIAGNOSTIC_LOG_TAG,
                "stage=query.activity_suggestions op=${operationId.ifBlank { "-" }} ok=$ok $message"
            )
        } catch (_: Throwable) {
            // Local JVM tests use the Android stub jar, where Log methods may be unimplemented.
        }
    }

    private fun buildActivitySuggestionDiagnosticMessage(
        lookbackDays: Int,
        topN: Int,
        anchorDateIso: String?,
        nativeOk: Boolean?,
        rawActivities: List<String>,
        authorableOk: Boolean?,
        authorableNames: List<String>,
        suggestions: List<String>,
        statusMessage: String
    ): String {
        val parts = mutableListOf(
            "lookbackDays=$lookbackDays",
            "topN=$topN",
            "anchorDateIso=${anchorDateIso?.takeIf { it.isNotBlank() } ?: "-"}"
        )
        nativeOk?.let { parts += "nativeOk=$it" }
        parts += "rawCount=${rawActivities.size}"
        authorableOk?.let { parts += "authorableOk=$it" }
        parts += "authorableCount=${authorableNames.size}"
        parts += "finalCount=${suggestions.size}"
        parts += "rawSample=${rawActivities.toDiagnosticSample()}"
        parts += "authorableSample=${authorableNames.toDiagnosticSample()}"
        parts += "finalSample=${suggestions.toDiagnosticSample()}"
        parts += "status=${statusMessage.replaceLineBreaks()}"
        return parts.joinToString(" ")
    }

    private fun List<String>.toDiagnosticSample(maxItems: Int = 5): String =
        take(maxItems)
            .joinToString(prefix = "[", postfix = "]", separator = ",") { item ->
                item.replaceLineBreaks()
            }

    private fun String.replaceLineBreaks(): String =
        replace('\n', ' ').replace('\r', ' ')

    private companion object {
        const val DIAGNOSTIC_LOG_TAG = "TimeTracerRuntime"
    }
}
