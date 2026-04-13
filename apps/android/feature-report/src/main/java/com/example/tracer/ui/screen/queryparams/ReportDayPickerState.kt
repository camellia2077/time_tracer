package com.example.tracer

import com.example.tracer.ui.components.mergeDateDigits
import java.time.LocalDate
import java.time.YearMonth

internal data class ReportDayPickerState(
    val displayMonth: YearMonth,
    val selectedDate: LocalDate?
)

internal fun resolveReportDayPickerState(
    year: String,
    month: String,
    day: String
): ReportDayPickerState? {
    val displayMonth = parseReportDisplayMonth(year = year, month = month) ?: return null
    val selectedDate = parseReportSelectedDate(
        displayMonth = displayMonth,
        day = day
    )
    return ReportDayPickerState(
        displayMonth = displayMonth,
        selectedDate = selectedDate
    )
}

internal fun mergePickedReportDay(
    year: String,
    month: String,
    pickedDate: LocalDate
): String = mergeDateDigits(
    year = year,
    month = month,
    day = pickedDate.dayOfMonth.toString().padStart(2, '0')
)

private fun parseReportDisplayMonth(
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

private fun parseReportSelectedDate(
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
