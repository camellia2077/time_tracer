package com.example.tracer.ui.components

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import java.time.DayOfWeek
import java.time.LocalDate
import java.time.YearMonth

class CalendarDatePickerSheetTest {
    @Test
    fun buildMonthCalendarGrid_mondayStartAddsLeadingDays() {
        val grid = buildMonthCalendarGrid(
            displayMonth = YearMonth.of(2026, 4),
            selectedDate = LocalDate.of(2026, 4, 15),
            firstDayOfWeek = DayOfWeek.MONDAY
        )

        assertEquals(YearMonth.of(2026, 4), grid.month)
        assertEquals(LocalDate.of(2026, 3, 30), grid.cells.first().date)
        assertFalse(grid.cells.first().isInCurrentMonth)
        assertEquals(35, grid.cells.size)
        assertEquals(0, grid.cells.size % 7)
    }

    @Test
    fun buildMonthCalendarGrid_addsTrailingDaysForCompleteWeeks() {
        val grid = buildMonthCalendarGrid(
            displayMonth = YearMonth.of(2026, 8),
            selectedDate = LocalDate.of(2026, 8, 15),
            firstDayOfWeek = DayOfWeek.MONDAY
        )

        assertEquals(LocalDate.of(2026, 7, 27), grid.cells.first().date)
        assertEquals(LocalDate.of(2026, 9, 6), grid.cells.last().date)
        assertEquals(42, grid.cells.size)
        assertFalse(grid.cells.last().isInCurrentMonth)
    }

    @Test
    fun buildMonthCalendarGrid_marksSelectedCurrentMonthDay() {
        val selectedDate = LocalDate.of(2026, 4, 15)

        val selectedCell = buildMonthCalendarGrid(
            displayMonth = YearMonth.of(2026, 4),
            selectedDate = selectedDate,
            firstDayOfWeek = DayOfWeek.MONDAY
        ).cells.single { it.isSelected }

        assertEquals(selectedDate, selectedCell.date)
        assertTrue(selectedCell.isInCurrentMonth)
        assertTrue(selectedCell.isEnabled)
    }

    @Test
    fun buildMonthCalendarGrid_disablesAdjacentMonthCellsWhenConfigured() {
        val grid = buildMonthCalendarGrid(
            displayMonth = YearMonth.of(2026, 4),
            selectedDate = null,
            allowAdjacentMonthSelection = false,
            firstDayOfWeek = DayOfWeek.MONDAY
        )

        val adjacentCell = grid.cells.first()
        val currentMonthCell = grid.cells.first { it.isInCurrentMonth }

        assertEquals(LocalDate.of(2026, 3, 30), adjacentCell.date)
        assertFalse(adjacentCell.isEnabled)
        assertTrue(currentMonthCell.isEnabled)
    }

    @Test
    fun buildMonthWeekRows_coversVisibleWeeksForDisplayMonth() {
        val rows = buildMonthWeekRows(
            displayMonth = YearMonth.of(2026, 4),
            firstDayOfWeek = DayOfWeek.MONDAY
        )

        assertEquals(5, rows.size)
        assertEquals(LocalDate.of(2026, 3, 30), rows.first().weekStart)
        assertEquals(LocalDate.of(2026, 5, 3), rows.last().weekEnd)
    }

    @Test
    fun buildMonthWeekRows_usesIsoWeekFromRowMonday() {
        val firstRow = buildMonthWeekRows(
            displayMonth = YearMonth.of(2026, 4),
            firstDayOfWeek = DayOfWeek.MONDAY
        ).first()

        assertEquals("202614", firstRow.isoWeekDigits)
    }

    @Test
    fun buildMonthWeekRows_marksSelectedVisibleWeek() {
        val selectedRow = buildMonthWeekRows(
            displayMonth = YearMonth.of(2026, 4),
            selectedWeekDigits = "202615",
            firstDayOfWeek = DayOfWeek.MONDAY
        ).single { it.isSelected }

        assertEquals(LocalDate.of(2026, 4, 6), selectedRow.weekStart)
        assertEquals(LocalDate.of(2026, 4, 12), selectedRow.weekEnd)
    }
}
