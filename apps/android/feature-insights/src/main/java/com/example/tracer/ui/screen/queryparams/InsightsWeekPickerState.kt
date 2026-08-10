package com.example.tracer

import com.example.tracer.ui.components.CalendarWeekRow
import com.example.tracer.ui.components.buildMonthWeekRows
import com.example.tracer.ui.components.formatWeekRangeText
import com.example.tracer.ui.components.splitYearMonthDigits
import java.time.LocalDate
import java.time.YearMonth
import java.time.temporal.WeekFields

internal data class InsightsWeekPickerState(
    val displayMonth: YearMonth,
    val selectedWeekRow: CalendarWeekRow?,
    val selectedWeekLabel: String?
)

internal fun resolveInsightsWeekPickerState(
    insightsMonthDigits: String,
    insightsWeekDigits: String
): InsightsWeekPickerState? {
    val displayMonth = parseInsightsMonthDigits(insightsMonthDigits) ?: return null
    val selectedWeekRow = buildMonthWeekRows(
        displayMonth = displayMonth,
        selectedWeekDigits = insightsWeekDigits.takeIf(::isWeekDigitsFormat)
    ).firstOrNull { it.isSelected }
    val selectedWeekLabel = resolveIsoWeekSelection(insightsWeekDigits)
        ?.let(::formatInsightsWeekSelectionLabel)
    return InsightsWeekPickerState(
        displayMonth = displayMonth,
        selectedWeekRow = selectedWeekRow,
        selectedWeekLabel = selectedWeekLabel
    )
}

internal fun mergePickedInsightsWeek(pickedRow: CalendarWeekRow): String = pickedRow.isoWeekDigits

internal fun formatInsightsWeekSelectionLabel(selection: InsightsIsoWeekSelection): String =
    "${formatWeekRangeText(selection.weekStart, selection.weekEnd)} · " +
        "W${selection.isoWeekDigits.takeLast(2)}"

internal fun resolveIsoWeekSelection(insightsWeekDigits: String): InsightsIsoWeekSelection? {
    if (!isWeekDigitsFormat(insightsWeekDigits)) {
        return null
    }
    val year = insightsWeekDigits.take(4).toIntOrNull() ?: return null
    val week = insightsWeekDigits.takeLast(2).toIntOrNull() ?: return null
    val maxIsoWeek = LocalDate.of(year, 12, 28).get(WeekFields.ISO.weekOfWeekBasedYear())
    if (week !in 1..maxIsoWeek) {
        return null
    }

    // The UI groups rows by display month, but the actual insights target still
    // resolves to a canonical ISO week using that row's Monday as the anchor.
    val weekFields = WeekFields.ISO
    val weekStart = LocalDate.of(year, 1, 4)
        .with(weekFields.weekBasedYear(), year.toLong())
        .with(weekFields.weekOfWeekBasedYear(), week.toLong())
        .with(weekFields.dayOfWeek(), 1L)
    return InsightsIsoWeekSelection(
        isoWeekDigits = insightsWeekDigits,
        weekStart = weekStart,
        weekEnd = weekStart.plusDays(6)
    )
}

internal data class InsightsIsoWeekSelection(
    val isoWeekDigits: String,
    val weekStart: LocalDate,
    val weekEnd: LocalDate
)

private fun parseInsightsMonthDigits(insightsMonthDigits: String): YearMonth? {
    val (year, month) = splitYearMonthDigits(insightsMonthDigits)
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
