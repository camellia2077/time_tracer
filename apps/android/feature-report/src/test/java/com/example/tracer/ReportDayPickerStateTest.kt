package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertNull
import org.junit.Test
import java.time.LocalDate
import java.time.YearMonth

class ReportDayPickerStateTest {
    @Test
    fun resolveReportDayPickerState_returnsDisplayMonthWhenYearMonthIsValid() {
        val state = resolveReportDayPickerState(
            year = "2026",
            month = "04",
            day = "12"
        )

        assertNotNull(state)
        assertEquals(YearMonth.of(2026, 4), state?.displayMonth)
        assertEquals(LocalDate.of(2026, 4, 12), state?.selectedDate)
    }

    @Test
    fun resolveReportDayPickerState_returnsNullWhenYearMonthIsIncomplete() {
        val state = resolveReportDayPickerState(
            year = "2026",
            month = "",
            day = "12"
        )

        assertNull(state)
    }

    @Test
    fun resolveReportDayPickerState_keepsPickerEnabledWhenDayIsInvalid() {
        val state = resolveReportDayPickerState(
            year = "2026",
            month = "04",
            day = "99"
        )

        assertNotNull(state)
        assertEquals(YearMonth.of(2026, 4), state?.displayMonth)
        assertNull(state?.selectedDate)
    }

    @Test
    fun mergePickedReportDay_onlyReplacesDayDigits() {
        val merged = mergePickedReportDay(
            year = "2026",
            month = "04",
            pickedDate = LocalDate.of(2026, 4, 27)
        )

        assertEquals("20260427", merged)
    }
}
