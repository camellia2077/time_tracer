package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Test

class InsightsChartFormattersTest {
    @Test
    fun standardDuration_includesSeconds() {
        assertEquals("1h 2m 3s", formatDurationHoursMinutes(3_723L))
    }

    @Test
    fun treeDuration_omitsSeconds() {
        assertEquals("1h 2m", formatTreemapDurationHoursMinutes(3_723L))
    }
}
