package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertNull
import org.junit.Test
import java.time.LocalDate
import java.time.YearMonth

class InsightsDayPickerStateTest {
    @Test
    fun resolveInsightsDayPickerState_returnsDisplayMonthWhenYearMonthIsValid() {
        val state = resolveInsightsDayPickerState(
            year = "2026",
            month = "04",
            day = "12"
        )

        assertNotNull(state)
        assertEquals(YearMonth.of(2026, 4), state?.displayMonth)
        assertEquals(LocalDate.of(2026, 4, 12), state?.selectedDate)
    }

    @Test
    fun resolveInsightsDayPickerState_returnsNullWhenYearMonthIsIncomplete() {
        val state = resolveInsightsDayPickerState(
            year = "2026",
            month = "",
            day = "12"
        )

        assertNull(state)
    }

    @Test
    fun resolveInsightsDayPickerState_keepsPickerEnabledWhenDayIsInvalid() {
        val state = resolveInsightsDayPickerState(
            year = "2026",
            month = "04",
            day = "99"
        )

        assertNotNull(state)
        assertEquals(YearMonth.of(2026, 4), state?.displayMonth)
        assertNull(state?.selectedDate)
    }

    @Test
    fun mergePickedInsightsDay_onlyReplacesDayDigits() {
        val merged = mergePickedInsightsDay(
            year = "2026",
            month = "04",
            pickedDate = LocalDate.of(2026, 4, 27)
        )

        assertEquals("20260427", merged)
    }
}
