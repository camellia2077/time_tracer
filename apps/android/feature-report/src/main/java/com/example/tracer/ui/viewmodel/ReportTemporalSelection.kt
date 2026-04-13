package com.example.tracer

import java.time.LocalDate
import java.time.YearMonth

internal sealed interface ReportTemporalSelection {
    val reportMode: ReportMode

    data class SingleDay(
        override val reportMode: ReportMode,
        val date: LocalDate
    ) : ReportTemporalSelection

    data class DateRange(
        override val reportMode: ReportMode,
        val start: LocalDate,
        val end: LocalDate
    ) : ReportTemporalSelection

    data class RecentDays(
        override val reportMode: ReportMode,
        val days: Int
    ) : ReportTemporalSelection
}

internal sealed interface ReportTemporalSelectionResolveResult {
    data class Success(
        val selection: ReportTemporalSelection
    ) : ReportTemporalSelectionResolveResult

    data class Failure(
        val reportMode: ReportMode,
        val validationError: String
    ) : ReportTemporalSelectionResolveResult
}

internal class ReportTemporalSelectionResolver(
    private val inputValidator: QueryInputValidator,
    private val textProvider: QueryReportTextProvider
) {
    fun resolve(currentState: QueryReportUiState): ReportTemporalSelectionResolveResult {
        return when (currentState.reportMode) {
            ReportMode.DAY -> resolveDay(currentState.reportDate.trim())
            ReportMode.WEEK -> resolveWeek(currentState.reportWeek.trim())
            ReportMode.MONTH -> resolveMonth(currentState.reportMonth.trim())
            ReportMode.YEAR -> resolveYear(currentState.reportYear.trim())
            ReportMode.RANGE -> resolveRange(
                startDigits = currentState.reportRangeStartDate.trim(),
                endDigits = currentState.reportRangeEndDate.trim()
            )
            ReportMode.RECENT -> resolveRecent(currentState.reportRecentDays.trim())
        }
    }

    private fun resolveDay(reportDateDigits: String): ReportTemporalSelectionResolveResult {
        val validationError = inputValidator.validateDateDigits(reportDateDigits)
        if (validationError != null) {
            return ReportTemporalSelectionResolveResult.Failure(
                reportMode = ReportMode.DAY,
                validationError = validationError
            )
        }
        return ReportTemporalSelectionResolveResult.Success(
            ReportTemporalSelection.SingleDay(
                reportMode = ReportMode.DAY,
                date = LocalDate.parse(inputValidator.toIsoDate(reportDateDigits))
            )
        )
    }

    private fun resolveWeek(reportWeekDigits: String): ReportTemporalSelectionResolveResult {
        val validationError = inputValidator.validateWeekDigits(reportWeekDigits)
        if (validationError != null) {
            return ReportTemporalSelectionResolveResult.Failure(
                reportMode = ReportMode.WEEK,
                validationError = validationError
            )
        }
        val selection = resolveIsoWeekSelection(reportWeekDigits)
            ?: return ReportTemporalSelectionResolveResult.Failure(
                reportMode = ReportMode.WEEK,
                validationError = textProvider.invalidWeekFormat()
            )
        return ReportTemporalSelectionResolveResult.Success(
            ReportTemporalSelection.DateRange(
                reportMode = ReportMode.WEEK,
                start = selection.weekStart,
                end = selection.weekEnd
            )
        )
    }

    private fun resolveMonth(reportMonthDigits: String): ReportTemporalSelectionResolveResult {
        val validationError = inputValidator.validateMonthDigits(reportMonthDigits)
        if (validationError != null) {
            return ReportTemporalSelectionResolveResult.Failure(
                reportMode = ReportMode.MONTH,
                validationError = validationError
            )
        }
        val month = parseYearMonthDigits(reportMonthDigits)
            ?: return ReportTemporalSelectionResolveResult.Failure(
                reportMode = ReportMode.MONTH,
                validationError = textProvider.invalidMonthFormat()
            )
        return ReportTemporalSelectionResolveResult.Success(
            ReportTemporalSelection.DateRange(
                reportMode = ReportMode.MONTH,
                start = month.atDay(1),
                end = month.atEndOfMonth()
            )
        )
    }

    private fun resolveYear(reportYearDigits: String): ReportTemporalSelectionResolveResult {
        val validationError = inputValidator.validateIsoYear(reportYearDigits)
        if (validationError != null) {
            return ReportTemporalSelectionResolveResult.Failure(
                reportMode = ReportMode.YEAR,
                validationError = validationError
            )
        }
        val year = reportYearDigits.toIntOrNull()
            ?: return ReportTemporalSelectionResolveResult.Failure(
                reportMode = ReportMode.YEAR,
                validationError = textProvider.invalidYearFormat()
            )
        return ReportTemporalSelectionResolveResult.Success(
            ReportTemporalSelection.DateRange(
                reportMode = ReportMode.YEAR,
                start = LocalDate.of(year, 1, 1),
                end = LocalDate.of(year, 12, 31)
            )
        )
    }

    private fun resolveRange(
        startDigits: String,
        endDigits: String
    ): ReportTemporalSelectionResolveResult {
        if (startDigits.isBlank() || endDigits.isBlank()) {
            return ReportTemporalSelectionResolveResult.Failure(
                reportMode = ReportMode.RANGE,
                validationError = textProvider.chartRangeBothRequired()
            )
        }

        val startValidationError = inputValidator.validateDateDigits(startDigits)
        if (startValidationError != null) {
            return ReportTemporalSelectionResolveResult.Failure(
                reportMode = ReportMode.RANGE,
                validationError = textProvider.chartRangeStartDateInvalid()
            )
        }

        val endValidationError = inputValidator.validateDateDigits(endDigits)
        if (endValidationError != null) {
            return ReportTemporalSelectionResolveResult.Failure(
                reportMode = ReportMode.RANGE,
                validationError = textProvider.chartRangeEndDateInvalid()
            )
        }

        val rangeOrderError = inputValidator.validateRangeOrder(startDigits, endDigits)
        if (rangeOrderError != null) {
            return ReportTemporalSelectionResolveResult.Failure(
                reportMode = ReportMode.RANGE,
                validationError = textProvider.chartRangeOrderInvalid()
            )
        }

        return ReportTemporalSelectionResolveResult.Success(
            ReportTemporalSelection.DateRange(
                reportMode = ReportMode.RANGE,
                start = LocalDate.parse(inputValidator.toIsoDate(startDigits)),
                end = LocalDate.parse(inputValidator.toIsoDate(endDigits))
            )
        )
    }

    private fun resolveRecent(reportRecentDays: String): ReportTemporalSelectionResolveResult {
        val validationError = inputValidator.validateRecentDays(reportRecentDays)
        if (validationError != null) {
            return ReportTemporalSelectionResolveResult.Failure(
                reportMode = ReportMode.RECENT,
                validationError = validationError
            )
        }
        return ReportTemporalSelectionResolveResult.Success(
            ReportTemporalSelection.RecentDays(
                reportMode = ReportMode.RECENT,
                days = reportRecentDays.toInt()
            )
        )
    }

    private fun parseYearMonthDigits(reportMonthDigits: String): YearMonth? {
        if (reportMonthDigits.length != 6) {
            return null
        }
        val year = reportMonthDigits.take(4).toIntOrNull() ?: return null
        val month = reportMonthDigits.takeLast(2).toIntOrNull() ?: return null
        return try {
            YearMonth.of(year, month)
        } catch (_: RuntimeException) {
            null
        }
    }
}
