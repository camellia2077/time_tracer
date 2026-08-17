@file:Suppress("LongMethod")

package com.example.tracer

import android.util.Log
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

internal class RuntimeDataQueryDelegate(
    private val queryTranslator: NativeQueryTranslator,
    private val executeNativeDataQuery: (
        request: DataQueryRequest,
        onRuntimePaths: ((RuntimePaths) -> Unit)?
    ) -> NativeCallResult
) {
    private companion object {
        const val INSIGHTS_QUERY_LOG_TAG = "TracerInsightsChart"
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
                val structuredResult = runStructuredTreeQuery(normalizedParams)
                structuredResult
            } catch (error: Exception) {
                TreeQueryResult(
                    ok = false,
                    found = false,
                    message = formatNativeFailure("query project tree failed", error)
                )
            }
        }

    suspend fun queryInsightsCalendarAvailability(): InsightsCalendarAvailabilityResult =
        withContext(Dispatchers.IO) {
            try {
                val yearsResult = runDataQuery(
                    DataQueryRequest(
                        action = NativeBridge.QUERY_ACTION_YEARS,
                        outputMode = DataQueryOutputMode.SEMANTIC_JSON
                    )
                )
                if (!yearsResult.ok) {
                    return@withContext InsightsCalendarAvailabilityResult(
                        ok = false,
                        message = yearsResult.message,
                        operationId = yearsResult.operationId
                    )
                }
                val years = parseSemanticListContent(yearsResult.outputText, "years")
                    ?: return@withContext InsightsCalendarAvailabilityResult(
                        ok = false,
                        message = "insights years query returned invalid payload.",
                        operationId = yearsResult.operationId
                    )

                val monthsResult = runDataQuery(
                    DataQueryRequest(
                        action = NativeBridge.QUERY_ACTION_MONTHS,
                        outputMode = DataQueryOutputMode.SEMANTIC_JSON
                    )
                )
                if (!monthsResult.ok) {
                    return@withContext InsightsCalendarAvailabilityResult(
                        ok = false,
                        years = years,
                        message = monthsResult.message,
                        operationId = monthsResult.operationId
                    )
                }
                val months = parseSemanticListContent(monthsResult.outputText, "months")
                    ?: return@withContext InsightsCalendarAvailabilityResult(
                        ok = false,
                        years = years,
                        message = "insights months query returned invalid payload.",
                        operationId = monthsResult.operationId
                    )

                InsightsCalendarAvailabilityResult(
                    ok = true,
                    years = years,
                    months = months,
                    message = "insights calendar availability loaded.",
                    operationId = monthsResult.operationId
                )
            } catch (error: Exception) {
                InsightsCalendarAvailabilityResult(
                    ok = false,
                    message = formatNativeFailure(
                        "query insights calendar availability failed",
                        error
                    )
                )
            }
        }

    suspend fun queryInsightsChart(params: InsightsChartQueryParams): InsightsChartQueryResult =
        withContext(Dispatchers.IO) {
            val fromDateIso = params.fromDateIso?.trim()?.takeIf { it.isNotEmpty() }
            val toDateIso = params.toDateIso?.trim()?.takeIf { it.isNotEmpty() }
            Log.i(
                INSIGHTS_QUERY_LOG_TAG,
                "native trend request; root=${params.root.orEmpty()} " +
                    "lookbackDays=${params.lookbackDays} from=$fromDateIso to=$toDateIso"
            )
            val validationFailure = validateInsightsChartQueryParams(
                lookbackDays = params.lookbackDays,
                fromDateIso = fromDateIso,
                toDateIso = toDateIso
            )
            if (validationFailure != null) {
                Log.i(
                    INSIGHTS_QUERY_LOG_TAG,
                    "native trend validation failed; message=${validationFailure.message}"
                )
                return@withContext validationFailure
            }

            try {
                val root = params.root?.trim()?.takeIf { it.isNotEmpty() }
                val queryResult = runDataQuery(
                    request = DataQueryRequest(
                        action = NativeBridge.QUERY_ACTION_INSIGHTS_CHART,
                        outputMode = DataQueryOutputMode.SEMANTIC_JSON,
                        root = root,
                        lookbackDays = params.lookbackDays,
                        fromDateIso = fromDateIso,
                        toDateIso = toDateIso,
                        averageDayBasis = params.averageDayBasis.wireValue
                    )
                )

                if (!queryResult.ok) {
                    Log.i(
                        INSIGHTS_QUERY_LOG_TAG,
                        "native trend failed; operationId=${queryResult.operationId} " +
                            "message=${queryResult.message}"
                    )
                    return@withContext InsightsChartQueryResult(
                        ok = false,
                        data = null,
                        message = queryResult.message,
                        operationId = queryResult.operationId
                    )
                }

                val parsed = parseInsightsChartContent(queryResult.outputText)
                    ?: run {
                        Log.i(
                            INSIGHTS_QUERY_LOG_TAG,
                            "native trend payload invalid; operationId=${queryResult.operationId}"
                        )
                        return@withContext InsightsChartQueryResult(
                        ok = false,
                        data = null,
                        message = appendFailureContext(
                            message = "insights chart query returned invalid payload.",
                            operationId = queryResult.operationId
                        ),
                        operationId = queryResult.operationId
                        )
                    }

                Log.i(
                    INSIGHTS_QUERY_LOG_TAG,
                    "native trend succeeded; operationId=${queryResult.operationId} " +
                        "points=${parsed.points.size} roots=${parsed.roots.size} " +
                        "lookbackDays=${parsed.lookbackDays}"
                )

                InsightsChartQueryResult(
                    ok = true,
                    data = parsed,
                    message = buildInsightsChartResultMessage(parsed.points.size),
                    operationId = queryResult.operationId
                )
            } catch (error: Exception) {
                Log.i(
                    INSIGHTS_QUERY_LOG_TAG,
                    "native trend exception; type=${error::class.simpleName} message=${error.message}"
                )
                InsightsChartQueryResult(
                    ok = false,
                    data = null,
                    message = formatNativeFailure("query insights chart failed", error)
                )
            }
        }

    suspend fun queryInsightsComposition(
        params: InsightsCompositionQueryParams
    ): InsightsCompositionQueryResult = withContext(Dispatchers.IO) {
        val fromDateIso = params.fromDateIso?.trim()?.takeIf { it.isNotEmpty() }
        val toDateIso = params.toDateIso?.trim()?.takeIf { it.isNotEmpty() }
        Log.i(
            INSIGHTS_QUERY_LOG_TAG,
            "native composition request; lookbackDays=${params.lookbackDays} " +
                "from=$fromDateIso to=$toDateIso"
        )
        val validationFailure = validateInsightsCompositionQueryParams(
            lookbackDays = params.lookbackDays,
            fromDateIso = fromDateIso,
            toDateIso = toDateIso
        )
        if (validationFailure != null) {
            Log.i(
                INSIGHTS_QUERY_LOG_TAG,
                "native composition validation failed; message=${validationFailure.message}"
            )
            return@withContext validationFailure
        }

        try {
            val queryResult = runDataQuery(
                request = DataQueryRequest(
                    action = NativeBridge.QUERY_ACTION_INSIGHTS_COMPOSITION,
                    outputMode = DataQueryOutputMode.SEMANTIC_JSON,
                    lookbackDays = params.lookbackDays,
                    fromDateIso = fromDateIso,
                    toDateIso = toDateIso,
                    averageDayBasis = params.averageDayBasis.wireValue
                )
            )

            if (!queryResult.ok) {
                Log.i(
                    INSIGHTS_QUERY_LOG_TAG,
                    "native composition failed; operationId=${queryResult.operationId} " +
                        "message=${queryResult.message}"
                )
                return@withContext InsightsCompositionQueryResult(
                    ok = false,
                    data = null,
                    message = queryResult.message,
                    operationId = queryResult.operationId
                )
            }

            val parsed = parseInsightsCompositionContent(queryResult.outputText)
                ?: run {
                    Log.i(
                        INSIGHTS_QUERY_LOG_TAG,
                        "native composition payload invalid; operationId=${queryResult.operationId}"
                    )
                    return@withContext InsightsCompositionQueryResult(
                    ok = false,
                    data = null,
                    message = appendFailureContext(
                        message = "insights composition query returned invalid payload.",
                        operationId = queryResult.operationId
                    ),
                    operationId = queryResult.operationId
                    )
                }

            Log.i(
                INSIGHTS_QUERY_LOG_TAG,
                "native composition succeeded; operationId=${queryResult.operationId} " +
                    "items=${parsed.tree.size} roots=${parsed.activeRootCount}"
            )

            InsightsCompositionQueryResult(
                ok = true,
                data = parsed,
                message = buildInsightsCompositionResultMessage(parsed.tree.size),
                operationId = queryResult.operationId
            )
        } catch (error: Exception) {
            Log.i(
                INSIGHTS_QUERY_LOG_TAG,
                "native composition exception; type=${error::class.simpleName} message=${error.message}"
            )
            InsightsCompositionQueryResult(
                ok = false,
                data = null,
                message = formatNativeFailure("query insights composition failed", error)
            )
        }
    }

    fun runDataQuery(request: DataQueryRequest): DataQueryTextResult {
        val queryResult = executeNativeDataQuery(request, null)
        return queryTranslator.toDataQueryTextResult(queryResult)
    }

    private fun runStructuredTreeQuery(params: DataTreeQueryParams): TreeQueryResult {
        val result = runDataQuery(
            request = DataQueryRequest(
                action = NativeBridge.QUERY_ACTION_TREE,
                outputMode = DataQueryOutputMode.SEMANTIC_JSON,
                treePeriod = params.period.wireValue,
                treePeriodArgument = params.periodArgument,
                treeMaxDepth = params.level
            )
        )
        if (!result.ok) {
            return TreeQueryResult(
                ok = false,
                found = false,
                message = result.message,
                operationId = result.operationId
            )
        }
        val payload = parseTreeQueryContent(result.outputText)
            ?: return TreeQueryResult(
                ok = false,
                found = false,
                message = appendFailureContext(
                    message = "tree query returned invalid payload.",
                    operationId = result.operationId
                ),
                operationId = result.operationId
            )
        return TreeQueryResult(
            ok = payload.ok,
            found = payload.found,
            roots = payload.roots,
            nodes = payload.nodes,
            message = buildTreeResultMessage(
                found = payload.found,
                roots = payload.roots,
                nodes = payload.nodes
            ),
            operationId = result.operationId
        )
    }
}
