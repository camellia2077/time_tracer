package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test
import java.time.LocalDate

class InsightsTemporalSelectionResolverTest {
    private val resolver = InsightsTemporalSelectionResolver(
        inputValidator = QueryInputValidator(),
        textProvider = DefaultQueryInsightsTextProvider
    )

    @Test
    fun resolve_day_returnsSingleDay() {
        val result = resolver.resolve(
            QueryInsightsUiState(
                insightsMode = InsightsMode.DAY,
                insightsDate = "20260412"
            )
        )

        val success = result as InsightsTemporalSelectionResolveResult.Success
        val selection = success.selection as InsightsTemporalSelection.SingleDay
        assertEquals(LocalDate.of(2026, 4, 12), selection.date)
    }

    @Test
    fun resolve_week_returnsDateRange() {
        val result = resolver.resolve(
            QueryInsightsUiState(
                insightsMode = InsightsMode.WEEK,
                insightsWeek = "202615"
            )
        )

        val success = result as InsightsTemporalSelectionResolveResult.Success
        val selection = success.selection as InsightsTemporalSelection.DateRange
        assertEquals(LocalDate.of(2026, 4, 6), selection.start)
        assertEquals(LocalDate.of(2026, 4, 12), selection.end)
    }

    @Test
    fun resolve_recent_returnsRecentDays() {
        val result = resolver.resolve(
            QueryInsightsUiState(
                insightsMode = InsightsMode.RECENT,
                insightsRecentDays = "11"
            )
        )

        val success = result as InsightsTemporalSelectionResolveResult.Success
        val selection = success.selection as InsightsTemporalSelection.RecentDays
        assertEquals(11, selection.days)
    }

    @Test
    fun resolve_invalidRange_returnsFailure() {
        val result = resolver.resolve(
            QueryInsightsUiState(
                insightsMode = InsightsMode.RANGE,
                insightsRangeStartDate = "20260420",
                insightsRangeEndDate = "20260401"
            )
        )

        val failure = result as InsightsTemporalSelectionResolveResult.Failure
        assertEquals(InsightsMode.RANGE, failure.insightsMode)
        assertTrue(failure.validationError.isNotBlank())
    }
}
