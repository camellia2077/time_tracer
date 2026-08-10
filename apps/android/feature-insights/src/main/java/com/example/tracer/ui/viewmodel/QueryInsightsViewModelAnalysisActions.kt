package com.example.tracer

internal suspend fun runTreeAnalysisAction(
    currentState: QueryInsightsUiState,
    period: DataTreePeriod,
    level: Int,
    source: QueryPeriodSource,
    periodArgumentResolver: QueryPeriodArgumentResolver,
    textProvider: QueryInsightsTextProvider,
    queryGateway: QueryGateway
): QueryInsightsUiState {
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
                maxAvailableDepth = result.maxAvailableDepth,
                message = result.message
            )
        } else {
            null
        },
        statusText = textProvider.queryTreeResult(result.ok)
    )
}
