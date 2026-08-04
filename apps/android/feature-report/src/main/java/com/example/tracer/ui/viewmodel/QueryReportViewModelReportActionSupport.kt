package com.example.tracer

internal const val REPORTING_TARGET_NOT_FOUND = "reporting.target.not_found"
internal fun QueryReportUiState.copyWithReportOutcome(
    period: DataTreePeriod,
    result: ReportCallResult,
    textProvider: QueryReportTextProvider,
    dayTimeline: StructuredDailyReport? = this.dayTimeline
): QueryReportUiState {
    val summary = buildReportSummary(period, result)
    val report = if (result.operationOk) {
        QueryResult.Report(
            text = result.outputText,
            summary = summary
        )
    } else {
        null
    }
    val nextReportResults = if (report != null) {
        reportResultsByPeriod + (period to report)
    } else {
        reportResultsByPeriod - period
    }
    val nextReportSummaries = if (summary != null) {
        reportSummariesByPeriod + (period to summary)
    } else {
        reportSummariesByPeriod - period
    }
    val nextReportErrors = when {
        result.operationOk -> reportErrorsByPeriod - period
        summary is ReportSummary.MissingTarget || summary is ReportSummary.NoData ->
            reportErrorsByPeriod - period
        else -> reportErrorsByPeriod + (period to result.outputText)
    }
    return copy(
        reportResultsByPeriod = nextReportResults,
        reportSummariesByPeriod = nextReportSummaries,
        reportErrorsByPeriod = nextReportErrors,
        dayTimeline = if (period == DataTreePeriod.DAY) dayTimeline else this.dayTimeline,
        dayReportNeedsRefresh = if (period == DataTreePeriod.DAY) false else dayReportNeedsRefresh,
        activeResult = report,
        analysisError = "",
        statusText = resolveReportStatusText(
            period = period,
            result = result,
            textProvider = textProvider
        )
    )
}

internal fun buildReportSummary(
    period: DataTreePeriod,
    result: ReportCallResult
): ReportSummary? {
    val errorContract = result.errorContract
    if (!result.operationOk &&
        errorContract?.errorCode == REPORTING_TARGET_NOT_FOUND &&
        period == DataTreePeriod.DAY
    ) {
        return ReportSummary.NoData(period = period)
    }
    if (!result.operationOk &&
        errorContract?.errorCode == REPORTING_TARGET_NOT_FOUND &&
        period.isNamedTargetPeriod()
    ) {
        return ReportSummary.MissingTarget(
            period = period,
            errorCode = errorContract.errorCode,
            errorCategory = errorContract.errorCategory,
            hints = errorContract.hints
        )
    }
    if (result.operationOk &&
        period.isWindowedPeriod()
    ) {
        val metadata = result.reportWindowMetadata ?: return null
        return ReportSummary.WindowMetadata(
            period = period,
            metadata = metadata
        )
    }
    return null
}

internal fun resolveReportStatusText(
    period: DataTreePeriod,
    result: ReportCallResult,
    textProvider: QueryReportTextProvider
): String {
    val mode = textProvider.periodLabel(period)
    return when {
        buildReportSummary(period, result) is ReportSummary.NoData ->
            textProvider.nativeReportNoData(mode)
        buildReportSummary(period, result) is ReportSummary.MissingTarget ->
            textProvider.nativeReportTargetMissing(mode)
        buildReportSummary(period, result) is ReportSummary.WindowMetadata &&
            result.reportWindowMetadata?.hasRecords == false ->
            textProvider.nativeReportEmptyWindow(mode)
        else -> textProvider.nativeReportResult(mode = mode, ok = result.operationOk)
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

