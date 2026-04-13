package com.example.tracer

import com.example.tracer.ui.components.CalendarWeekRow
import com.example.tracer.ui.components.buildMonthWeekRows
import com.example.tracer.ui.components.formatWeekRangeText
import com.example.tracer.ui.components.splitYearMonthDigits
import java.time.LocalDate
import java.time.YearMonth
import java.time.temporal.WeekFields

internal data class ReportWeekPickerState(
    val displayMonth: YearMonth,
    val selectedWeekRow: CalendarWeekRow?,
    val selectedWeekLabel: String?
)

internal fun resolveReportWeekPickerState(
    reportMonthDigits: String,
    reportWeekDigits: String
): ReportWeekPickerState? {
    val displayMonth = parseReportMonthDigits(reportMonthDigits) ?: return null
    val selectedWeekRow = buildMonthWeekRows(
        displayMonth = displayMonth,
        selectedWeekDigits = reportWeekDigits.takeIf(::isWeekDigitsFormat)
    ).firstOrNull { it.isSelected }
    val selectedWeekLabel = resolveIsoWeekSelection(reportWeekDigits)
        ?.let(::formatReportWeekSelectionLabel)
    return ReportWeekPickerState(
        displayMonth = displayMonth,
        selectedWeekRow = selectedWeekRow,
        selectedWeekLabel = selectedWeekLabel
    )
}

internal fun mergePickedReportWeek(pickedRow: CalendarWeekRow): String = pickedRow.isoWeekDigits

internal fun formatReportWeekSelectionLabel(selection: ReportIsoWeekSelection): String =
    "${formatWeekRangeText(selection.weekStart, selection.weekEnd)} · " +
        "W${selection.isoWeekDigits.takeLast(2)}"

internal fun resolveIsoWeekSelection(reportWeekDigits: String): ReportIsoWeekSelection? {
    if (!isWeekDigitsFormat(reportWeekDigits)) {
        return null
    }
    val year = reportWeekDigits.take(4).toIntOrNull() ?: return null
    val week = reportWeekDigits.takeLast(2).toIntOrNull() ?: return null
    val maxIsoWeek = LocalDate.of(year, 12, 28).get(WeekFields.ISO.weekOfWeekBasedYear())
    if (week !in 1..maxIsoWeek) {
        return null
    }

    // The UI groups rows by display month, but the actual report target still
    // resolves to a canonical ISO week using that row's Monday as the anchor.
    val weekFields = WeekFields.ISO
    val weekStart = LocalDate.of(year, 1, 4)
        .with(weekFields.weekBasedYear(), year.toLong())
        .with(weekFields.weekOfWeekBasedYear(), week.toLong())
        .with(weekFields.dayOfWeek(), 1L)
    return ReportIsoWeekSelection(
        isoWeekDigits = reportWeekDigits,
        weekStart = weekStart,
        weekEnd = weekStart.plusDays(6)
    )
}

internal data class ReportIsoWeekSelection(
    val isoWeekDigits: String,
    val weekStart: LocalDate,
    val weekEnd: LocalDate
)

private fun parseReportMonthDigits(reportMonthDigits: String): YearMonth? {
    val (year, month) = splitYearMonthDigits(reportMonthDigits)
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

private fun isWeekDigitsFormat(value: String): Boolean =
    Regex("""^\d{6}$""").matches(value)
