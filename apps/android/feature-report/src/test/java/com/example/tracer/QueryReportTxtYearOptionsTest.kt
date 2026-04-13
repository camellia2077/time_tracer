package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Test

class QueryReportTxtYearOptionsTest {
    @Test
    fun deriveTxtYearOptions_extractsDistinctYearsFromTxtMonthKeys() {
        val years = deriveTxtYearOptions(
            listOf("2026-04", "2025-12", "2026-01", "invalid", "2024-08")
        )

        assertEquals(listOf("2026", "2025", "2024"), years)
    }

    @Test
    fun deriveTxtYearOptions_ignoresMalformedMonthKeys() {
        val years = deriveTxtYearOptions(
            listOf("2026", "2026-4", "abcd-02", "2026-13", " 2025-02 ")
        )

        assertEquals(listOf("2025"), years)
    }
}
