package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Test

class ReportChartHeatmapPolicyTest {
    @Test
    fun rangeUsesSingleMonthWhenExplicitWindowStaysWithinMonth() {
        assertEquals(
            ReportChartVisualMode.HEATMAP_MONTH,
            resolveReportChartVisualMode(
                reportMode = ReportMode.RANGE,
                requestedMode = ReportChartVisualMode.HEATMAP_MONTH,
                points = listOf(chartPoint("2026-07-01")),
                fromDateIso = "2026-07-20",
                toDateIso = "2026-07-31"
            )
        )
    }

    @Test
    fun rangeUsesMultiMonthWhenExplicitWindowCrossesMonth() {
        assertEquals(
            ReportChartVisualMode.HEATMAP_MULTI_MONTH,
            resolveReportChartVisualMode(
                reportMode = ReportMode.RANGE,
                requestedMode = ReportChartVisualMode.HEATMAP_MONTH,
                points = emptyList(),
                fromDateIso = "2026-07-31",
                toDateIso = "2026-08-01"
            )
        )
    }

    @Test
    fun recentFallsBackToPointMonthsWhenExplicitWindowIsUnavailable() {
        assertEquals(
            ReportChartVisualMode.HEATMAP_MULTI_MONTH,
            resolveReportChartVisualMode(
                reportMode = ReportMode.RECENT,
                requestedMode = ReportChartVisualMode.HEATMAP_MONTH,
                points = listOf(
                    chartPoint("2026-07-31"),
                    chartPoint("2026-08-01")
                )
            )
        )
    }

    @Test
    fun yearOnlyOffersMultiMonthHeatmap() {
        val modes = availableReportChartVisualModes(ReportMode.YEAR)

        assertEquals(false, ReportChartVisualMode.HEATMAP_MONTH in modes)
        assertEquals(true, ReportChartVisualMode.HEATMAP_MULTI_MONTH in modes)
    }

    private fun chartPoint(date: String): ReportChartPoint = ReportChartPoint(
        date = date,
        durationSeconds = 60L
    )
}
