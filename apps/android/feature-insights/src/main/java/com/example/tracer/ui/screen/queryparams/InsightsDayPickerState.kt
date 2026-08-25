package com.example.tracer

import com.example.tracer.ui.components.mergeDateDigits
import java.time.LocalDate
import java.time.YearMonth

internal data class InsightsDayPickerState(
    val displayMonth: YearMonth,
    val selectedDate: LocalDate?
)

internal fun resolveInsightsDayPickerState(
    insightsDate: String
): InsightsDayPickerState? {
    if (insightsDate.length != 8) {
        return null
    }
    val year = insightsDate.substring(0, 4)
    val month = insightsDate.substring(4, 6)
    val day = insightsDate.substring(6, 8)
    val displayMonth = parseInsightsDisplayMonth(year = year, month = month) ?: return null
    val selectedDate = parseInsightsSelectedDate(
        displayMonth = displayMonth,
        day = day
    )
    return InsightsDayPickerState(
        displayMonth = displayMonth,
        selectedDate = selectedDate
    )
}

internal fun mergePickedInsightsDay(
    year: String,
    month: String,
    pickedDate: LocalDate
): String = mergeDateDigits(
    year = year,
    month = month,
    day = pickedDate.dayOfMonth.toString().padStart(2, '0')
)

private fun parseInsightsDisplayMonth(
    year: String,
    month: String
): YearMonth? {
    if (year.length != 4 || month.length != 2) {
        return null
    }
    val parsedYear = year.toIntOrNull() ?: return null
    val parsedMonth = month.toIntOrNull() ?: return null
    return try {
        YearMonth.of(parsedYear, parsedMonth)
    } catch (_: RuntimeException) {
        null
    }
}

private fun parseInsightsSelectedDate(
    displayMonth: YearMonth,
    day: String
): LocalDate? {
    if (day.length != 2) {
        return null
    }
    val parsedDay = day.toIntOrNull() ?: return null
    return try {
        displayMonth.atDay(parsedDay)
    } catch (_: RuntimeException) {
        null
    }
}
