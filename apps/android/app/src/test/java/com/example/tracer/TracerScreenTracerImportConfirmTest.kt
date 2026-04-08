package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Test

class TracerScreenTracerImportConfirmTest {
    @Test
    fun formatTracerExchangeCreatedAt_formatsUtcToLocalLikeText() {
        val result = formatTracerExchangeCreatedAt("2026-03-20T08:00:00Z")
        assertEquals(16, result.length)
        assertEquals('-', result[4])
        assertEquals('-', result[7])
        assertEquals(' ', result[10])
        assertEquals(':', result[13])
    }

    @Test
    fun formatTracerExchangeCreatedAt_fallsBackToRawTextOnParseFailure() {
        assertEquals("not-a-date", formatTracerExchangeCreatedAt("not-a-date"))
    }
}
