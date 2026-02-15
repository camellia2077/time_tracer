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

internal class QueryPeriodArgumentResolver {
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
            return "$subjectLabel argument is required for period ${period.wireValue}."
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
                    "Invalid range argument. Use start|end (example: 2026-02-01|2026-02-15)."
                } else {
                    val start = parsePeriodDate(parts[0])
                    val end = parsePeriodDate(parts[1])
                    if (start == null || end == null) {
                        "Invalid range date value. Use YYYY-MM-DD (or YYYYMMDD)."
                    } else if (start.after(end)) {
                        "$subjectLabel range invalid. Start date must be <= end date."
                    } else {
                        null
                    }
                }
            }
        }
    }

    private fun validateDateDigits(value: String): String? {
        if (!Regex("""^\d{8}$""").matches(value)) {
            return "Invalid day format. Use digits YYYYMMDD (example: 20260214)."
        }
        return try {
            strictBasicIsoDateFormatter.parse(value)
            null
        } catch (_: Exception) {
            "Invalid date value: $value."
        }
    }

    private fun validateMonthDigits(value: String): String? {
        if (!Regex("""^\d{6}$""").matches(value)) {
            return "Invalid month format. Use digits YYYYMM (example: 202602)."
        }
        return try {
            strictMonthDigitsFormatter.parse(value)
            null
        } catch (_: Exception) {
            "Invalid month value: $value."
        }
    }

    private fun validateIsoYear(value: String): String? {
        if (!Regex("""^\d{4}$""").matches(value)) {
            return "Invalid year format. Use ISO year: YYYY (example: 2026)."
        }
        return try {
            strictIsoYearFormatter.parse(value)
            null
        } catch (_: Exception) {
            "Invalid ISO year value: $value."
        }
    }

    private fun validateWeekDigits(value: String): String? {
        val match = Regex("""^(\d{4})(\d{2})$""").matchEntire(value)
            ?: return "Invalid week format. Use digits YYYYWW (example: 202607)."

        val year = match.groupValues[1].toIntOrNull()
            ?: return "Invalid week year: ${match.groupValues[1]}."
        val week = match.groupValues[2].toIntOrNull()
            ?: return "Invalid week number: ${match.groupValues[2]}."

        val cal = Calendar.getInstance(Locale.US).apply {
            set(Calendar.YEAR, year)
            set(Calendar.MONTH, Calendar.DECEMBER)
            set(Calendar.DAY_OF_MONTH, 28)
            firstDayOfWeek = Calendar.MONDAY
            minimalDaysInFirstWeek = 4
        }
        val maxIsoWeek = cal.getActualMaximum(Calendar.WEEK_OF_YEAR)
        if (week !in 1..maxIsoWeek) {
            return "Invalid week value: $value. $year supports week 01 to $maxIsoWeek."
        }
        return null
    }

    private fun validateRecentDays(value: String): String? {
        if (value.isBlank()) {
            return "Recent days is required and must be a number."
        }
        val days = value.toIntOrNull()
            ?: return "Recent days must be numeric."
        if (days <= 0) {
            return "Recent days must be greater than 0."
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
