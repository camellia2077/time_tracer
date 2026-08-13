package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Test

class TxtEditorInitialDayMarkerTest {
    @Test
    fun ignoresPersistedMarkerFromAnotherMonth() {
        assertEquals(
            "",
            initialDayMarkerForSelectedMonth(
                initialDayMarker = "0813",
                selectedMonth = "2026-06"
            )
        )
    }

    @Test
    fun retainsPersistedMarkerForTheSelectedMonth() {
        assertEquals(
            "0613",
            initialDayMarkerForSelectedMonth(
                initialDayMarker = "0613",
                selectedMonth = "2026-06"
            )
        )
    }
}
