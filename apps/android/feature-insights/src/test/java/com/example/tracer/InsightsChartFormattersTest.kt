package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Test

class InsightsChartFormattersTest {
    @Test
    fun standardDuration_includesSeconds() {
        assertEquals("1h 2m 3s", formatDurationHoursMinutes(3_723L))
    }

    @Test
    fun standardDuration_usesDaysAfterTwentyFourHours() {
        assertEquals("1d 1h 2m 3s", formatDurationHoursMinutes(90_123L))
    }

    @Test
    fun treeDuration_omitsSeconds() {
        assertEquals("1h 2m", formatTreemapDurationHoursMinutes(3_723L))
    }

    @Test
    fun treeDuration_usesDaysAfterTwentyFourHours() {
        assertEquals("1d 1h 2m", formatTreemapDurationHoursMinutes(90_120L))
    }

    @Test
    fun compactDuration_omitsSecondsAfterOneHour() {
        assertEquals(
            "1h 2m",
            formatInsightsDuration(3_723L, InsightsDurationFormat.COMPACT)
        )
    }
}
