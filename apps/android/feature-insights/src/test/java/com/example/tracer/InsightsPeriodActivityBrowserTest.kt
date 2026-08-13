package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Test

class InsightsPeriodActivityBrowserTest {
    @Test
    fun recordsAreOrderedFromEarliestToLatestDate() {
        val days = listOf(
            StructuredDailyInsights(date = "2026-08-03", totalDurationSeconds = 0L),
            StructuredDailyInsights(date = "2025-12-31", totalDurationSeconds = 0L),
            StructuredDailyInsights(date = "2026-01-01", totalDurationSeconds = 0L)
        )

        assertEquals(
            listOf("2025-12-31", "2026-01-01", "2026-08-03"),
            sortPeriodActivityDaysChronologically(days).map(StructuredDailyInsights::date)
        )
    }

    @Test
    fun recordDurationUsesDaysForDurationsAtLeastOneDay() {
        assertEquals("1d 1h 2m", formatPeriodActivityDuration(90_120L))
    }
}
