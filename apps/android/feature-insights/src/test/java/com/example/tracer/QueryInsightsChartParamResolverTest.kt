package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class QueryInsightsChartParamResolverTest {
    private val resolver = QueryInsightsChartParamResolver(
        inputValidator = QueryInputValidator(),
        textProvider = DefaultQueryInsightsTextProvider
    )

    @Test
    fun resolve_month_mapsToMonthDateWindow() {
        val params = resolver.resolve(
            QueryInsightsUiState(
                insightsMode = InsightsMode.MONTH,
                insightsMonth = "202602"
            )
        )

        assertEquals(InsightsMode.MONTH, params.insightsMode)
        assertEquals("2026-02-01", params.fromDateIso)
        assertEquals("2026-02-28", params.toDateIso)
        assertEquals(28, params.lookbackDays)
    }

    @Test
    fun resolve_year_mapsToYearDateWindow() {
        val params = resolver.resolve(
            QueryInsightsUiState(
                insightsMode = InsightsMode.YEAR,
                insightsYear = "2024"
            )
        )

        assertEquals("2024-01-01", params.fromDateIso)
        assertEquals("2024-12-31", params.toDateIso)
        assertEquals(366, params.lookbackDays)
    }

    @Test
    fun resolve_week_mapsIsoWeekToMondaySundayWindow() {
        val params = resolver.resolve(
            QueryInsightsUiState(
                insightsMode = InsightsMode.WEEK,
                insightsWeek = "202615"
            )
        )

        assertEquals("2026-04-06", params.fromDateIso)
        assertEquals("2026-04-12", params.toDateIso)
        assertEquals(7, params.lookbackDays)
    }

    @Test
    fun resolve_range_usesInclusiveDayCount() {
        val params = resolver.resolve(
            QueryInsightsUiState(
                insightsMode = InsightsMode.RANGE,
                insightsRangeStartDate = "20260210",
                insightsRangeEndDate = "20260214"
            )
        )

        assertEquals("2026-02-10", params.fromDateIso)
        assertEquals("2026-02-14", params.toDateIso)
        assertEquals(5, params.lookbackDays)
    }

    @Test
    fun resolve_recent_usesLookbackOnly() {
        val params = resolver.resolve(
            QueryInsightsUiState(
                insightsMode = InsightsMode.RECENT,
                insightsRecentDays = "9"
            )
        )

        assertEquals(9, params.lookbackDays)
        assertEquals(null, params.fromDateIso)
        assertEquals(null, params.toDateIso)
        assertTrue(params.validationError.isEmpty())
    }
}
