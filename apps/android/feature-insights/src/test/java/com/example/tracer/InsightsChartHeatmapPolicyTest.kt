package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Test

class InsightsChartHeatmapPolicyTest {
    @Test
    fun rangeUsesSingleMonthWhenExplicitWindowStaysWithinMonth() {
        assertEquals(
            InsightsChartVisualMode.HEATMAP_MONTH,
            resolveInsightsChartVisualMode(
                insightsMode = InsightsMode.RANGE,
                requestedMode = InsightsChartVisualMode.HEATMAP_MONTH,
                points = listOf(chartPoint("2026-07-01")),
                fromDateIso = "2026-07-20",
                toDateIso = "2026-07-31"
            )
        )
    }

    @Test
    fun rangeUsesMultiMonthWhenExplicitWindowCrossesMonth() {
        assertEquals(
            InsightsChartVisualMode.HEATMAP_MULTI_MONTH,
            resolveInsightsChartVisualMode(
                insightsMode = InsightsMode.RANGE,
                requestedMode = InsightsChartVisualMode.HEATMAP_MONTH,
                points = emptyList(),
                fromDateIso = "2026-07-31",
                toDateIso = "2026-08-01"
            )
        )
    }

    @Test
    fun recentFallsBackToPointMonthsWhenExplicitWindowIsUnavailable() {
        assertEquals(
            InsightsChartVisualMode.HEATMAP_MULTI_MONTH,
            resolveInsightsChartVisualMode(
                insightsMode = InsightsMode.RECENT,
                requestedMode = InsightsChartVisualMode.HEATMAP_MONTH,
                points = listOf(
                    chartPoint("2026-07-31"),
                    chartPoint("2026-08-01")
                )
            )
        )
    }

    @Test
    fun yearOnlyOffersMultiMonthHeatmap() {
        val modes = availableInsightsChartVisualModes(InsightsMode.YEAR)

        assertEquals(false, InsightsChartVisualMode.HEATMAP_MONTH in modes)
        assertEquals(true, InsightsChartVisualMode.HEATMAP_MULTI_MONTH in modes)
    }

    private fun chartPoint(date: String): InsightsChartPoint = InsightsChartPoint(
        date = date,
        durationSeconds = 60L
    )
}
