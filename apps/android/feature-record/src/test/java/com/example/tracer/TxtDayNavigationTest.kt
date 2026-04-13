package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertNull
import org.junit.Test
import java.time.LocalDate

class TxtDayNavigationTest {
    @Test
    fun navigateToDay_sameMonthClearsPendingWithoutOpeningMonth() {
        var pendingDay: LocalDate? = LocalDate.of(2026, 4, 1)
        var dayMarker = ""
        var openedMonth = ""

        navigateToDay(
            targetDay = LocalDate.of(2026, 4, 12),
            selectedMonth = "2026-04",
            onPendingDayChange = { pendingDay = it },
            onDayMarkerInputChange = { dayMarker = it },
            onOpenMonth = { openedMonth = it }
        )

        assertNull(pendingDay)
        assertEquals("0412", dayMarker)
        assertEquals("", openedMonth)
    }

    @Test
    fun navigateToDay_crossMonthPreservesPendingAndOpensMonth() {
        var pendingDay: LocalDate? = null
        var dayMarker = ""
        var openedMonth = ""
        val targetDay = LocalDate.of(2026, 5, 1)

        navigateToDay(
            targetDay = targetDay,
            selectedMonth = "2026-04",
            onPendingDayChange = { pendingDay = it },
            onDayMarkerInputChange = { dayMarker = it },
            onOpenMonth = { openedMonth = it }
        )

        assertEquals(targetDay, pendingDay)
        assertEquals("0501", dayMarker)
        assertEquals("2026-05", openedMonth)
    }
}
