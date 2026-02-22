package com.example.tracer

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
    return runningState.copy(
        activeResult = if (result.operationOk) QueryResult.Report(result.outputText) else null,
        statusText = textProvider.nativeReportResult(
            mode = textProvider.periodLabel(DataTreePeriod.DAY),
            ok = result.operationOk
        )
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
    return runningState.copy(
        activeResult = if (result.operationOk) QueryResult.Report(result.outputText) else null,
        statusText = textProvider.nativeReportResult(
            mode = textProvider.periodLabel(DataTreePeriod.MONTH),
            ok = result.operationOk
        )
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
    return runningState.copy(
        activeResult = if (result.operationOk) QueryResult.Report(result.outputText) else null,
        statusText = textProvider.nativeReportResult(
            mode = textProvider.periodLabel(DataTreePeriod.YEAR),
            ok = result.operationOk
        )
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
    return runningState.copy(
        activeResult = if (result.operationOk) QueryResult.Report(result.outputText) else null,
        statusText = textProvider.nativeReportResult(
            mode = textProvider.periodLabel(DataTreePeriod.WEEK),
            ok = result.operationOk
        )
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
    return runningState.copy(
        activeResult = if (result.operationOk) QueryResult.Report(result.outputText) else null,
        statusText = textProvider.nativeReportResult(
            mode = textProvider.periodLabel(DataTreePeriod.RECENT),
            ok = result.operationOk
        )
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
    return runningState.copy(
        activeResult = if (result.operationOk) QueryResult.Report(result.outputText) else null,
        statusText = textProvider.nativeReportResult(
            mode = textProvider.periodLabel(DataTreePeriod.RANGE),
            ok = result.operationOk
        )
    )
}
