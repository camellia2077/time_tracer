package com.example.tracer

import java.time.YearMonth
import org.junit.Assert.assertEquals
import org.junit.Test

class InsightsMultiMonthHeatmapTest {
    @Test
    fun resolveMultiMonthHeatmapMonths_yearAlwaysIncludesTwelveMonths() {
        val months = resolveMultiMonthHeatmapMonths(
            points = listOf(
                InsightsChartPoint(date = "2026-03-04", durationSeconds = 3_600L),
                InsightsChartPoint(date = "2026-11-08", durationSeconds = 1_800L)
            ),
            insightsMode = InsightsMode.YEAR
        )

        assertEquals(12, months.size)
        assertEquals(YearMonth.of(2026, 1), months.first())
        assertEquals(YearMonth.of(2026, 12), months.last())
    }

    @Test
    fun resolveMultiMonthHeatmapMonths_rangeIncludesOnlyMonthsInRange() {
        val months = resolveMultiMonthHeatmapMonths(
            points = listOf(
                InsightsChartPoint(date = "2026-02-27", durationSeconds = 3_600L),
                InsightsChartPoint(date = "2026-03-01", durationSeconds = 1_800L),
                InsightsChartPoint(date = "2026-03-18", durationSeconds = 900L)
            ),
            insightsMode = InsightsMode.RANGE
        )

        assertEquals(listOf(YearMonth.of(2026, 2), YearMonth.of(2026, 3)), months)
    }
}
