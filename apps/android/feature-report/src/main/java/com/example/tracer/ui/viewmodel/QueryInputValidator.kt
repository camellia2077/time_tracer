package com.example.tracer

import java.text.SimpleDateFormat
import java.util.Calendar
import java.util.Locale

internal class QueryInputValidator(
    private val textProvider: QueryReportTextProvider = DefaultQueryReportTextProvider
) {
    private val strictBasicIsoDateFormatter: SimpleDateFormat =
        SimpleDateFormat("yyyyMMdd", Locale.US).apply { isLenient = false }
    private val strictMonthDigitsFormatter: SimpleDateFormat =
        SimpleDateFormat("yyyyMM", Locale.US).apply { isLenient = false }
    private val strictIsoYearFormatter: SimpleDateFormat =
        SimpleDateFormat("yyyy", Locale.US).apply { isLenient = false }

    fun toIsoDate(value: String): String {
        val date = strictBasicIsoDateFormatter.parse(value)
        return SimpleDateFormat("yyyy-MM-dd", Locale.US).format(date!!)
    }

    fun validateDateDigits(value: String): String? {
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

    fun toIsoMonth(value: String): String {
        val date = strictMonthDigitsFormatter.parse(value)
        return SimpleDateFormat("yyyy-MM", Locale.US).format(date!!)
    }

    fun validateMonthDigits(value: String): String? {
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

    fun validateIsoYear(value: String): String? {
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

    fun toIsoWeek(value: String): String {
        val year = value.substring(0, 4).toInt()
        val week = value.substring(4, 6).toInt()
        return String.format(Locale.US, "%04d-W%02d", year, week)
    }

    fun validateWeekDigits(value: String): String? {
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

    fun validateRangeOrder(startDateDigits: String, endDateDigits: String): String? {
        val start = strictBasicIsoDateFormatter.parse(startDateDigits)
        val end = strictBasicIsoDateFormatter.parse(endDateDigits)
        if (start != null && end != null && start.after(end)) {
            return textProvider.invalidRangeOrder()
        }
        return null
    }

    fun validateRecentDays(value: String): String? {
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
}
