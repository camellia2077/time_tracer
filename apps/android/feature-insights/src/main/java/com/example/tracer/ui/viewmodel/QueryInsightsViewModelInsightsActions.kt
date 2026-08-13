package com.example.tracer

private suspend fun InsightsGateway.loadStructuredInsights(
    request: TemporalInsightsQueryRequest,
    markdownResult: InsightsCallResult
): StructuredInsightsCallResult? = if (markdownResult.operationOk) {
    insightsStructured(request).takeIf { it.operationOk }
} else {
    null
}

internal suspend fun runDayInsightsAction(
    currentState: QueryInsightsUiState,
    inputValidator: QueryInputValidator,
    textProvider: QueryInsightsTextProvider,
    insightsGateway: InsightsGateway,
    locale: String = "en",
    emit: (QueryInsightsUiState) -> Unit
): QueryInsightsUiState {
    val dayDigits = currentState.insightsDate.trim()
    val validationError = inputValidator.validateDateDigits(dayDigits)
    if (validationError != null) {
        return currentState.copy(
            statusText = validationError,
            activeResult = null,
            dayTimeline = null
        )
    }

    val dayIso = inputValidator.toIsoDate(dayDigits)
    val request = TemporalInsightsQueryRequest(
        displayMode = InsightsDisplayMode.DAY,
        selection = TemporalSelectionPayload(
            kind = TemporalSelectionKind.SINGLE_DAY,
            date = dayIso
        ),
        locale = locale
    )
    val runningState = currentState.copy(
        dayTimeline = null,
        statusValues = emptyList(),
        statusText = textProvider.nativeInsightsRunning(textProvider.periodLabel(DataTreePeriod.DAY))
    )
    emit(runningState)
    val result = insightsGateway.insightsMarkdown(request)
    val structuredResult = insightsGateway.loadStructuredInsights(request, result)
    val nextState = runningState.copyWithInsightsOutcome(
        period = DataTreePeriod.DAY,
        result = result,
        textProvider = textProvider,
        dayTimeline = structuredResult?.insights,
        activityDays = structuredResult.activityDaysFor(DataTreePeriod.DAY),
        projectTree = structuredResult.projectTreeFor(),
        statusValues = structuredResult?.takeIf { it.operationOk }?.statuses.orEmpty()
    )
    return nextState
}
internal suspend fun runMonthInsightsAction(
    currentState: QueryInsightsUiState,
    inputValidator: QueryInputValidator,
    textProvider: QueryInsightsTextProvider,
    insightsGateway: InsightsGateway,
    locale: String = "en",
    emit: (QueryInsightsUiState) -> Unit
): QueryInsightsUiState {
    val monthDigits = currentState.insightsMonth.trim()
    val validationError = inputValidator.validateMonthDigits(monthDigits)
    if (validationError != null) {
        return currentState.copy(statusText = validationError, activeResult = null)
    }

    val monthIso = inputValidator.toIsoMonth(monthDigits)
    val runningState = currentState.copy(
        statusValues = emptyList(),
        statusText = textProvider.nativeInsightsRunning(textProvider.periodLabel(DataTreePeriod.MONTH))
    )
    emit(runningState)
    val request = TemporalInsightsQueryRequest(
            displayMode = InsightsDisplayMode.MONTH,
            selection = TemporalSelectionPayload(
                kind = TemporalSelectionKind.DATE_RANGE,
                startDate = "$monthIso-01",
                endDate = java.time.YearMonth.parse(monthIso).atEndOfMonth().toString()
            ),
            locale = locale
        )
    val result = insightsGateway.insightsMarkdown(request)
    val structuredResult = insightsGateway.loadStructuredInsights(request, result)
    return runningState.copyWithInsightsOutcome(
        period = DataTreePeriod.MONTH,
        result = result,
        textProvider = textProvider,
        activityDays = structuredResult.activityDaysFor(DataTreePeriod.MONTH),
        projectTree = structuredResult.projectTreeFor(),
        statusValues = structuredResult?.statuses.orEmpty()
    )
}
internal suspend fun runYearInsightsAction(
    currentState: QueryInsightsUiState,
    inputValidator: QueryInputValidator,
    textProvider: QueryInsightsTextProvider,
    insightsGateway: InsightsGateway,
    locale: String = "en",
    emit: (QueryInsightsUiState) -> Unit
): QueryInsightsUiState {
    val year = currentState.insightsYear.trim()
    val validationError = inputValidator.validateIsoYear(year)
    if (validationError != null) {
        return currentState.copy(statusText = validationError, activeResult = null)
    }

    val runningState = currentState.copy(
        statusValues = emptyList(),
        statusText = textProvider.nativeInsightsRunning(textProvider.periodLabel(DataTreePeriod.YEAR))
    )
    emit(runningState)
    val request = TemporalInsightsQueryRequest(
            displayMode = InsightsDisplayMode.YEAR,
            selection = TemporalSelectionPayload(
                kind = TemporalSelectionKind.DATE_RANGE,
                startDate = "$year-01-01",
                endDate = "$year-12-31"
            ),
            locale = locale
        )
    val result = insightsGateway.insightsMarkdown(request)
    val structuredResult = insightsGateway.loadStructuredInsights(request, result)
    return runningState.copyWithInsightsOutcome(
        period = DataTreePeriod.YEAR,
        result = result,
        textProvider = textProvider,
        activityDays = structuredResult.activityDaysFor(DataTreePeriod.YEAR),
        projectTree = structuredResult.projectTreeFor(),
        statusValues = structuredResult?.statuses.orEmpty()
    )
}

internal suspend fun runWeekInsightsAction(
    currentState: QueryInsightsUiState,
    inputValidator: QueryInputValidator,
    textProvider: QueryInsightsTextProvider,
    insightsGateway: InsightsGateway,
    locale: String = "en",
    emit: (QueryInsightsUiState) -> Unit
): QueryInsightsUiState {
    val weekDigits = currentState.insightsWeek.trim()
    val validationError = inputValidator.validateWeekDigits(weekDigits)
    if (validationError != null) {
        return currentState.copy(statusText = validationError, activeResult = null)
    }

    val runningState = currentState.copy(
        statusValues = emptyList(),
        statusText = textProvider.nativeInsightsRunning(textProvider.periodLabel(DataTreePeriod.WEEK))
    )
    emit(runningState)
    val weekRange = resolveIsoWeekSelection(weekDigits)
        ?: return currentState.copy(statusText = textProvider.invalidWeekFormat(), activeResult = null)
    val request = TemporalInsightsQueryRequest(
            displayMode = InsightsDisplayMode.WEEK,
            selection = TemporalSelectionPayload(
                kind = TemporalSelectionKind.DATE_RANGE,
                startDate = weekRange.weekStart.toString(),
                endDate = weekRange.weekEnd.toString()
            ),
            locale = locale
        )
    val result = insightsGateway.insightsMarkdown(request)
    val structuredResult = insightsGateway.loadStructuredInsights(request, result)
    return runningState.copyWithInsightsOutcome(
        period = DataTreePeriod.WEEK,
        result = result,
        textProvider = textProvider,
        activityDays = structuredResult.activityDaysFor(DataTreePeriod.WEEK),
        projectTree = structuredResult.projectTreeFor(),
        statusValues = structuredResult?.statuses.orEmpty()
    )
}

internal suspend fun runRecentInsightsAction(
    currentState: QueryInsightsUiState,
    inputValidator: QueryInputValidator,
    textProvider: QueryInsightsTextProvider,
    insightsGateway: InsightsGateway,
    locale: String = "en",
    emit: (QueryInsightsUiState) -> Unit
): QueryInsightsUiState {
    val recentDays = currentState.insightsRecentDays.trim()
    val validationError = inputValidator.validateRecentDays(recentDays)
    if (validationError != null) {
        return currentState.copy(statusText = validationError, activeResult = null)
    }

    val runningState = currentState.copy(
        statusValues = emptyList(),
        statusText = textProvider.nativeInsightsRunning(textProvider.periodLabel(DataTreePeriod.RECENT))
    )
    emit(runningState)
    val request = TemporalInsightsQueryRequest(
            displayMode = InsightsDisplayMode.RECENT,
            selection = TemporalSelectionPayload(
                kind = TemporalSelectionKind.RECENT_DAYS,
                days = recentDays.toInt()
            ),
            locale = locale
        )
    val result = insightsGateway.insightsMarkdown(request)
    val structuredResult = insightsGateway.loadStructuredInsights(request, result)
    return runningState.copyWithInsightsOutcome(
        period = DataTreePeriod.RECENT,
        result = result,
        textProvider = textProvider,
        activityDays = structuredResult.activityDaysFor(DataTreePeriod.RECENT),
        projectTree = structuredResult.projectTreeFor(),
        statusValues = structuredResult?.statuses.orEmpty()
    )
}

internal suspend fun runRangeInsightsAction(
    currentState: QueryInsightsUiState,
    inputValidator: QueryInputValidator,
    textProvider: QueryInsightsTextProvider,
    insightsGateway: InsightsGateway,
    locale: String = "en",
    emit: (QueryInsightsUiState) -> Unit
): QueryInsightsUiState {
    val startDateDigits = currentState.insightsRangeStartDate.trim()
    val endDateDigits = currentState.insightsRangeEndDate.trim()

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
        statusValues = emptyList(),
        statusText = textProvider.nativeInsightsRunning(textProvider.periodLabel(DataTreePeriod.RANGE))
    )
    emit(runningState)
    val request = TemporalInsightsQueryRequest(
            displayMode = InsightsDisplayMode.RANGE,
            selection = TemporalSelectionPayload(
                kind = TemporalSelectionKind.DATE_RANGE,
                startDate = startIso,
                endDate = endIso
            ),
            locale = locale
        )
    val result = insightsGateway.insightsMarkdown(request)
    val structuredResult = insightsGateway.loadStructuredInsights(request, result)
    return runningState.copyWithInsightsOutcome(
        period = DataTreePeriod.RANGE,
        result = result,
        textProvider = textProvider,
        activityDays = structuredResult.activityDaysFor(DataTreePeriod.RANGE),
        projectTree = structuredResult.projectTreeFor(),
        statusValues = structuredResult?.statuses.orEmpty()
    )
}

private fun StructuredInsightsCallResult?.activityDaysFor(
    period: DataTreePeriod
): List<StructuredDailyInsights> = when {
    this == null -> emptyList()
    period == DataTreePeriod.DAY -> listOfNotNull(insights)
    else -> activityDays
}

private fun StructuredInsightsCallResult?.projectTreeFor(): List<StructuredInsightsProjectNode> =
    this?.projectTree.orEmpty()
