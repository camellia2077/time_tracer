package com.example.tracer


private data class ChartQueryCacheKey(
    val root: String,
    val insightsMode: InsightsMode,
    val lookbackDays: Int,
    val fromDateIso: String,
    val toDateIso: String,
    val averageDayBasis: InsightsAverageDayBasis
)

internal class QueryInsightsChartUseCase(
    private val queryGateway: QueryGateway,
    private val inputValidator: QueryInputValidator,
    private val textProvider: QueryInsightsTextProvider,
    private val nowMs: () -> Long = { System.currentTimeMillis() }
) {
    private val cache = LinkedHashMap<ChartQueryCacheKey, ChartRenderModel>()
    private val maxCacheEntries = 24
    private var operationCounter = 0L
    private val paramResolver = QueryInsightsChartParamResolver(inputValidator, textProvider)

    fun invalidateCache() {
        cache.clear()
    }

    suspend fun executeComparison(
        comparison: ComparisonPeriodRequest,
        selectedRoot: String,
        averageDayBasis: InsightsAverageDayBasis
    ): ChartComparisonResult {
        val params = comparison.request.toChartQueryParams(
            root = selectedRoot.trim().ifEmpty { null },
            averageDayBasis = averageDayBasis
        ) ?: return ChartComparisonResult(
            renderModel = null,
            errorMessage = "Comparison period is invalid."
        )
        val queryResult = queryGateway.queryInsightsChart(params)
        val payload = queryResult.data
        if (!queryResult.ok || payload == null) {
            return ChartComparisonResult(
                renderModel = null,
                errorMessage = queryResult.message.ifBlank {
                    textProvider.chartPayloadInvalid()
                }
            )
        }
        val domainModel = mapCorePayloadToDomainModel(payload)
        return ChartComparisonResult(
            renderModel = mapDomainModelToRenderModel(
                model = domainModel,
                selectedRootOverride = params.root.orEmpty(),
                fromDateIso = params.fromDateIso,
                toDateIso = params.toDateIso
            )
        )
    }

    suspend fun execute(
        currentState: QueryInsightsUiState,
        emit: (QueryInsightsUiState) -> Unit
    ): QueryInsightsUiState {
        val params = paramResolver.resolve(currentState)
        if (params.validationError.isNotBlank()) {
            return currentState.copy(
                trendChartLoading = false,
                trendChartError = params.validationError,
                resultDisplayMode = InsightsResultDisplayMode.CHART,
                statusText = params.validationError
            )
        }

        val requestedRoot = currentState.trendChartSelectedRoot.trim().ifEmpty { "" }
        val cacheKey = ChartQueryCacheKey(
            root = requestedRoot,
            insightsMode = params.insightsMode,
            lookbackDays = params.lookbackDays,
            fromDateIso = params.fromDateIso.orEmpty(),
            toDateIso = params.toDateIso.orEmpty(),
            averageDayBasis = currentState.averageDayBasis
        )
        val operationId = nextOperationId()
        val parameterHash = computeParameterHash(cacheKey)

        val cached = cache[cacheKey]
        if (cached != null) {
            val trace = ChartQueryTrace(
                operationId = operationId,
                parameterHash = parameterHash,
                durationMs = 0L,
                pointCount = cached.points.size,
                rootCount = cached.roots.size,
                cacheHit = true
            )
            return buildSuccessState(
                baseState = currentState,
                params = params,
                renderModel = cached,
                trace = trace
            )
        }

        val runningState = currentState.copy(
            trendChartLoading = true,
            trendChartError = "",
            resultDisplayMode = InsightsResultDisplayMode.CHART,
            statusText = textProvider.queryChartRunning()
        )
        emit(runningState)
        val startedAt = nowMs()

        val queryResult = queryGateway.queryInsightsChart(
            InsightsChartQueryParams(
                root = requestedRoot.ifBlank { null },
                lookbackDays = params.lookbackDays,
                fromDateIso = params.fromDateIso,
                toDateIso = params.toDateIso,
                averageDayBasis = currentState.averageDayBasis
            )
        )
        val elapsedMs = (nowMs() - startedAt).coerceAtLeast(0L)

        val payload = queryResult.data
        if (!queryResult.ok || payload == null) {
            val trace = ChartQueryTrace(
                operationId = operationId,
                parameterHash = parameterHash,
                durationMs = elapsedMs,
                pointCount = 0,
                rootCount = 0,
                cacheHit = false
            )
            val errorMessage = queryResult.message.ifBlank { textProvider.chartPayloadInvalid() }
            return runningState.copy(
                trendChartLoading = false,
                trendChartError = errorMessage,
                trendChartLastTrace = trace,
                statusText = "${textProvider.queryChartResult(ok = false)} " +
                    "[op=${trace.operationId}, hash=${trace.parameterHash}, ms=${trace.durationMs}]"
            )
        }

        val domainModel = mapCorePayloadToDomainModel(payload)
        val renderModel = mapDomainModelToRenderModel(
            model = domainModel,
            selectedRootOverride = requestedRoot,
            fromDateIso = params.fromDateIso,
            toDateIso = params.toDateIso
        )
        putCache(cacheKey, renderModel)

        val trace = ChartQueryTrace(
            operationId = operationId,
            parameterHash = parameterHash,
            durationMs = elapsedMs,
            pointCount = renderModel.points.size,
            rootCount = renderModel.roots.size,
            cacheHit = false
        )
        return buildSuccessState(
            baseState = runningState,
            params = params,
            renderModel = renderModel,
            trace = trace
        )
    }

    private fun putCache(key: ChartQueryCacheKey, model: ChartRenderModel) {
        if (cache.containsKey(key)) {
            cache.remove(key)
        }
        cache[key] = model
        while (cache.size > maxCacheEntries) {
            val firstKey = cache.entries.firstOrNull()?.key ?: break
            cache.remove(firstKey)
        }
    }

    private fun buildSuccessState(
        baseState: QueryInsightsUiState,
        params: ResolvedChartQueryParams,
        renderModel: ChartRenderModel,
        trace: ChartQueryTrace
    ): QueryInsightsUiState {
        val statusSuffix = "[op=${trace.operationId}, hash=${trace.parameterHash}, " +
            "cache=${trace.cacheHit}, ms=${trace.durationMs}, points=${trace.pointCount}]"
        return baseState.copy(
            trendChartLoading = false,
            trendChartError = "",
            trendChartRenderModel = renderModel,
            trendChartLastTrace = trace,
            trendChartRoots = renderModel.roots,
            trendChartSelectedRoot = renderModel.selectedRoot,
            trendChartPoints = renderModel.points,
            trendChartAverageDurationSeconds = renderModel.averageDurationSeconds,
            trendChartTotalDurationSeconds = renderModel.totalDurationSeconds,
            trendChartActiveDays = renderModel.activeDays,
            trendChartRangeDays = renderModel.rangeDays,
            resultDisplayMode = InsightsResultDisplayMode.CHART,
            statusText = "${textProvider.queryChartResult(ok = true)} $statusSuffix"
        )
    }

    private fun nextOperationId(): String {
        operationCounter += 1
        return "chart-${nowMs()}-${operationCounter.toString().padStart(4, '0')}"
    }

    private fun computeParameterHash(key: ChartQueryCacheKey): String {
        val raw = "${key.root}|${key.insightsMode}|${key.lookbackDays}|" +
            "${key.fromDateIso}|${key.toDateIso}|${key.averageDayBasis}"
        return raw.hashCode().toUInt().toString(16).padStart(8, '0')
    }
}

private fun TemporalInsightsQueryRequest.toChartQueryParams(
    root: String?,
    averageDayBasis: InsightsAverageDayBasis
): InsightsChartQueryParams? = when (val value = selection) {
    is TemporalSelectionPayload -> when (value.kind) {
        TemporalSelectionKind.SINGLE_DAY -> value.date?.let { date ->
            InsightsChartQueryParams(
                root = root,
                lookbackDays = 1,
                fromDateIso = date,
                toDateIso = date,
                averageDayBasis = averageDayBasis
            )
        }
        TemporalSelectionKind.DATE_RANGE -> {
            val start = value.startDate
            val end = value.endDate
            if (start == null || end == null) null else {
                val days = runCatching {
                    java.time.temporal.ChronoUnit.DAYS.between(
                        java.time.LocalDate.parse(start),
                        java.time.LocalDate.parse(end)
                    ).toInt() + 1
                }.getOrNull()?.takeIf { it > 0 } ?: return null
                InsightsChartQueryParams(
                    root = root,
                    lookbackDays = days,
                    fromDateIso = start,
                    toDateIso = end,
                    averageDayBasis = averageDayBasis
                )
            }
        }
        TemporalSelectionKind.RECENT_DAYS -> {
            val days = value.days?.takeIf { it > 0 }
            val anchor = value.anchorDate?.let {
                runCatching { java.time.LocalDate.parse(it) }.getOrNull()
            }
            if (days == null || anchor == null) {
                null
            } else {
                val start = anchor.minusDays(days.toLong() - 1)
                InsightsChartQueryParams(
                    root = root,
                    lookbackDays = days,
                    fromDateIso = start.toString(),
                    toDateIso = anchor.toString(),
                    averageDayBasis = averageDayBasis
                )
            }
        }
    }
}

