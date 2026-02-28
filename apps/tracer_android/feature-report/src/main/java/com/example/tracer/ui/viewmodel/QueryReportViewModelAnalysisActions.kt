package com.example.tracer

internal suspend fun runStatsAnalysisAction(
    currentState: QueryReportUiState,
    period: DataTreePeriod,
    source: QueryPeriodSource,
    periodArgumentResolver: QueryPeriodArgumentResolver,
    textProvider: QueryReportTextProvider,
    queryGateway: QueryGateway
): QueryReportUiState {
    val resolved = periodArgumentResolver.resolveAndValidate(
        period = period,
        source = source,
        subjectLabel = textProvider.statsSubjectLabel()
    )
    if (resolved is QueryPeriodResolveResult.Failure) {
        return currentState.copy(
            analysisLoading = false,
            analysisError = resolved.message,
            activeResult = null,
            statsPeriod = period
        )
    }
    val normalizedArgument = (resolved as QueryPeriodResolveResult.Success).argument

    val runningState = currentState.copy(
        analysisLoading = true,
        analysisError = "",
        statsPeriod = period,
        statusText = textProvider.queryStatsRunning(textProvider.periodLabel(period))
    )

    val result = queryGateway.queryDayDurationStats(
        DataDurationQueryParams(
            period = period,
            periodArgument = normalizedArgument
        )
    )

    return runningState.copy(
        analysisLoading = false,
        analysisError = if (result.ok) "" else result.message,
        activeResult = if (result.ok) QueryResult.Stats(
            result.outputText,
            period
        ) else null,
        statsPeriod = period,
        statusText = textProvider.queryStatsResult(
            period = textProvider.periodLabel(period),
            ok = result.ok
        )
    )
}

internal suspend fun runTreeAnalysisAction(
    currentState: QueryReportUiState,
    period: DataTreePeriod,
    level: Int,
    source: QueryPeriodSource,
    periodArgumentResolver: QueryPeriodArgumentResolver,
    textProvider: QueryReportTextProvider,
    queryGateway: QueryGateway
): QueryReportUiState {
    val withTreePeriodState = currentState.copy(treePeriod = period)
    val resolved = periodArgumentResolver.resolveAndValidate(
        period = period,
        source = source,
        subjectLabel = textProvider.treeSubjectLabel()
    )
    if (resolved is QueryPeriodResolveResult.Failure) {
        return withTreePeriodState.copy(
            analysisLoading = false,
            analysisError = resolved.message,
            activeResult = null
        )
    }
    val normalizedArg = (resolved as QueryPeriodResolveResult.Success).argument
    if (level < -1) {
        return withTreePeriodState.copy(
            analysisLoading = false,
            analysisError = textProvider.treeLevelMustBeAtLeastMinusOne(),
            activeResult = null
        )
    }

    val runningState = withTreePeriodState.copy(
        analysisLoading = true,
        analysisError = "",
        statusText = textProvider.queryTreeRunning()
    )

    val result = queryGateway.queryProjectTree(
        DataTreeQueryParams(
            period = period,
            periodArgument = normalizedArg,
            level = level
        )
    )

    return runningState.copy(
        analysisLoading = false,
        analysisError = if (result.ok) "" else result.message,
        activeResult = if (result.ok) {
            QueryResult.Tree(
                period = period,
                nodes = result.nodes,
                found = result.found,
                roots = result.roots,
                message = result.message,
                fallbackText = result.legacyText,
                usesTextFallback = result.usesTextFallback
            )
        } else {
            null
        },
        statusText = textProvider.queryTreeResult(result.ok)
    )
}
