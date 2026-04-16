package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

internal class RuntimeQueryDelegate(
    private val queryTranslator: NativeQueryTranslator,
    private val executeNativeTreeQuery: (DataTreeQueryParams) -> NativeCallResult,
    private val executeNativeDataQuery: (
        request: DataQueryRequest,
        onRuntimePaths: ((RuntimePaths) -> Unit)?
    ) -> NativeCallResult
) {
    private val dataDelegate = RuntimeDataQueryDelegate(
        queryTranslator = queryTranslator,
        executeNativeTreeQuery = executeNativeTreeQuery,
        executeNativeDataQuery = executeNativeDataQuery
    )
    private val mappingDelegate = RuntimeMappingQueryDelegate(
        runDataQuery = dataDelegate::runDataQuery
    )

    suspend fun queryActivitySuggestions(
        lookbackDays: Int,
        topN: Int
    ): ActivitySuggestionResult = withContext(Dispatchers.IO) {
        val validationFailure = validateSuggestionQueryParams(
            lookbackDays = lookbackDays,
            topN = topN
        )
        if (validationFailure != null) {
            return@withContext validationFailure
        }

        try {
            val queryResult = executeNativeDataQuery(
                DataQueryRequest(
                    action = NativeBridge.QUERY_ACTION_ACTIVITY_SUGGEST,
                    topN = topN,
                    lookbackDays = lookbackDays
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
                    return@withContext ActivitySuggestionResult(
                        ok = false,
                        suggestions = emptyList(),
                        message = contentResult.error.legacyMessage(),
                        operationId = contentResult.error.operationId
                    )
                }
            }
            val authorableTokensResult = mappingDelegate.queryAuthorableEventTokensFromCore()
            val validActivityNames = if (authorableTokensResult.ok) {
                authorableTokensResult.names.toSet()
            } else {
                emptySet()
            }
            val suggestions = normalizeSuggestedActivities(
                activities = rawActivities,
                validActivityNames = validActivityNames,
                maxItems = topN
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
            )
        }
    }

    suspend fun queryDayDurations(params: DataDurationQueryParams): DataQueryTextResult =
        dataDelegate.queryDayDurations(params)

    suspend fun queryDayDurationStats(params: DataDurationQueryParams): DataQueryTextResult =
        dataDelegate.queryDayDurationStats(params)

    suspend fun queryProjectTree(params: DataTreeQueryParams): TreeQueryResult =
        dataDelegate.queryProjectTree(params)

    suspend fun queryProjectTreeText(params: DataTreeQueryParams): DataQueryTextResult =
        dataDelegate.queryProjectTreeText(params)

    suspend fun queryReportChart(params: ReportChartQueryParams): ReportChartQueryResult =
        dataDelegate.queryReportChart(params)

    suspend fun queryReportComposition(
        params: ReportCompositionQueryParams
    ): ReportCompositionQueryResult =
        dataDelegate.queryReportComposition(params)

    suspend fun listActivityMappingNames(): ActivityMappingNamesResult =
        mappingDelegate.listActivityMappingNames()

    suspend fun listActivityAliasKeys(): ActivityMappingNamesResult =
        mappingDelegate.listActivityAliasKeys()

    suspend fun listWakeKeywords(): ActivityMappingNamesResult =
        mappingDelegate.listWakeKeywords()

    suspend fun listAuthorableEventTokens(): ActivityMappingNamesResult =
        mappingDelegate.listAuthorableEventTokens()
}
