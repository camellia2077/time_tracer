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
            val mappingNamesResult = queryActivityMappingNamesFromCore()
            val validActivityNames = if (mappingNamesResult.ok) {
                mappingNamesResult.names.toSet()
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
        withContext(Dispatchers.IO) {
            runDataQuery(
                request = DataQueryRequest(
                    action = NativeBridge.QUERY_ACTION_DAYS_DURATION,
                    year = params.year,
                    month = params.month,
                    fromDateIso = params.fromDateIso,
                    toDateIso = params.toDateIso,
                    reverse = params.reverse,
                    limit = params.limit
                )
            )
        }

    suspend fun queryDayDurationStats(params: DataDurationQueryParams): DataQueryTextResult =
        withContext(Dispatchers.IO) {
            val period = params.period
            val normalizedPeriodArgument = if (period == null) {
                null
            } else {
                val periodValidation = validateAndNormalizePeriodArgument(
                    period = period,
                    periodArgument = params.periodArgument
                )
                if (periodValidation.error != null) {
                    return@withContext periodValidation.error
                }
                periodValidation.argument
            }
            runDataQuery(
                request = DataQueryRequest(
                    action = NativeBridge.QUERY_ACTION_DAYS_STATS,
                    year = params.year,
                    month = params.month,
                    fromDateIso = params.fromDateIso,
                    toDateIso = params.toDateIso,
                    topN = params.topN,
                    treePeriod = period?.wireValue,
                    treePeriodArgument = normalizedPeriodArgument
                )
            )
        }

    suspend fun queryProjectTree(params: DataTreeQueryParams): TreeQueryResult =
        withContext(Dispatchers.IO) {
            val periodValidation = validateAndNormalizePeriodArgument(
                period = params.period,
                periodArgument = params.periodArgument
            )
            if (periodValidation.error != null) {
                return@withContext TreeQueryResult(
                    ok = false,
                    found = false,
                    message = periodValidation.error.message,
                    operationId = periodValidation.error.operationId
                )
            }
            val normalizedParams = params.copy(periodArgument = periodValidation.argument)

            try {
                val structuredResult = queryTranslator.toTreeQueryResult(
                    executeNativeTreeQuery(normalizedParams)
                )
                if (structuredResult.ok) {
                    return@withContext structuredResult
                }

                val legacyResult = runLegacyTreeQuery(normalizedParams)
                if (!legacyResult.ok) {
                    return@withContext structuredResult
                }
                val operationId = structuredResult.operationId.ifBlank {
                    legacyResult.operationId
                }
                TreeQueryResult(
                    ok = true,
                    found = legacyResult.outputText.isNotBlank(),
                    roots = emptyList(),
                    nodes = emptyList(),
                    message = buildTreeResultMessage(
                        found = legacyResult.outputText.isNotBlank(),
                        roots = emptyList(),
                        nodes = emptyList(),
                        usesTextFallback = true
                    ),
                    operationId = operationId,
                    legacyText = legacyResult.outputText,
                    usesTextFallback = true
                )
            } catch (error: Exception) {
                TreeQueryResult(
                    ok = false,
                    found = false,
                    message = formatNativeFailure("query project tree failed", error)
                )
            }
        }

    suspend fun queryProjectTreeText(params: DataTreeQueryParams): DataQueryTextResult =
        withContext(Dispatchers.IO) {
            val periodValidation = validateAndNormalizePeriodArgument(
                period = params.period,
                periodArgument = params.periodArgument
            )
            if (periodValidation.error != null) {
                return@withContext periodValidation.error
            }
            runLegacyTreeQuery(params.copy(periodArgument = periodValidation.argument))
        }

    suspend fun queryReportChart(params: ReportChartQueryParams): ReportChartQueryResult =
        withContext(Dispatchers.IO) {
            val fromDateIso = params.fromDateIso?.trim()?.takeIf { it.isNotEmpty() }
            val toDateIso = params.toDateIso?.trim()?.takeIf { it.isNotEmpty() }
            val validationFailure = validateReportChartQueryParams(
                lookbackDays = params.lookbackDays,
                fromDateIso = fromDateIso,
                toDateIso = toDateIso
            )
            if (validationFailure != null) {
                return@withContext validationFailure
            }

            try {
                val root = params.root?.trim()?.takeIf { it.isNotEmpty() }
                val queryResult = runDataQuery(
                    request = DataQueryRequest(
                        action = NativeBridge.QUERY_ACTION_REPORT_CHART,
                        outputMode = DataQueryOutputMode.SEMANTIC_JSON,
                        root = root,
                        lookbackDays = params.lookbackDays,
                        fromDateIso = fromDateIso,
                        toDateIso = toDateIso
                    )
                )

                if (!queryResult.ok) {
                    return@withContext ReportChartQueryResult(
                        ok = false,
                        data = null,
                        message = queryResult.message,
                        operationId = queryResult.operationId
                    )
                }

                val parsed = parseReportChartContent(queryResult.outputText)
                    ?: return@withContext ReportChartQueryResult(
                        ok = false,
                        data = null,
                        message = appendFailureContext(
                            message = "report chart query returned invalid payload.",
                            operationId = queryResult.operationId
                        ),
                        operationId = queryResult.operationId
                    )

                ReportChartQueryResult(
                    ok = true,
                    data = parsed,
                    message = buildReportChartResultMessage(parsed.points.size),
                    operationId = queryResult.operationId
                )
            } catch (error: Exception) {
                ReportChartQueryResult(
                    ok = false,
                    data = null,
                    message = formatNativeFailure("query report chart failed", error)
                )
            }
        }

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

    private fun runDataQuery(request: DataQueryRequest): DataQueryTextResult {
        val queryResult = executeNativeDataQuery(request, null)
        return queryTranslator.toDataQueryTextResult(queryResult)
    }

    private fun runLegacyTreeQuery(params: DataTreeQueryParams): DataQueryTextResult {
        return runDataQuery(
            request = DataQueryRequest(
                action = NativeBridge.QUERY_ACTION_TREE,
                treePeriod = params.period.wireValue,
                treePeriodArgument = params.periodArgument,
                treeMaxDepth = params.level
            )
        )
    }

    private fun queryActivityMappingNamesFromCore(): ActivityMappingNamesResult {
        val queryResult = runDataQuery(
            request = DataQueryRequest(
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
}
