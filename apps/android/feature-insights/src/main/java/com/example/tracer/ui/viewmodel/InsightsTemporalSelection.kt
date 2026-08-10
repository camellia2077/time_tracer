package com.example.tracer

import java.time.LocalDate
import java.time.YearMonth

internal sealed interface InsightsTemporalSelection {
    val insightsMode: InsightsMode

    data class SingleDay(
        override val insightsMode: InsightsMode,
        val date: LocalDate
    ) : InsightsTemporalSelection

    data class DateRange(
        override val insightsMode: InsightsMode,
        val start: LocalDate,
        val end: LocalDate
    ) : InsightsTemporalSelection

    data class RecentDays(
        override val insightsMode: InsightsMode,
        val days: Int
    ) : InsightsTemporalSelection
}

internal sealed interface InsightsTemporalSelectionResolveResult {
    data class Success(
        val selection: InsightsTemporalSelection
    ) : InsightsTemporalSelectionResolveResult

    data class Failure(
        val insightsMode: InsightsMode,
        val validationError: String
    ) : InsightsTemporalSelectionResolveResult
}

internal class InsightsTemporalSelectionResolver(
    private val inputValidator: QueryInputValidator,
    private val textProvider: QueryInsightsTextProvider
) {
    fun resolve(currentState: QueryInsightsUiState): InsightsTemporalSelectionResolveResult {
        return when (currentState.insightsMode) {
            InsightsMode.DAY -> resolveDay(currentState.insightsDate.trim())
            InsightsMode.WEEK -> resolveWeek(currentState.insightsWeek.trim())
            InsightsMode.MONTH -> resolveMonth(currentState.insightsMonth.trim())
            InsightsMode.YEAR -> resolveYear(currentState.insightsYear.trim())
            InsightsMode.RANGE -> resolveRange(
                startDigits = currentState.insightsRangeStartDate.trim(),
                endDigits = currentState.insightsRangeEndDate.trim()
            )
            InsightsMode.RECENT -> resolveRecent(currentState.insightsRecentDays.trim())
        }
    }

    private fun resolveDay(insightsDateDigits: String): InsightsTemporalSelectionResolveResult {
        val validationError = inputValidator.validateDateDigits(insightsDateDigits)
        if (validationError != null) {
            return InsightsTemporalSelectionResolveResult.Failure(
                insightsMode = InsightsMode.DAY,
                validationError = validationError
            )
        }
        return InsightsTemporalSelectionResolveResult.Success(
            InsightsTemporalSelection.SingleDay(
                insightsMode = InsightsMode.DAY,
                date = LocalDate.parse(inputValidator.toIsoDate(insightsDateDigits))
            )
        )
    }

    private fun resolveWeek(insightsWeekDigits: String): InsightsTemporalSelectionResolveResult {
        val validationError = inputValidator.validateWeekDigits(insightsWeekDigits)
        if (validationError != null) {
            return InsightsTemporalSelectionResolveResult.Failure(
                insightsMode = InsightsMode.WEEK,
                validationError = validationError
            )
        }
        val selection = resolveIsoWeekSelection(insightsWeekDigits)
            ?: return InsightsTemporalSelectionResolveResult.Failure(
                insightsMode = InsightsMode.WEEK,
                validationError = textProvider.invalidWeekFormat()
            )
        return InsightsTemporalSelectionResolveResult.Success(
            InsightsTemporalSelection.DateRange(
                insightsMode = InsightsMode.WEEK,
                start = selection.weekStart,
                end = selection.weekEnd
            )
        )
    }

    private fun resolveMonth(insightsMonthDigits: String): InsightsTemporalSelectionResolveResult {
        val validationError = inputValidator.validateMonthDigits(insightsMonthDigits)
        if (validationError != null) {
            return InsightsTemporalSelectionResolveResult.Failure(
                insightsMode = InsightsMode.MONTH,
                validationError = validationError
            )
        }
        val month = parseYearMonthDigits(insightsMonthDigits)
            ?: return InsightsTemporalSelectionResolveResult.Failure(
                insightsMode = InsightsMode.MONTH,
                validationError = textProvider.invalidMonthFormat()
            )
        return InsightsTemporalSelectionResolveResult.Success(
            InsightsTemporalSelection.DateRange(
                insightsMode = InsightsMode.MONTH,
                start = month.atDay(1),
                end = month.atEndOfMonth()
            )
        )
    }

    private fun resolveYear(insightsYearDigits: String): InsightsTemporalSelectionResolveResult {
        val validationError = inputValidator.validateIsoYear(insightsYearDigits)
        if (validationError != null) {
            return InsightsTemporalSelectionResolveResult.Failure(
                insightsMode = InsightsMode.YEAR,
                validationError = validationError
            )
        }
        val year = insightsYearDigits.toIntOrNull()
            ?: return InsightsTemporalSelectionResolveResult.Failure(
                insightsMode = InsightsMode.YEAR,
                validationError = textProvider.invalidYearFormat()
            )
        return InsightsTemporalSelectionResolveResult.Success(
            InsightsTemporalSelection.DateRange(
                insightsMode = InsightsMode.YEAR,
                start = LocalDate.of(year, 1, 1),
                end = LocalDate.of(year, 12, 31)
            )
        )
    }

    private fun resolveRange(
        startDigits: String,
        endDigits: String
    ): InsightsTemporalSelectionResolveResult {
        if (startDigits.isBlank() || endDigits.isBlank()) {
            return InsightsTemporalSelectionResolveResult.Failure(
                insightsMode = InsightsMode.RANGE,
                validationError = textProvider.chartRangeBothRequired()
            )
        }

        val startValidationError = inputValidator.validateDateDigits(startDigits)
        if (startValidationError != null) {
            return InsightsTemporalSelectionResolveResult.Failure(
                insightsMode = InsightsMode.RANGE,
                validationError = textProvider.chartRangeStartDateInvalid()
            )
        }

        val endValidationError = inputValidator.validateDateDigits(endDigits)
        if (endValidationError != null) {
            return InsightsTemporalSelectionResolveResult.Failure(
                insightsMode = InsightsMode.RANGE,
                validationError = textProvider.chartRangeEndDateInvalid()
            )
        }

        val rangeOrderError = inputValidator.validateRangeOrder(startDigits, endDigits)
        if (rangeOrderError != null) {
            return InsightsTemporalSelectionResolveResult.Failure(
                insightsMode = InsightsMode.RANGE,
                validationError = textProvider.chartRangeOrderInvalid()
            )
        }

        return InsightsTemporalSelectionResolveResult.Success(
            InsightsTemporalSelection.DateRange(
                insightsMode = InsightsMode.RANGE,
                start = LocalDate.parse(inputValidator.toIsoDate(startDigits)),
                end = LocalDate.parse(inputValidator.toIsoDate(endDigits))
            )
        )
    }

    private fun resolveRecent(insightsRecentDays: String): InsightsTemporalSelectionResolveResult {
        val validationError = inputValidator.validateRecentDays(insightsRecentDays)
        if (validationError != null) {
            return InsightsTemporalSelectionResolveResult.Failure(
                insightsMode = InsightsMode.RECENT,
                validationError = validationError
            )
        }
        return InsightsTemporalSelectionResolveResult.Success(
            InsightsTemporalSelection.RecentDays(
                insightsMode = InsightsMode.RECENT,
                days = insightsRecentDays.toInt()
            )
        )
    }

    private fun parseYearMonthDigits(insightsMonthDigits: String): YearMonth? {
        if (insightsMonthDigits.length != 6) {
            return null
        }
        val year = insightsMonthDigits.take(4).toIntOrNull() ?: return null
        val month = insightsMonthDigits.takeLast(2).toIntOrNull() ?: return null
        return try {
            YearMonth.of(year, month)
        } catch (_: RuntimeException) {
            null
        }
    }
}
