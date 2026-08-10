package com.example.tracer

internal const val INSIGHTS_TARGET_NOT_FOUND = "insights.target.not_found"
internal fun QueryInsightsUiState.copyWithInsightsOutcome(
    period: DataTreePeriod,
    result: InsightsCallResult,
    textProvider: QueryInsightsTextProvider,
    dayTimeline: StructuredDailyInsights? = this.dayTimeline,
    statusValues: List<InsightsStatusValue> = emptyList()
): QueryInsightsUiState {
    val summary = buildInsightsSummary(period, result)
    val insights = if (result.operationOk) {
        QueryResult.Insights(
            text = result.outputText,
            summary = summary
        )
    } else {
        null
    }
    val nextInsightsResults = if (insights != null) {
        insightsResultsByPeriod + (period to insights)
    } else {
        insightsResultsByPeriod - period
    }
    val nextInsightsSummaries = if (summary != null) {
        insightsSummariesByPeriod + (period to summary)
    } else {
        insightsSummariesByPeriod - period
    }
    val nextInsightsErrors = when {
        result.operationOk -> insightsErrorsByPeriod - period
        summary is InsightsSummary.MissingTarget || summary is InsightsSummary.NoData ->
            insightsErrorsByPeriod - period
        else -> insightsErrorsByPeriod + (period to result.outputText)
    }
    return copy(
        insightsResultsByPeriod = nextInsightsResults,
        insightsSummariesByPeriod = nextInsightsSummaries,
        insightsErrorsByPeriod = nextInsightsErrors,
        dayTimeline = if (period == DataTreePeriod.DAY) dayTimeline else this.dayTimeline,
        statusValues = statusValues,
        dayInsightsNeedsRefresh = if (period == DataTreePeriod.DAY) false else dayInsightsNeedsRefresh,
        activeResult = insights,
        analysisError = "",
        statusText = resolveInsightsStatusText(
            period = period,
            result = result,
            textProvider = textProvider
        )
    )
}

internal fun buildInsightsSummary(
    period: DataTreePeriod,
    result: InsightsCallResult
): InsightsSummary? {
    val errorContract = result.errorContract
    if (!result.operationOk &&
        errorContract?.errorCode == INSIGHTS_TARGET_NOT_FOUND &&
        period == DataTreePeriod.DAY
    ) {
        return InsightsSummary.NoData(period = period)
    }
    if (!result.operationOk &&
        errorContract?.errorCode == INSIGHTS_TARGET_NOT_FOUND &&
        period.isNamedTargetPeriod()
    ) {
        return InsightsSummary.MissingTarget(
            period = period,
            errorCode = errorContract.errorCode,
            errorCategory = errorContract.errorCategory,
            hints = errorContract.hints
        )
    }
    if (result.operationOk &&
        period.isWindowedPeriod()
    ) {
        val metadata = result.insightsWindowMetadata ?: return null
        return InsightsSummary.WindowMetadata(
            period = period,
            metadata = metadata
        )
    }
    return null
}

internal fun resolveInsightsStatusText(
    period: DataTreePeriod,
    result: InsightsCallResult,
    textProvider: QueryInsightsTextProvider
): String {
    val mode = textProvider.periodLabel(period)
    return when {
        buildInsightsSummary(period, result) is InsightsSummary.NoData ->
            textProvider.nativeInsightsNoData(mode)
        buildInsightsSummary(period, result) is InsightsSummary.MissingTarget ->
            textProvider.nativeInsightsTargetMissing(mode)
        buildInsightsSummary(period, result) is InsightsSummary.WindowMetadata &&
            result.insightsWindowMetadata?.hasRecords == false ->
            textProvider.nativeInsightsEmptyWindow(mode)
        else -> textProvider.nativeInsightsResult(mode = mode, ok = result.operationOk)
    }
}

internal fun DataTreePeriod.isNamedTargetPeriod(): Boolean {
    return this == DataTreePeriod.DAY ||
        this == DataTreePeriod.WEEK ||
        this == DataTreePeriod.MONTH ||
        this == DataTreePeriod.YEAR
}

internal fun DataTreePeriod.isWindowedPeriod(): Boolean {
    return this == DataTreePeriod.RECENT || this == DataTreePeriod.RANGE
}

