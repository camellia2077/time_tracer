@file:Suppress("LongMethod", "TooManyFunctions")

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

    suspend fun queryFrequentActivities(
        lookbackDays: Int,
        topN: Int,
        anchorDateIso: String? = null
    ): ActivityFrequentResult = withContext(Dispatchers.IO) {
        val validationFailure = validateFrequentQueryParams(
            lookbackDays = lookbackDays,
            topN = topN
        )
        if (validationFailure != null) {
            recordActivityFrequentDiagnostics(
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
            // `0` is a valid business configuration for frequent activities: it means
            // "do not query any frequent data" and also keeps the UI friendly
            // when users clear the numeric fields before typing replacement values.
            val message = if (lookbackDays == 0) {
                buildFrequentActivitiesResultMessage(emptyList(), lookbackDays)
            } else {
                "Frequent query skipped because topN=0."
            }
            recordActivityFrequentDiagnostics(
                operationId = nextDiagnosticOperationId(),
                ok = true,
                lookbackDays = lookbackDays,
                topN = topN,
                anchorDateIso = anchorDateIso,
                nativeOk = true,
                frequentActivities = emptyList(),
                message = message
            )
            return@withContext ActivityFrequentResult(
                ok = true,
                frequentActivities = emptyList(),
                message = message
            )
        }

        try {
            val queryResult = executeNativeDataQuery(
                DataQueryRequest(
                    action = NativeBridge.QUERY_ACTION_ACTIVITY_FREQUENT,
                    topN = topN,
                    lookbackDays = lookbackDays,
                    anchorDateIso = anchorDateIso
                ),
                null
            )
            val contentResult = queryTranslator.toContentResult(
                queryResult = queryResult,
                defaultFailureMessage = "query frequent activities failed."
            )
            val rawActivities = when (contentResult) {
                is DomainResult.Success -> parseFrequentActivities(contentResult.value)
                is DomainResult.Failure -> {
                    recordActivityFrequentDiagnostics(
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
                    return@withContext ActivityFrequentResult(
                        ok = false,
                        frequentActivities = emptyList(),
                        message = contentResult.error.legacyMessage(),
                        operationId = contentResult.error.operationId
                    )
                }
            }
            val frequentActivities = normalizeFrequentActivities(
                activities = rawActivities,
                validActivityNames = emptySet(),
                maxItems = topN
            )
            val authorableTokensResult = mappingDelegate.queryAuthorableEventTokensFromCore()
            recordActivityFrequentDiagnostics(
                operationId = queryResult.operationId,
                ok = true,
                lookbackDays = lookbackDays,
                topN = topN,
                anchorDateIso = anchorDateIso,
                nativeOk = true,
                rawActivities = rawActivities,
                authorableOk = authorableTokensResult.ok,
                authorableNames = authorableTokensResult.names,
                frequentActivities = frequentActivities,
                message = buildFrequentActivitiesResultMessage(frequentActivities, lookbackDays)
            )

            ActivityFrequentResult(
                ok = true,
                frequentActivities = frequentActivities,
                message = buildFrequentActivitiesResultMessage(frequentActivities, lookbackDays),
                operationId = queryResult.operationId
            )
        } catch (error: Exception) {
            ActivityFrequentResult(
                ok = false,
                frequentActivities = emptyList(),
                message = formatNativeFailure("query frequent activities failed", error)
            ).also {
                recordActivityFrequentDiagnostics(
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

    suspend fun queryPreviousActivityTail(
        targetDateIso: String
    ): PreviousActivityTailResult = withContext(Dispatchers.IO) {
        val query = runActivitySemanticQuery(
            targetDateIso = targetDateIso,
            action = NativeBridge.QUERY_ACTION_PREVIOUS_ACTIVITY_TAIL,
            queryName = "previous activity tail"
        )
        query.failureMessage?.let {
            return@withContext PreviousActivityTailResult(
                ok = false,
                found = false,
                message = it,
                operationId = query.operationId
            )
        }
        val parsed = parsePreviousActivityTailContent(query.outputText.orEmpty())
            ?: return@withContext PreviousActivityTailResult(
                ok = false,
                found = false,
                message = appendFailureContext(
                    message = "previous activity tail query returned invalid payload.",
                    operationId = query.operationId
                ),
                operationId = query.operationId
            )
        PreviousActivityTailResult(
            ok = true,
            found = parsed.found,
            tail = parsed.tail,
            message = if (!parsed.found) {
                "No previous activity tail found."
            } else {
                "Loaded previous activity tail."
            },
            operationId = query.operationId
        )
    }

    suspend fun queryLatestActivityRecord(
        targetDateIso: String
    ): LatestActivityRecordResult = withContext(Dispatchers.IO) {
        val query = runActivitySemanticQuery(
            targetDateIso = targetDateIso,
            action = NativeBridge.QUERY_ACTION_LATEST_ACTIVITY_RECORD,
            queryName = "latest activity record"
        )
        query.failureMessage?.let {
            return@withContext LatestActivityRecordResult(
                ok = false,
                found = false,
                message = it,
                operationId = query.operationId
            )
        }
        val parsed = parseLatestActivityRecordContent(query.outputText.orEmpty())
            ?: return@withContext LatestActivityRecordResult(
                ok = false,
                found = false,
                message = appendFailureContext(
                    message = "latest activity record query returned invalid payload.",
                    operationId = query.operationId
                ),
                operationId = query.operationId
            )
        LatestActivityRecordResult(
            ok = true,
            found = parsed.found,
            record = parsed.record,
            message = if (parsed.found) {
                "Loaded latest activity record."
            } else {
                "No latest activity record found."
            },
            operationId = query.operationId
        )
    }

    private data class ActivitySemanticQuery(
        val outputText: String? = null,
        val operationId: String = "",
        val failureMessage: String? = null
    )

    private fun runActivitySemanticQuery(
        targetDateIso: String,
        action: Int,
        queryName: String
    ): ActivitySemanticQuery {
        val normalizedDate = targetDateIso.trim()
        if (normalizedDate.isEmpty()) {
            return ActivitySemanticQuery(
                failureMessage = "$queryName query requires a target date."
            )
        }

        return try {
            val queryResult = dataDelegate.runDataQuery(
                DataQueryRequest(
                    action = action,
                    outputMode = DataQueryOutputMode.SEMANTIC_JSON,
                    fromDateIso = normalizedDate
                )
            )
            if (!queryResult.ok) {
                ActivitySemanticQuery(
                    operationId = queryResult.operationId,
                    failureMessage = appendFailureContext(
                        message = "$queryName query failed: ${queryResult.message}",
                        operationId = queryResult.operationId
                    )
                )
            } else {
                ActivitySemanticQuery(
                    outputText = queryResult.outputText,
                    operationId = queryResult.operationId
                )
            }
        } catch (error: Exception) {
            ActivitySemanticQuery(
                failureMessage = formatNativeFailure("query $queryName failed", error)
            )
        }
    }

    suspend fun queryDayDurations(params: DataDurationQueryParams): DataQueryTextResult =
        dataDelegate.queryDayDurations(params)

    suspend fun queryDayDurationStats(params: DataDurationQueryParams): DataQueryTextResult =
        dataDelegate.queryDayDurationStats(params)

    suspend fun queryProjectTree(params: DataTreeQueryParams): TreeQueryResult =
        dataDelegate.queryProjectTree(params)

    suspend fun queryInsightsCalendarAvailability(): InsightsCalendarAvailabilityResult =
        dataDelegate.queryInsightsCalendarAvailability()

    suspend fun queryInsightsChart(params: InsightsChartQueryParams): InsightsChartQueryResult =
        dataDelegate.queryInsightsChart(params)

    suspend fun queryInsightsComposition(
        params: InsightsCompositionQueryParams
    ): InsightsCompositionQueryResult =
        dataDelegate.queryInsightsComposition(params)

    suspend fun listActivityMappingNames(): ActivityMappingNamesResult =
        mappingDelegate.listActivityMappingNames()

    suspend fun listActivityHierarchyLeafMappings(): ActivityHierarchyLeafMappingListResult =
        mappingDelegate.listActivityHierarchyLeafMappings()

    suspend fun listCanonicalCatalog(): CanonicalCatalogResult =
        canonicalCatalogDelegate.listCanonicalCatalog()

    suspend fun listActivityHierarchyLeafKeys(): ActivityMappingNamesResult =
        mappingDelegate.listActivityHierarchyLeafKeys()

    suspend fun listWakeKeywords(): ActivityMappingNamesResult =
        mappingDelegate.listWakeKeywords()

    suspend fun listAuthorableEventTokens(): ActivityMappingNamesResult =
        mappingDelegate.listAuthorableEventTokens()

    private fun nextDiagnosticOperationId(): String =
        nextOperationId?.invoke("query_activity_frequent") ?: ""

    private fun recordActivityFrequentDiagnostics(
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
        frequentActivities: List<String> = emptyList()
    ) {
        val diagnosticMessage = buildActivityFrequentDiagnosticMessage(
            lookbackDays = lookbackDays,
            topN = topN,
            anchorDateIso = anchorDateIso,
            nativeOk = nativeOk,
            rawActivities = rawActivities,
            authorableOk = authorableOk,
            authorableNames = authorableNames,
            frequentActivities = frequentActivities,
            statusMessage = message
        )
        logActivityFrequentDiagnostics(
            operationId = operationId,
            ok = ok,
            message = diagnosticMessage
        )
        diagnosticsRecorder?.record(
            RuntimeDiagnosticRecord(
                timestampEpochMs = System.currentTimeMillis(),
                operationId = operationId.ifBlank { "query_activity_frequent" },
                stage = "query.activity_frequent",
                ok = ok,
                initialized = null,
                message = diagnosticMessage
            )
        )
    }

    private fun logActivityFrequentDiagnostics(
        operationId: String,
        ok: Boolean,
        message: String
    ) {
        try {
            Log.i(
                DIAGNOSTIC_LOG_TAG,
                "stage=query.activity_frequent op=${operationId.ifBlank { "-" }} ok=$ok $message"
            )
        } catch (_: Throwable) {
            // Local JVM tests use the Android stub jar, where Log methods may be unimplemented.
        }
    }

    private fun buildActivityFrequentDiagnosticMessage(
        lookbackDays: Int,
        topN: Int,
        anchorDateIso: String?,
        nativeOk: Boolean?,
        rawActivities: List<String>,
        authorableOk: Boolean?,
        authorableNames: List<String>,
        frequentActivities: List<String>,
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
        parts += "finalCount=${frequentActivities.size}"
        parts += "rawSample=${rawActivities.toDiagnosticSample()}"
        parts += "authorableSample=${authorableNames.toDiagnosticSample()}"
        parts += "finalSample=${frequentActivities.toDiagnosticSample()}"
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
