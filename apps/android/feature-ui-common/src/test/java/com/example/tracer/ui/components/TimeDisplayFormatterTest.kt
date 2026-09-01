package com.example.tracer.ui.components

import org.junit.Assert.assertEquals
import org.junit.Test

class TimeDisplayFormatterTest {
    @Test
    fun formatDisplayClockTime_convertsMidnightNoonAndAfternoonToTwelveHour() {
        assertEquals("12:00 AM", formatDisplayClockTime("00:00", use12Hour = true))
        assertEquals("12:00 PM", formatDisplayClockTime("12:00", use12Hour = true))
        assertEquals("5:40:30 PM", formatDisplayClockTime("17:40:30", use12Hour = true))
    }

    @Test
    fun formatDisplayClockTime_preservesIsoValueInTwentyFourHourMode() {
        assertEquals("17:40:30", formatDisplayClockTime("17:40:30", use12Hour = false))
    }

    @Test
    fun formatDisplayClockTimeWithoutSeconds_formatsLatestRecordBoundary() {
        assertEquals("5:40 PM", formatDisplayClockTimeWithoutSeconds("17:40:30", use12Hour = true))
        assertEquals("17:40", formatDisplayClockTimeWithoutSeconds("17:40:30", use12Hour = false))
    }
}
