package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class QueryReportChartParamResolverTest {
    private val resolver = QueryReportChartParamResolver(
        inputValidator = QueryInputValidator(),
        textProvider = DefaultQueryReportTextProvider
    )

    @Test
    fun resolve_month_mapsToMonthDateWindow() {
        val params = resolver.resolve(
            QueryReportUiState(
                reportMode = ReportMode.MONTH,
                reportMonth = "202602"
            )
        )

        assertEquals(ReportMode.MONTH, params.reportMode)
        assertEquals("2026-02-01", params.fromDateIso)
        assertEquals("2026-02-28", params.toDateIso)
        assertEquals(28, params.lookbackDays)
    }

    @Test
    fun resolve_year_mapsToYearDateWindow() {
        val params = resolver.resolve(
            QueryReportUiState(
                reportMode = ReportMode.YEAR,
                reportYear = "2024"
            )
        )

        assertEquals("2024-01-01", params.fromDateIso)
        assertEquals("2024-12-31", params.toDateIso)
        assertEquals(366, params.lookbackDays)
    }

    @Test
    fun resolve_week_mapsIsoWeekToMondaySundayWindow() {
        val params = resolver.resolve(
            QueryReportUiState(
                reportMode = ReportMode.WEEK,
                reportWeek = "202615"
            )
        )

        assertEquals("2026-04-06", params.fromDateIso)
        assertEquals("2026-04-12", params.toDateIso)
        assertEquals(7, params.lookbackDays)
    }

    @Test
    fun resolve_range_usesInclusiveDayCount() {
        val params = resolver.resolve(
            QueryReportUiState(
                reportMode = ReportMode.RANGE,
                reportRangeStartDate = "20260210",
                reportRangeEndDate = "20260214"
            )
        )

        assertEquals("2026-02-10", params.fromDateIso)
        assertEquals("2026-02-14", params.toDateIso)
        assertEquals(5, params.lookbackDays)
    }

    @Test
    fun resolve_recent_usesLookbackOnly() {
        val params = resolver.resolve(
            QueryReportUiState(
                reportMode = ReportMode.RECENT,
                reportRecentDays = "9"
            )
        )

        assertEquals(9, params.lookbackDays)
        assertEquals(null, params.fromDateIso)
        assertEquals(null, params.toDateIso)
        assertTrue(params.validationError.isEmpty())
    }
}
