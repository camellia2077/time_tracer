package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertNull
import org.junit.Test

class NativeResponseCodecTest {
    private val codec = NativeResponseCodec()

    @Test
    fun parse_readsStructuredInsightsFields_whenPresent() {
        val response = """
            {
              "ok": false,
              "content": "",
              "error_message": "missing insights target",
              "error_code": "insights.target.not_found",
              "error_category": "insights",
              "hints": ["Try another day.", "Inspect available dates."],
              "has_records": false,
              "matched_day_count": 0,
              "matched_record_count": 0,
              "start_date": "2026-02-01",
              "end_date": "2026-02-07",
              "requested_days": 7
            }
        """.trimIndent()

        val payload = codec.parse(response)

        assertEquals(false, payload.ok)
        assertEquals("insights.target.not_found", payload.errorContract?.errorCode)
        assertEquals("insights", payload.errorContract?.errorCategory)
        assertEquals(
            listOf("Try another day.", "Inspect available dates."),
            payload.errorContract?.hints
        )
        assertEquals(false, payload.insightsWindowMetadata?.hasRecords)
        assertEquals(0, payload.insightsWindowMetadata?.matchedDayCount)
        assertEquals("2026-02-01", payload.insightsWindowMetadata?.startDate)
        assertEquals(7, payload.insightsWindowMetadata?.requestedDays)
    }

    @Test
    fun parse_leavesStructuredFieldsNull_whenAbsent() {
        val payload = codec.parse("""{"ok":true,"content":"# Insights","error_message":""}""")

        assertNull(payload.errorContract)
        assertNull(payload.insightsWindowMetadata)
        assertNotNull(payload)
    }
}
