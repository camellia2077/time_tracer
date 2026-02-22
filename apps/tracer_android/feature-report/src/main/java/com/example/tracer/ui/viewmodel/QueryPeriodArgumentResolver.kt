package com.example.tracer

import java.text.SimpleDateFormat
import java.util.Calendar
import java.util.Date
import java.util.Locale

internal data class QueryPeriodSource(
    val dayDigits: String,
    val monthDigits: String,
    val yearDigits: String,
    val weekDigits: String,
    val rangeStartDigits: String,
    val rangeEndDigits: String,
    val recentDays: String
)

internal sealed interface QueryPeriodResolveResult {
    data class Success(val argument: String) : QueryPeriodResolveResult
    data class Failure(val message: String) : QueryPeriodResolveResult
}

internal class QueryPeriodArgumentResolver(
    private val textProvider: QueryReportTextProvider = DefaultQueryReportTextProvider
) {
    private val strictBasicIsoDateFormatter: SimpleDateFormat =
        SimpleDateFormat("yyyyMMdd", Locale.US).apply { isLenient = false }
    private val strictMonthDigitsFormatter: SimpleDateFormat =
        SimpleDateFormat("yyyyMM", Locale.US).apply { isLenient = false }
    private val strictIsoYearFormatter: SimpleDateFormat =
        SimpleDateFormat("yyyy", Locale.US).apply { isLenient = false }

    fun resolveAndValidate(
        period: DataTreePeriod,
        source: QueryPeriodSource,
        subjectLabel: String
    ): QueryPeriodResolveResult {
        val periodArgument = resolvePeriodArgument(period, source).trim()
        val validationError = validatePeriodArgument(period, periodArgument, subjectLabel)
        return if (validationError != null) {
            QueryPeriodResolveResult.Failure(validationError)
        } else {
            QueryPeriodResolveResult.Success(periodArgument)
        }
    }

    private fun resolvePeriodArgument(period: DataTreePeriod, source: QueryPeriodSource): String {
        return when (period) {
            DataTreePeriod.DAY -> source.dayDigits.trim()
            DataTreePeriod.MONTH -> source.monthDigits.trim()
            DataTreePeriod.YEAR -> source.yearDigits.trim()
            DataTreePeriod.WEEK -> normalizeWeekPeriodArgument(source.weekDigits)
            DataTreePeriod.RECENT -> source.recentDays.trim()
            DataTreePeriod.RANGE -> {
                val start = normalizeDatePeriodArgument(source.rangeStartDigits)
                val end = normalizeDatePeriodArgument(source.rangeEndDigits)
                if (start.isBlank() || end.isBlank()) {
                    ""
                } else {
                    "$start|$end"
                }
            }
        }
    }

    private fun validatePeriodArgument(
        period: DataTreePeriod,
        periodArgument: String,
        subjectLabel: String
    ): String? {
        if (periodArgument.isBlank()) {
            return textProvider.periodArgumentRequired(
                subjectLabel = subjectLabel,
                periodLabel = textProvider.periodLabel(period)
            )
        }

        return when (period) {
            DataTreePeriod.DAY -> validateDateDigits(periodArgument)
            DataTreePeriod.MONTH -> validateMonthDigits(periodArgument)
            DataTreePeriod.YEAR -> validateIsoYear(periodArgument)
            DataTreePeriod.WEEK -> {
                val normalizedWeekDigits = periodArgument.replace("-W", "").replace("-w", "")
                validateWeekDigits(normalizedWeekDigits)
            }
            DataTreePeriod.RECENT -> validateRecentDays(periodArgument)
            DataTreePeriod.RANGE -> {
                val parts = periodArgument.split("|")
                if (parts.size != 2) {
                    textProvider.invalidRangeArgumentFormat()
                } else {
                    val start = parsePeriodDate(parts[0])
                    val end = parsePeriodDate(parts[1])
                    if (start == null || end == null) {
                        textProvider.invalidRangeDateValue()
                    } else if (start.after(end)) {
                        textProvider.subjectRangeInvalid(subjectLabel)
                    } else {
                        null
                    }
                }
            }
        }
    }

    private fun validateDateDigits(value: String): String? {
        if (!Regex("""^\d{8}$""").matches(value)) {
            return textProvider.invalidDayFormat()
        }
        return try {
            strictBasicIsoDateFormatter.parse(value)
            null
        } catch (_: Exception) {
            textProvider.invalidDateValue(value)
        }
    }

    private fun validateMonthDigits(value: String): String? {
        if (!Regex("""^\d{6}$""").matches(value)) {
            return textProvider.invalidMonthFormat()
        }
        return try {
            strictMonthDigitsFormatter.parse(value)
            null
        } catch (_: Exception) {
            textProvider.invalidMonthValue(value)
        }
    }

    private fun validateIsoYear(value: String): String? {
        if (!Regex("""^\d{4}$""").matches(value)) {
            return textProvider.invalidYearFormat()
        }
        return try {
            strictIsoYearFormatter.parse(value)
            null
        } catch (_: Exception) {
            textProvider.invalidIsoYearValue(value)
        }
    }

    private fun validateWeekDigits(value: String): String? {
        val match = Regex("""^(\d{4})(\d{2})$""").matchEntire(value)
            ?: return textProvider.invalidWeekFormat()

        val year = match.groupValues[1].toIntOrNull()
            ?: return textProvider.invalidWeekYear(match.groupValues[1])
        val week = match.groupValues[2].toIntOrNull()
            ?: return textProvider.invalidWeekNumber(match.groupValues[2])

        val cal = Calendar.getInstance(Locale.US).apply {
            set(Calendar.YEAR, year)
            set(Calendar.MONTH, Calendar.DECEMBER)
            set(Calendar.DAY_OF_MONTH, 28)
            firstDayOfWeek = Calendar.MONDAY
            minimalDaysInFirstWeek = 4
        }
        val maxIsoWeek = cal.getActualMaximum(Calendar.WEEK_OF_YEAR)
        if (week !in 1..maxIsoWeek) {
            return textProvider.invalidWeekValue(value, year, maxIsoWeek)
        }
        return null
    }

    private fun validateRecentDays(value: String): String? {
        if (value.isBlank()) {
            return textProvider.recentDaysRequired()
        }
        val days = value.toIntOrNull()
            ?: return textProvider.recentDaysMustBeNumeric()
        if (days <= 0) {
            return textProvider.recentDaysMustBeGreaterThanZero()
        }
        return null
    }

    private fun parsePeriodDate(dateText: String): Date? {
        val trimmed = dateText.trim()
        if (trimmed.isEmpty()) {
            return null
        }
        return try {
            if (Regex("""^\d{8}$""").matches(trimmed)) {
                strictBasicIsoDateFormatter.parse(trimmed)
            } else {
                SimpleDateFormat("yyyy-MM-dd", Locale.US).apply { isLenient = false }.parse(trimmed)
            }
        } catch (_: Exception) {
            null
        }
    }

    private fun normalizeWeekPeriodArgument(raw: String): String {
        val digits = raw.filter { it.isDigit() }.take(6)
        if (digits.length == 6) {
            return "${digits.take(4)}-W${digits.drop(4)}"
        }
        return raw.trim()
    }

    private fun normalizeDatePeriodArgument(raw: String): String {
        val digits = raw.filter { it.isDigit() }.take(8)
        if (digits.length == 8) {
            val year = digits.take(4)
            val month = digits.drop(4).take(2)
            val day = digits.drop(6).take(2)
            return "$year-$month-$day"
        }
        return raw.trim()
    }
}
