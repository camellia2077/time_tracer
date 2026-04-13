package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test
import java.time.LocalDate

class ReportTemporalSelectionResolverTest {
    private val resolver = ReportTemporalSelectionResolver(
        inputValidator = QueryInputValidator(),
        textProvider = DefaultQueryReportTextProvider
    )

    @Test
    fun resolve_day_returnsSingleDay() {
        val result = resolver.resolve(
            QueryReportUiState(
                reportMode = ReportMode.DAY,
                reportDate = "20260412"
            )
        )

        val success = result as ReportTemporalSelectionResolveResult.Success
        val selection = success.selection as ReportTemporalSelection.SingleDay
        assertEquals(LocalDate.of(2026, 4, 12), selection.date)
    }

    @Test
    fun resolve_week_returnsDateRange() {
        val result = resolver.resolve(
            QueryReportUiState(
                reportMode = ReportMode.WEEK,
                reportWeek = "202615"
            )
        )

        val success = result as ReportTemporalSelectionResolveResult.Success
        val selection = success.selection as ReportTemporalSelection.DateRange
        assertEquals(LocalDate.of(2026, 4, 6), selection.start)
        assertEquals(LocalDate.of(2026, 4, 12), selection.end)
    }

    @Test
    fun resolve_recent_returnsRecentDays() {
        val result = resolver.resolve(
            QueryReportUiState(
                reportMode = ReportMode.RECENT,
                reportRecentDays = "11"
            )
        )

        val success = result as ReportTemporalSelectionResolveResult.Success
        val selection = success.selection as ReportTemporalSelection.RecentDays
        assertEquals(11, selection.days)
    }

    @Test
    fun resolve_invalidRange_returnsFailure() {
        val result = resolver.resolve(
            QueryReportUiState(
                reportMode = ReportMode.RANGE,
                reportRangeStartDate = "20260420",
                reportRangeEndDate = "20260401"
            )
        )

        val failure = result as ReportTemporalSelectionResolveResult.Failure
        assertEquals(ReportMode.RANGE, failure.reportMode)
        assertTrue(failure.validationError.isNotBlank())
    }
}
