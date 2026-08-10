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
            trendChartUsesLegacyStatsFallback = renderModel.usesLegacyStatsFallback,
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

