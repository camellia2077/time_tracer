package com.example.tracer

import java.time.Clock
import java.time.Instant
import java.time.ZoneId
import org.junit.Assert.assertEquals
import org.junit.Test

class TxtEditorInitialDayMarkerTest {
    private val clock = Clock.fixed(
        Instant.parse("2026-09-06T17:00:00Z"),
        ZoneId.of("Asia/Shanghai")
    )

    @Test
    fun startsAtTodayInDeviceTimeZoneBeforeLogicalDayCutoff() {
        assertEquals("0907", initialDayMarkerForSelectedMonth(clock, "2026-09"))
    }

    @Test
    fun newSessionDoesNotRestorePreviouslySelectedDay() {
        val previousSession = TxtEditorSessionController(
            TxtEditorSessionState(dayMarkerInput = initialDayMarkerForSelectedMonth(clock, "2026-09"))
        )
        previousSession.updateDayMarkerInput("0903")
        val reopenedSession = TxtEditorSessionController(
            TxtEditorSessionState(dayMarkerInput = initialDayMarkerForSelectedMonth(clock, "2026-09"))
        )
        assertEquals("0903", previousSession.state.dayMarkerInput)
        assertEquals("0907", reopenedSession.state.dayMarkerInput)
    }

    @Test
    fun otherMonthsDeferToCoreDefault() {
        assertEquals("", initialDayMarkerForSelectedMonth(clock, "2026-08"))
        assertEquals("", initialDayMarkerForSelectedMonth(clock, "2025-09"))
        assertEquals("", initialDayMarkerForSelectedMonth(clock, ""))
    }

    @Test
    fun newSessionUsesNewDateAfterMidnight() {
        val nextDay = Clock.offset(clock, java.time.Duration.ofDays(1))
        assertEquals("0908", initialDayMarkerForSelectedMonth(nextDay, "2026-09"))
    }
}
