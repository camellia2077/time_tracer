package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Test

class QueryInsightsCalendarStateTest {
    @Test
    fun asyncQueryStateKeepsTheLatestCalendarAvailability() {
        val staleResult = QueryInsightsUiState(
            availableInsightsMonths = emptyList(),
            statusText = "query complete"
        )
        val actual = staleResult.preserveCalendarAvailability(
            latestCalendarMonths = listOf("2025-01", "2026-06")
        )

        assertEquals(listOf("2025-01", "2026-06"), actual.availableInsightsMonths)
        assertEquals("query complete", actual.statusText)
    }
}
