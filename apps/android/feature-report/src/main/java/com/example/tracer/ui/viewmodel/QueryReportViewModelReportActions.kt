package com.example.tracer

private const val REPORTING_TARGET_NOT_FOUND = "reporting.target.not_found"

internal suspend fun runDayReportAction(
    currentState: QueryReportUiState,
    inputValidator: QueryInputValidator,
    textProvider: QueryReportTextProvider,
    reportGateway: ReportGateway,
    emit: (QueryReportUiState) -> Unit
): QueryReportUiState {
    val dayDigits = currentState.reportDate.trim()
    val validationError = inputValidator.validateDateDigits(dayDigits)
    if (validationError != null) {
        return currentState.copy(statusText = validationError, activeResult = null)
    }

    val dayIso = inputValidator.toIsoDate(dayDigits)
    val runningState = currentState.copy(
        statusText = textProvider.nativeReportRunning(textProvider.periodLabel(DataTreePeriod.DAY))
    )
    emit(runningState)
    val result = reportGateway.reportDayMarkdown(dayIso)
    return runningState.copyWithReportOutcome(
        period = DataTreePeriod.DAY,
        result = result,
        textProvider = textProvider
    )
}

internal suspend fun runMonthReportAction(
    currentState: QueryReportUiState,
    inputValidator: QueryInputValidator,
    textProvider: QueryReportTextProvider,
    reportGateway: ReportGateway,
    emit: (QueryReportUiState) -> Unit
): QueryReportUiState {
    val monthDigits = currentState.reportMonth.trim()
    val validationError = inputValidator.validateMonthDigits(monthDigits)
    if (validationError != null) {
        return currentState.copy(statusText = validationError, activeResult = null)
    }

    val monthIso = inputValidator.toIsoMonth(monthDigits)
    val runningState = currentState.copy(
        statusText = textProvider.nativeReportRunning(textProvider.periodLabel(DataTreePeriod.MONTH))
    )
    emit(runningState)
    val result = reportGateway.reportMonthMarkdown(monthIso)
    return runningState.copyWithReportOutcome(
        period = DataTreePeriod.MONTH,
        result = result,
        textProvider = textProvider
    )
}

internal suspend fun runYearReportAction(
    currentState: QueryReportUiState,
    inputValidator: QueryInputValidator,
    textProvider: QueryReportTextProvider,
    reportGateway: ReportGateway,
    emit: (QueryReportUiState) -> Unit
): QueryReportUiState {
    val year = currentState.reportYear.trim()
    val validationError = inputValidator.validateIsoYear(year)
    if (validationError != null) {
        return currentState.copy(statusText = validationError, activeResult = null)
    }

    val runningState = currentState.copy(
        statusText = textProvider.nativeReportRunning(textProvider.periodLabel(DataTreePeriod.YEAR))
    )
    emit(runningState)
    val result = reportGateway.reportYearMarkdown(year)
    return runningState.copyWithReportOutcome(
        period = DataTreePeriod.YEAR,
        result = result,
        textProvider = textProvider
    )
}

internal suspend fun runWeekReportAction(
    currentState: QueryReportUiState,
    inputValidator: QueryInputValidator,
    textProvider: QueryReportTextProvider,
    reportGateway: ReportGateway,
    emit: (QueryReportUiState) -> Unit
): QueryReportUiState {
    val weekDigits = currentState.reportWeek.trim()
    val validationError = inputValidator.validateWeekDigits(weekDigits)
    if (validationError != null) {
        return currentState.copy(statusText = validationError, activeResult = null)
    }

    val weekIso = inputValidator.toIsoWeek(weekDigits)
    val runningState = currentState.copy(
        statusText = textProvider.nativeReportRunning(textProvider.periodLabel(DataTreePeriod.WEEK))
    )
    emit(runningState)
    val result = reportGateway.reportWeekMarkdown(weekIso)
    return runningState.copyWithReportOutcome(
        period = DataTreePeriod.WEEK,
        result = result,
        textProvider = textProvider
    )
}

internal suspend fun runRecentReportAction(
    currentState: QueryReportUiState,
    inputValidator: QueryInputValidator,
    textProvider: QueryReportTextProvider,
    reportGateway: ReportGateway,
    emit: (QueryReportUiState) -> Unit
): QueryReportUiState {
    val recentDays = currentState.reportRecentDays.trim()
    val validationError = inputValidator.validateRecentDays(recentDays)
    if (validationError != null) {
        return currentState.copy(statusText = validationError, activeResult = null)
    }

    val runningState = currentState.copy(
        statusText = textProvider.nativeReportRunning(textProvider.periodLabel(DataTreePeriod.RECENT))
    )
    emit(runningState)
    val result = reportGateway.reportRecentMarkdown(recentDays)
    return runningState.copyWithReportOutcome(
        period = DataTreePeriod.RECENT,
        result = result,
        textProvider = textProvider
    )
}

internal suspend fun runRangeReportAction(
    currentState: QueryReportUiState,
    inputValidator: QueryInputValidator,
    textProvider: QueryReportTextProvider,
    reportGateway: ReportGateway,
    emit: (QueryReportUiState) -> Unit
): QueryReportUiState {
    val startDateDigits = currentState.reportRangeStartDate.trim()
    val endDateDigits = currentState.reportRangeEndDate.trim()

    val startValidationError = inputValidator.validateDateDigits(startDateDigits)
    if (startValidationError != null) {
        return currentState.copy(
            statusText = textProvider.rangeStartDateInvalid(startValidationError),
            activeResult = null
        )
    }

    val endValidationError = inputValidator.validateDateDigits(endDateDigits)
    if (endValidationError != null) {
        return currentState.copy(
            statusText = textProvider.rangeEndDateInvalid(endValidationError),
            activeResult = null
        )
    }

    val rangeOrderError = inputValidator.validateRangeOrder(startDateDigits, endDateDigits)
    if (rangeOrderError != null) {
        return currentState.copy(statusText = rangeOrderError, activeResult = null)
    }

    val startIso = inputValidator.toIsoDate(startDateDigits)
    val endIso = inputValidator.toIsoDate(endDateDigits)
    val runningState = currentState.copy(
        statusText = textProvider.nativeReportRunning(textProvider.periodLabel(DataTreePeriod.RANGE))
    )
    emit(runningState)
    val result = reportGateway.reportRange(startDate = startIso, endDate = endIso)
    return runningState.copyWithReportOutcome(
        period = DataTreePeriod.RANGE,
        result = result,
        textProvider = textProvider
    )
}

private fun QueryReportUiState.copyWithReportOutcome(
    period: DataTreePeriod,
    result: ReportCallResult,
    textProvider: QueryReportTextProvider
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
        summary is ReportSummary.MissingTarget -> reportErrorsByPeriod - period
        else -> reportErrorsByPeriod + (period to result.outputText)
    }
    return copy(
        reportResultsByPeriod = nextReportResults,
        reportSummariesByPeriod = nextReportSummaries,
        reportErrorsByPeriod = nextReportErrors,
        activeResult = report,
        analysisError = "",
        statusText = resolveReportStatusText(
            period = period,
            result = result,
            textProvider = textProvider
        )
    )
}

private fun buildReportSummary(
    period: DataTreePeriod,
    result: ReportCallResult
): ReportSummary? {
    val errorContract = result.errorContract
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

private fun resolveReportStatusText(
    period: DataTreePeriod,
    result: ReportCallResult,
    textProvider: QueryReportTextProvider
): String {
    val mode = textProvider.periodLabel(period)
    return when {
        buildReportSummary(period, result) is ReportSummary.MissingTarget ->
            textProvider.nativeReportTargetMissing(mode)
        buildReportSummary(period, result) is ReportSummary.WindowMetadata &&
            result.reportWindowMetadata?.hasRecords == false ->
            textProvider.nativeReportEmptyWindow(mode)
        else -> textProvider.nativeReportResult(mode = mode, ok = result.operationOk)
    }
}

private fun DataTreePeriod.isNamedTargetPeriod(): Boolean {
    return this == DataTreePeriod.DAY ||
        this == DataTreePeriod.WEEK ||
        this == DataTreePeriod.MONTH ||
        this == DataTreePeriod.YEAR
}

private fun DataTreePeriod.isWindowedPeriod(): Boolean {
    return this == DataTreePeriod.RECENT || this == DataTreePeriod.RANGE
}
