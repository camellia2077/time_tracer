package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertNull
import org.junit.Test
import java.time.LocalDate
import java.time.YearMonth

class ReportWeekPickerStateTest {
    @Test
    fun resolveIsoWeekSelection_returnsWeekRangeForValidDigits() {
        val selection = resolveIsoWeekSelection("202615")

        assertNotNull(selection)
        assertEquals(LocalDate.of(2026, 4, 6), selection?.weekStart)
        assertEquals(LocalDate.of(2026, 4, 12), selection?.weekEnd)
    }

    @Test
    fun resolveReportWeekPickerState_selectsVisibleWeekWhenMonthMatches() {
        val state = resolveReportWeekPickerState(
            reportMonthDigits = "202604",
            reportWeekDigits = "202615"
        )

        assertNotNull(state)
        assertEquals(YearMonth.of(2026, 4), state?.displayMonth)
        assertEquals("202615", state?.selectedWeekRow?.isoWeekDigits)
        assertEquals("04-06 ~ 04-12 · W15", state?.selectedWeekLabel)
    }

    @Test
    fun resolveReportWeekPickerState_keepsLabelWithoutPreselectWhenWeekOutsideVisibleMonth() {
        val state = resolveReportWeekPickerState(
            reportMonthDigits = "202605",
            reportWeekDigits = "202615"
        )

        assertNotNull(state)
        assertEquals(YearMonth.of(2026, 5), state?.displayMonth)
        assertNull(state?.selectedWeekRow)
        assertEquals("04-06 ~ 04-12 · W15", state?.selectedWeekLabel)
    }

    @Test
    fun mergePickedReportWeek_returnsIsoWeekDigits() {
        val row = com.example.tracer.ui.components.CalendarWeekRow(
            weekStart = LocalDate.of(2026, 4, 27),
            weekEnd = LocalDate.of(2026, 5, 3),
            isoWeekDigits = "202618",
            days = emptyList(),
            isSelected = false
        )

        assertEquals("202618", mergePickedReportWeek(row))
    }
}
