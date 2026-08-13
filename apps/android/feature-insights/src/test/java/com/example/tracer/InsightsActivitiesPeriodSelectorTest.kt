package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Test

class InsightsActivitiesPeriodSelectorTest {
    @Test
    fun formatSummary_usesTheSelectedKindOfPeriod() {
        val draft = InsightsPeriodSelection(
            date = "20260518",
            month = "202605",
            year = "2026",
            week = "202620"
        )

        assertEquals("2026-05-18", formatInsightsActivityPeriodSummary(InsightsMode.DAY, draft))
        assertEquals("2026-05", formatInsightsActivityPeriodSummary(InsightsMode.MONTH, draft))
        assertEquals("2026", formatInsightsActivityPeriodSummary(InsightsMode.YEAR, draft))
        assertEquals(
            "05-11 ~ 05-17 · W20",
            formatInsightsActivityPeriodSummary(InsightsMode.WEEK, draft)
        )
    }
}
