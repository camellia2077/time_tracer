package com.example.tracer

import java.time.LocalDate
import java.time.format.DateTimeParseException

internal fun validateReportChartQueryParams(
    lookbackDays: Int,
    fromDateIso: String?,
    toDateIso: String?
): ReportChartQueryResult? {
    val hasFrom = !fromDateIso.isNullOrBlank()
    val hasTo = !toDateIso.isNullOrBlank()
    if (hasFrom != hasTo) {
        return ReportChartQueryResult(
            ok = false,
            data = null,
            message = "fromDateIso and toDateIso must be provided together."
        )
    }

    if (hasFrom && hasTo) {
        val start = parseIsoDateOrNull(fromDateIso)
            ?: return ReportChartQueryResult(
                ok = false,
                data = null,
                message = "fromDateIso must be YYYY-MM-DD."
            )
        val end = parseIsoDateOrNull(toDateIso)
            ?: return ReportChartQueryResult(
                ok = false,
                data = null,
                message = "toDateIso must be YYYY-MM-DD."
            )
        if (start.isAfter(end)) {
            return ReportChartQueryResult(
                ok = false,
                data = null,
                message = "fromDateIso must be less than or equal to toDateIso."
            )
        }
        return null
    }

    if (lookbackDays <= 0) {
        return ReportChartQueryResult(
            ok = false,
            data = null,
            message = "lookbackDays must be greater than 0."
        )
    }
    return null
}

internal fun normalizePeriodArgument(periodArgument: String?): String {
    return periodArgument?.trim().orEmpty()
}

internal fun validatePeriodArgument(
    period: DataTreePeriod,
    normalizedPeriodArgument: String
): DataQueryTextResult? {
    if (period == DataTreePeriod.RECENT || normalizedPeriodArgument.isNotBlank()) {
        return null
    }
    return DataQueryTextResult(
        ok = false,
        outputText = "",
        message = "periodArgument is required for period ${period.wireValue}."
    )
}

internal fun validateAndNormalizePeriodArgument(
    period: DataTreePeriod,
    periodArgument: String?
): PeriodArgumentValidationResult {
    val normalizedArgument = normalizePeriodArgument(periodArgument)
    val validationError = validatePeriodArgument(period, normalizedArgument)
    return PeriodArgumentValidationResult(
        argument = normalizedArgument,
        error = validationError
    )
}

internal fun validateSuggestionQueryParams(
    lookbackDays: Int,
    topN: Int
): ActivitySuggestionResult? {
    if (lookbackDays <= 0) {
        return ActivitySuggestionResult(
            ok = false,
            suggestions = emptyList(),
            message = "lookbackDays must be greater than 0."
        )
    }
    if (topN <= 0) {
        return ActivitySuggestionResult(
            ok = false,
            suggestions = emptyList(),
            message = "topN must be greater than 0."
        )
    }
    return null
}

private fun parseIsoDateOrNull(value: String?): LocalDate? {
    if (value.isNullOrBlank()) {
        return null
    }
    return try {
        LocalDate.parse(value)
    } catch (_: DateTimeParseException) {
        null
    }
}
