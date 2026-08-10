package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertNull
import org.junit.Test
import java.time.Clock
import java.time.Instant
import java.time.LocalDate
import java.time.YearMonth
import java.time.ZoneId

class InsightsWeekPickerStateTest {
    @Test
    fun initialQueryInsightsUiState_usesIsoWeekForDefaultWeek() {
        val state = initialQueryInsightsUiState(
            Clock.fixed(
                Instant.parse("2026-07-12T12:00:00Z"),
                ZoneId.of("UTC")
            )
        )

        assertEquals("202628", state.insightsWeek)
    }

    @Test
    fun resolveIsoWeekSelection_returnsWeekRangeForValidDigits() {
        val selection = resolveIsoWeekSelection("202615")

        assertNotNull(selection)
        assertEquals(LocalDate.of(2026, 4, 6), selection?.weekStart)
        assertEquals(LocalDate.of(2026, 4, 12), selection?.weekEnd)
    }

    @Test
    fun resolveInsightsWeekPickerState_selectsVisibleWeekWhenMonthMatches() {
        val state = resolveInsightsWeekPickerState(
            insightsMonthDigits = "202604",
            insightsWeekDigits = "202615"
        )

        assertNotNull(state)
        assertEquals(YearMonth.of(2026, 4), state?.displayMonth)
        assertEquals("202615", state?.selectedWeekRow?.isoWeekDigits)
        assertEquals("04-06 ~ 04-12 · W15", state?.selectedWeekLabel)
    }

    @Test
    fun resolveInsightsWeekPickerState_keepsLabelWithoutPreselectWhenWeekOutsideVisibleMonth() {
        val state = resolveInsightsWeekPickerState(
            insightsMonthDigits = "202605",
            insightsWeekDigits = "202615"
        )

        assertNotNull(state)
        assertEquals(YearMonth.of(2026, 5), state?.displayMonth)
        assertNull(state?.selectedWeekRow)
        assertEquals("04-06 ~ 04-12 · W15", state?.selectedWeekLabel)
    }

    @Test
    fun mergePickedInsightsWeek_returnsIsoWeekDigits() {
        val row = com.example.tracer.ui.components.CalendarWeekRow(
            weekStart = LocalDate.of(2026, 4, 27),
            weekEnd = LocalDate.of(2026, 5, 3),
            isoWeekDigits = "202618",
            days = emptyList(),
            isSelected = false
        )

        assertEquals("202618", mergePickedInsightsWeek(row))
    }
}
