package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

internal class RuntimeDataQueryDelegate(
    private val queryTranslator: NativeQueryTranslator,
    private val executeNativeTreeQuery: (DataTreeQueryParams) -> NativeCallResult,
    private val executeNativeDataQuery: (
        request: DataQueryRequest,
        onRuntimePaths: ((RuntimePaths) -> Unit)?
    ) -> NativeCallResult
) {
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

    suspend fun queryReportComposition(
        params: ReportCompositionQueryParams
    ): ReportCompositionQueryResult = withContext(Dispatchers.IO) {
        val fromDateIso = params.fromDateIso?.trim()?.takeIf { it.isNotEmpty() }
        val toDateIso = params.toDateIso?.trim()?.takeIf { it.isNotEmpty() }
        val validationFailure = validateReportCompositionQueryParams(
            lookbackDays = params.lookbackDays,
            fromDateIso = fromDateIso,
            toDateIso = toDateIso
        )
        if (validationFailure != null) {
            return@withContext validationFailure
        }

        try {
            val queryResult = runDataQuery(
                request = DataQueryRequest(
                    action = NativeBridge.QUERY_ACTION_REPORT_COMPOSITION,
                    outputMode = DataQueryOutputMode.SEMANTIC_JSON,
                    lookbackDays = params.lookbackDays,
                    fromDateIso = fromDateIso,
                    toDateIso = toDateIso
                )
            )

            if (!queryResult.ok) {
                return@withContext ReportCompositionQueryResult(
                    ok = false,
                    data = null,
                    message = queryResult.message,
                    operationId = queryResult.operationId
                )
            }

            val parsed = parseReportCompositionContent(queryResult.outputText)
                ?: return@withContext ReportCompositionQueryResult(
                    ok = false,
                    data = null,
                    message = appendFailureContext(
                        message = "report composition query returned invalid payload.",
                        operationId = queryResult.operationId
                    ),
                    operationId = queryResult.operationId
                )

            ReportCompositionQueryResult(
                ok = true,
                data = parsed,
                message = buildReportCompositionResultMessage(parsed.slices.size),
                operationId = queryResult.operationId
            )
        } catch (error: Exception) {
            ReportCompositionQueryResult(
                ok = false,
                data = null,
                message = formatNativeFailure("query report composition failed", error)
            )
        }
    }

    fun runDataQuery(request: DataQueryRequest): DataQueryTextResult {
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
}
