package com.example.tracer

import java.time.LocalDate
import java.time.format.DateTimeParseException

internal fun validateReportChartQueryParams(
    lookbackDays: Int,
    fromDateIso: String?,
    toDateIso: String?
): ReportChartQueryResult? {
    val error = validateReportWindowQueryParams(lookbackDays, fromDateIso, toDateIso)
        ?: return null
    return ReportChartQueryResult(
        ok = false,
        data = null,
        message = error
    )
}

internal fun validateReportCompositionQueryParams(
    lookbackDays: Int,
    fromDateIso: String?,
    toDateIso: String?
): ReportCompositionQueryResult? {
    val error = validateReportWindowQueryParams(lookbackDays, fromDateIso, toDateIso)
        ?: return null
    return ReportCompositionQueryResult(
        ok = false,
        data = null,
        message = error
    )
}

private fun validateReportWindowQueryParams(
    lookbackDays: Int,
    fromDateIso: String?,
    toDateIso: String?
): String? {
    val hasFrom = !fromDateIso.isNullOrBlank()
    val hasTo = !toDateIso.isNullOrBlank()
    if (hasFrom != hasTo) {
        return "fromDateIso and toDateIso must be provided together."
    }

    if (hasFrom && hasTo) {
        val start = parseIsoDateOrNull(fromDateIso)
            ?: return "fromDateIso must be YYYY-MM-DD."
        val end = parseIsoDateOrNull(toDateIso)
            ?: return "toDateIso must be YYYY-MM-DD."
        if (start.isAfter(end)) {
            return "fromDateIso must be less than or equal to toDateIso."
        }
        return null
    }

    if (lookbackDays <= 0) {
        return "lookbackDays must be greater than 0."
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
