package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertNull
import org.junit.Test

class InsightsPreviousPeriodComparisonTest {
    @Test
    fun dayComparison_movesAcrossMonthBoundary() {
        val resolved = resolveDefaultComparisonPeriodRequest(
            state = comparisonState(InsightsMode.DAY).copy(insightsDate = "20260301"),
            locale = "en"
        )

        assertEquals("2026-02-28", resolved?.request?.selection?.date)
        assertEquals("2026-02-28", resolved?.label)
    }

    @Test
    fun weekComparison_movesAcrossIsoYearBoundary() {
        val resolved = resolveDefaultComparisonPeriodRequest(
            state = comparisonState(InsightsMode.WEEK).copy(insightsWeek = "202601"),
            locale = "en"
        )

        assertEquals("2025-12-22", resolved?.request?.selection?.startDate)
        assertEquals("2025-12-28", resolved?.request?.selection?.endDate)
    }

    @Test
    fun monthAndYearComparison_usePreviousCalendarPeriods() {
        val month = resolveDefaultComparisonPeriodRequest(
            comparisonState(InsightsMode.MONTH).copy(insightsMonth = "202601"),
            "en"
        )
        val year = resolveDefaultComparisonPeriodRequest(
            comparisonState(InsightsMode.YEAR).copy(insightsYear = "2024"),
            "en"
        )

        assertEquals("2025-12-01", month?.request?.selection?.startDate)
        assertEquals("2025-12-31", month?.request?.selection?.endDate)
        assertEquals("2023-01-01", year?.request?.selection?.startDate)
        assertEquals("2023-12-31", year?.request?.selection?.endDate)
    }

    @Test
    fun rangeComparison_usesAdjacentEqualLengthWindowAcrossLeapDay() {
        val resolved = resolveDefaultComparisonPeriodRequest(
            comparisonState(InsightsMode.RANGE).copy(
                insightsRangeStartDate = "20240301",
                insightsRangeEndDate = "20240303"
            ),
            "en"
        )

        assertEquals("2024-02-27", resolved?.request?.selection?.startDate)
        assertEquals("2024-02-29", resolved?.request?.selection?.endDate)
    }

    @Test
    fun recentComparison_usesMetadataEndDateAsAnchoredPreviousWindow() {
        val state = comparisonState(InsightsMode.RECENT).copy(
            insightsSummariesByPeriod = mapOf(
                DataTreePeriod.RECENT to InsightsSummary.WindowMetadata(
                    period = DataTreePeriod.RECENT,
                    metadata = InsightsWindowMetadata(
                        hasRecords = true,
                        matchedDayCount = 7,
                        matchedRecordCount = 10,
                        startDate = "2026-03-01",
                        endDate = "2026-03-07",
                        requestedDays = 7
                    )
                )
            )
        )

        val resolved = resolveDefaultComparisonPeriodRequest(state, "en")

        assertEquals(InsightsDisplayMode.RECENT, resolved?.request?.displayMode)
        assertEquals(7, resolved?.request?.selection?.days)
        assertEquals("2026-02-28", resolved?.request?.selection?.anchorDate)
        assertEquals("2026-02-22 – 2026-02-28", resolved?.label)
    }

    @Test
    fun comparisonIsUnavailableWithoutCurrentResultOrRecentMetadata() {
        assertNull(resolveDefaultComparisonPeriodRequest(QueryInsightsUiState(), "en"))
        assertNull(resolveDefaultComparisonPeriodRequest(comparisonState(InsightsMode.RECENT), "en"))
    }

    private fun comparisonState(mode: InsightsMode): QueryInsightsUiState {
        val period = mode.toDataTreePeriod()
        return QueryInsightsUiState(
            insightsMode = mode,
            insightsResultsByPeriod = mapOf(
                period to QueryResult.Insights(text = "current")
            )
        )
    }
}
