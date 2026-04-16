package com.example.tracer

private data class ChartQueryCacheKey(
    val root: String,
    val reportMode: ReportMode,
    val lookbackDays: Int,
    val fromDateIso: String,
    val toDateIso: String
)

internal class QueryReportChartUseCase(
    private val queryGateway: QueryGateway,
    private val inputValidator: QueryInputValidator,
    private val textProvider: QueryReportTextProvider,
    private val nowMs: () -> Long = { System.currentTimeMillis() }
) {
    private val cache = LinkedHashMap<ChartQueryCacheKey, ChartRenderModel>()
    private val maxCacheEntries = 24
    private var operationCounter = 0L
    private val paramResolver = QueryReportChartParamResolver(inputValidator, textProvider)

    suspend fun execute(
        currentState: QueryReportUiState,
        emit: (QueryReportUiState) -> Unit
    ): QueryReportUiState {
        val params = paramResolver.resolve(currentState)
        if (params.validationError.isNotBlank()) {
            return currentState.copy(
                trendChartLoading = false,
                trendChartError = params.validationError,
                resultDisplayMode = ReportResultDisplayMode.CHART,
                statusText = params.validationError
            )
        }

        val requestedRoot = currentState.trendChartSelectedRoot.trim().ifEmpty { "" }
        val cacheKey = ChartQueryCacheKey(
            root = requestedRoot,
            reportMode = params.reportMode,
            lookbackDays = params.lookbackDays,
            fromDateIso = params.fromDateIso.orEmpty(),
            toDateIso = params.toDateIso.orEmpty()
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
            resultDisplayMode = ReportResultDisplayMode.CHART,
            statusText = textProvider.queryChartRunning()
        )
        emit(runningState)
        val startedAt = nowMs()

        val queryResult = queryGateway.queryReportChart(
            ReportChartQueryParams(
                root = requestedRoot.ifBlank { null },
                lookbackDays = params.lookbackDays,
                fromDateIso = params.fromDateIso,
                toDateIso = params.toDateIso
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
            selectedRootOverride = requestedRoot
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
        baseState: QueryReportUiState,
        params: ResolvedChartQueryParams,
        renderModel: ChartRenderModel,
        trace: ChartQueryTrace
    ): QueryReportUiState {
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
            resultDisplayMode = ReportResultDisplayMode.CHART,
            statusText = "${textProvider.queryChartResult(ok = true)} $statusSuffix"
        )
    }

    private fun nextOperationId(): String {
        operationCounter += 1
        return "chart-${nowMs()}-${operationCounter.toString().padStart(4, '0')}"
    }

    private fun computeParameterHash(key: ChartQueryCacheKey): String {
        val raw = "${key.root}|${key.reportMode}|${key.lookbackDays}|" +
            "${key.fromDateIso}|${key.toDateIso}"
        return raw.hashCode().toUInt().toString(16).padStart(8, '0')
    }
}

private data class CompositionQueryCacheKey(
    val reportMode: ReportMode,
    val lookbackDays: Int,
    val fromDateIso: String,
    val toDateIso: String
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
            toDateIso = params.toDateIso.orEmpty()
        )
        val operationId = nextOperationId()
        val parameterHash = computeParameterHash(cacheKey)
        val cached = cache[cacheKey]
        if (cached != null) {
            val trace = ChartQueryTrace(
                operationId = operationId,
                parameterHash = parameterHash,
                durationMs = 0L,
                pointCount = cached.slices.size,
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
                toDateIso = params.toDateIso
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
            pointCount = renderModel.slices.size,
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
        val raw = "${key.reportMode}|${key.lookbackDays}|${key.fromDateIso}|${key.toDateIso}"
        return raw.hashCode().toUInt().toString(16).padStart(8, '0')
    }
}
