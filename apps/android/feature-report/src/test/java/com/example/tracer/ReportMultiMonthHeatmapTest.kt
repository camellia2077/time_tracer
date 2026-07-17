package com.example.tracer

import java.time.YearMonth
import org.junit.Assert.assertEquals
import org.junit.Test

class ReportMultiMonthHeatmapTest {
    @Test
    fun resolveMultiMonthHeatmapMonths_yearAlwaysIncludesTwelveMonths() {
        val months = resolveMultiMonthHeatmapMonths(
            points = listOf(
                ReportChartPoint(date = "2026-03-04", durationSeconds = 3_600L),
                ReportChartPoint(date = "2026-11-08", durationSeconds = 1_800L)
            ),
            reportMode = ReportMode.YEAR
        )

        assertEquals(12, months.size)
        assertEquals(YearMonth.of(2026, 1), months.first())
        assertEquals(YearMonth.of(2026, 12), months.last())
    }

    @Test
    fun resolveMultiMonthHeatmapMonths_rangeIncludesOnlyMonthsInRange() {
        val months = resolveMultiMonthHeatmapMonths(
            points = listOf(
                ReportChartPoint(date = "2026-02-27", durationSeconds = 3_600L),
                ReportChartPoint(date = "2026-03-01", durationSeconds = 1_800L),
                ReportChartPoint(date = "2026-03-18", durationSeconds = 900L)
            ),
            reportMode = ReportMode.RANGE
        )

        assertEquals(listOf(YearMonth.of(2026, 2), YearMonth.of(2026, 3)), months)
    }
}
