package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Test

class InsightsChartAggregationTest {
    @Test
    fun rangeUsesMonthlyAggregationOnlyAfter90Days() {
        assertEquals(
            false,
            shouldAggregateChartPointsByMonth(
                InsightsMode.RANGE,
                "2026-01-01",
                "2026-03-31"
            )
        )
        assertEquals(
            true,
            shouldAggregateChartPointsByMonth(
                InsightsMode.RANGE,
                "2026-01-01",
                "2026-04-01"
            )
        )
    }

    @Test
    fun yearChartPoints_sumDurationsIntoCalendarMonths() {
        val result = aggregateYearChartPoints(
            points = listOf(
                InsightsChartPoint("2026-01-02", 10L),
                InsightsChartPoint("2026-01-20", 20L),
                InsightsChartPoint("2026-03-03", 40L)
            ),
            fromDateIso = "2026-01-01",
            toDateIso = "2026-12-31"
        )

        assertEquals(12, result.size)
        assertEquals("2026-01-01", result.first().date)
        assertEquals(30L, result.first().durationSeconds)
        assertEquals("2026-02-01", result[1].date)
        assertEquals(0L, result[1].durationSeconds)
        assertEquals(40L, result[2].durationSeconds)
        assertEquals("2026-12-01", result.last().date)
    }
}
