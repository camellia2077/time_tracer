package com.example.tracer

internal suspend fun runChartQueryAction(
    currentState: QueryReportUiState,
    inputValidator: QueryInputValidator,
    textProvider: QueryReportTextProvider,
    queryGateway: QueryGateway
): QueryReportUiState {
    val lookbackDaysText = currentState.chartLookbackDays.trim()
    val rangeStartDigits = currentState.chartRangeStartDate.trim()
    val rangeEndDigits = currentState.chartRangeEndDate.trim()
    val useRangeQuery = currentState.chartDateInputMode == ChartDateInputMode.RANGE

    var fromDateIso: String? = null
    var toDateIso: String? = null
    val lookbackDays: Int

    if (useRangeQuery) {
        if (rangeStartDigits.isBlank() || rangeEndDigits.isBlank()) {
            val error = textProvider.chartRangeBothRequired()
            return currentState.copy(
                chartLoading = false,
                chartError = error,
                resultDisplayMode = ReportResultDisplayMode.CHART,
                statusText = error
            )
        }

        val startValidationError = inputValidator.validateDateDigits(rangeStartDigits)
        if (startValidationError != null) {
            val error = textProvider.chartRangeStartDateInvalid()
            return currentState.copy(
                chartLoading = false,
                chartError = error,
                resultDisplayMode = ReportResultDisplayMode.CHART,
                statusText = error
            )
        }

        val endValidationError = inputValidator.validateDateDigits(rangeEndDigits)
        if (endValidationError != null) {
            val error = textProvider.chartRangeEndDateInvalid()
            return currentState.copy(
                chartLoading = false,
                chartError = error,
                resultDisplayMode = ReportResultDisplayMode.CHART,
                statusText = error
            )
        }

        val rangeOrderError = inputValidator.validateRangeOrder(rangeStartDigits, rangeEndDigits)
        if (rangeOrderError != null) {
            val error = textProvider.chartRangeOrderInvalid()
            return currentState.copy(
                chartLoading = false,
                chartError = error,
                resultDisplayMode = ReportResultDisplayMode.CHART,
                statusText = error
            )
        }

        fromDateIso = inputValidator.toIsoDate(rangeStartDigits)
        toDateIso = inputValidator.toIsoDate(rangeEndDigits)
        lookbackDays = lookbackDaysText.toIntOrNull()?.takeIf { it > 0 } ?: 7
    } else {
        val lookbackValidationError = inputValidator.validateRecentDays(lookbackDaysText)
        if (lookbackValidationError != null) {
            return currentState.copy(
                chartLoading = false,
                chartError = lookbackValidationError,
                resultDisplayMode = ReportResultDisplayMode.CHART,
                statusText = lookbackValidationError
            )
        }
        lookbackDays = lookbackDaysText.toInt()
    }
    val selectedRoot = currentState.chartSelectedRoot.trim().ifEmpty { null }

    val runningState = currentState.copy(
        chartLoading = true,
        chartError = "",
        chartUsesLegacyStatsFallback = false,
        resultDisplayMode = ReportResultDisplayMode.CHART,
        statusText = textProvider.queryChartRunning()
    )

    val result = queryGateway.queryReportChart(
        ReportChartQueryParams(
            root = selectedRoot,
            lookbackDays = lookbackDays,
            fromDateIso = fromDateIso,
            toDateIso = toDateIso
        )
    )

    val chartData = result.data
    if (!result.ok || chartData == null) {
        return runningState.copy(
            chartLoading = false,
            chartError = result.message.ifBlank { textProvider.chartPayloadInvalid() },
            statusText = textProvider.queryChartResult(ok = false)
        )
    }

    val roots = chartData.roots.distinct()
    val resolvedRoot = when {
        selectedRoot != null && roots.contains(selectedRoot) -> selectedRoot
        chartData.selectedRoot.isNotBlank() && roots.contains(chartData.selectedRoot) ->
            chartData.selectedRoot

        selectedRoot != null -> selectedRoot
        else -> ""
    }

    return runningState.copy(
        chartLoading = false,
        chartError = "",
        chartRoots = roots,
        chartSelectedRoot = resolvedRoot,
        chartLookbackDays = if (useRangeQuery) {
            runningState.chartLookbackDays
        } else {
            chartData.lookbackDays.toString()
        },
        chartPoints = chartData.points.sortedBy { it.date },
        chartAverageDurationSeconds = chartData.averageDurationSeconds,
        chartTotalDurationSeconds = chartData.totalDurationSeconds,
        chartActiveDays = chartData.activeDays,
        chartRangeDays = chartData.rangeDays,
        chartUsesLegacyStatsFallback = chartData.usesLegacyStatsFallback,
        statusText = textProvider.queryChartResult(ok = true)
    )
}
