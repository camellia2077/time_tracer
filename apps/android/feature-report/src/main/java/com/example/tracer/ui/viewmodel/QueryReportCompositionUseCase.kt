package com.example.tracer

private data class CompositionQueryCacheKey(
    val reportMode: ReportMode,
    val lookbackDays: Int,
    val fromDateIso: String,
    val toDateIso: String,
    val averageDayBasis: ReportAverageDayBasis
)

internal class QueryReportCompositionUseCase(
    private val queryGateway: QueryGateway,
    private val inputValidator: QueryInputValidator,
    private val textProvider: QueryReportTextProvider,
    private val nowMs: () -> Long = { System.currentTimeMillis() }
) {
    private val cache = LinkedHashMap<CompositionQueryCacheKey, CompositionChartRenderModel>()
    private val maxCacheEntries = 24
    private var operationCounter = 0L
    private val paramResolver = QueryReportChartParamResolver(inputValidator, textProvider)

    fun invalidateCache() {
        cache.clear()
    }

    suspend fun execute(
        currentState: QueryReportUiState,
        emit: (QueryReportUiState) -> Unit
    ): QueryReportUiState {
        val params = paramResolver.resolve(currentState)
        if (params.validationError.isNotBlank()) {
            return currentState.copy(
                compositionChartLoading = false,
                compositionChartError = params.validationError,
                resultDisplayMode = ReportResultDisplayMode.CHART,
                statusText = params.validationError
            )
        }

        val cacheKey = CompositionQueryCacheKey(
            reportMode = params.reportMode,
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
                pointCount = cached.tree.size,
                rootCount = cached.activeRootCount,
                cacheHit = true
            )
            return buildSuccessState(
                baseState = currentState,
                renderModel = cached,
                trace = trace
            )
        }

        val runningState = currentState.copy(
            compositionChartLoading = true,
            compositionChartError = "",
            resultDisplayMode = ReportResultDisplayMode.CHART,
            statusText = textProvider.queryCompositionRunning()
        )
        emit(runningState)
        val startedAt = nowMs()

        val queryResult = queryGateway.queryReportComposition(
            ReportCompositionQueryParams(
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
            val errorMessage = queryResult.message.ifBlank {
                textProvider.compositionPayloadInvalid()
            }
            return runningState.copy(
                compositionChartLoading = false,
                compositionChartError = errorMessage,
                compositionChartLastTrace = trace,
                statusText = "${textProvider.queryCompositionResult(ok = false)} " +
                    "[op=${trace.operationId}, hash=${trace.parameterHash}, ms=${trace.durationMs}]"
            )
        }

        val renderModel = mapCorePayloadToCompositionRenderModel(payload)
        putCache(cacheKey, renderModel)
        val trace = ChartQueryTrace(
            operationId = operationId,
            parameterHash = parameterHash,
            durationMs = elapsedMs,
            pointCount = renderModel.tree.size,
            rootCount = renderModel.activeRootCount,
            cacheHit = false
        )
        return buildSuccessState(
            baseState = runningState,
            renderModel = renderModel,
            trace = trace
        )
    }

    private fun putCache(
        key: CompositionQueryCacheKey,
        model: CompositionChartRenderModel
    ) {
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
        baseState: QueryReportUiState,
        renderModel: CompositionChartRenderModel,
        trace: ChartQueryTrace
    ): QueryReportUiState {
        val statusSuffix = "[op=${trace.operationId}, hash=${trace.parameterHash}, " +
            "cache=${trace.cacheHit}, ms=${trace.durationMs}, items=${trace.pointCount}]"
        return baseState.copy(
            compositionChartLoading = false,
            compositionChartError = "",
            compositionChartRenderModel = renderModel,
            compositionChartLastTrace = trace,
            resultDisplayMode = ReportResultDisplayMode.CHART,
            statusText = "${textProvider.queryCompositionResult(ok = true)} $statusSuffix"
        )
    }

    private fun nextOperationId(): String {
        operationCounter += 1
        return "composition-${nowMs()}-${operationCounter.toString().padStart(4, '0')}"
    }

    private fun computeParameterHash(key: CompositionQueryCacheKey): String {
        val raw = "${key.reportMode}|${key.lookbackDays}|${key.fromDateIso}|" +
            "${key.toDateIso}|${key.averageDayBasis}"
        return raw.hashCode().toUInt().toString(16).padStart(8, '0')
    }
}

