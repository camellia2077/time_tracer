package com.example.tracer

internal data class ResolvedChartQueryParams(
    val dateInputMode: ChartDateInputMode,
    val lookbackDays: Int,
    val fromDateIso: String?,
    val toDateIso: String?,
    val validationError: String
)

internal class QueryReportChartParamResolver(
    private val inputValidator: QueryInputValidator,
    private val textProvider: QueryReportTextProvider
) {
    fun resolve(currentState: QueryReportUiState): ResolvedChartQueryParams {
        val lookbackDaysText = currentState.chartLookbackDays.trim()
        val rangeStartDigits = currentState.chartRangeStartDate.trim()
        val rangeEndDigits = currentState.chartRangeEndDate.trim()
        val useRangeQuery = currentState.chartDateInputMode == ChartDateInputMode.RANGE

        var fromDateIso: String? = null
        var toDateIso: String? = null
        val lookbackDays: Int

        if (useRangeQuery) {
            if (rangeStartDigits.isBlank() || rangeEndDigits.isBlank()) {
                return ResolvedChartQueryParams(
                    dateInputMode = currentState.chartDateInputMode,
                    lookbackDays = 0,
                    fromDateIso = null,
                    toDateIso = null,
                    validationError = textProvider.chartRangeBothRequired()
                )
            }

            val startValidationError = inputValidator.validateDateDigits(rangeStartDigits)
            if (startValidationError != null) {
                return ResolvedChartQueryParams(
                    dateInputMode = currentState.chartDateInputMode,
                    lookbackDays = 0,
                    fromDateIso = null,
                    toDateIso = null,
                    validationError = textProvider.chartRangeStartDateInvalid()
                )
            }

            val endValidationError = inputValidator.validateDateDigits(rangeEndDigits)
            if (endValidationError != null) {
                return ResolvedChartQueryParams(
                    dateInputMode = currentState.chartDateInputMode,
                    lookbackDays = 0,
                    fromDateIso = null,
                    toDateIso = null,
                    validationError = textProvider.chartRangeEndDateInvalid()
                )
            }

            val rangeOrderError = inputValidator.validateRangeOrder(rangeStartDigits, rangeEndDigits)
            if (rangeOrderError != null) {
                return ResolvedChartQueryParams(
                    dateInputMode = currentState.chartDateInputMode,
                    lookbackDays = 0,
                    fromDateIso = null,
                    toDateIso = null,
                    validationError = textProvider.chartRangeOrderInvalid()
                )
            }

            fromDateIso = inputValidator.toIsoDate(rangeStartDigits)
            toDateIso = inputValidator.toIsoDate(rangeEndDigits)
            lookbackDays = lookbackDaysText.toIntOrNull()?.takeIf { it > 0 } ?: 7
        } else {
            val lookbackValidationError = inputValidator.validateRecentDays(lookbackDaysText)
            if (lookbackValidationError != null) {
                return ResolvedChartQueryParams(
                    dateInputMode = currentState.chartDateInputMode,
                    lookbackDays = 0,
                    fromDateIso = null,
                    toDateIso = null,
                    validationError = lookbackValidationError
                )
            }
            lookbackDays = lookbackDaysText.toInt()
        }

        return ResolvedChartQueryParams(
            dateInputMode = currentState.chartDateInputMode,
            lookbackDays = lookbackDays,
            fromDateIso = fromDateIso,
            toDateIso = toDateIso,
            validationError = ""
        )
    }
}
