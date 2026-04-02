package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertNull
import org.junit.Test

class NativeResponseCodecTest {
    private val codec = NativeResponseCodec()

    @Test
    fun parse_readsStructuredReportFields_whenPresent() {
        val response = """
            {
              "ok": false,
              "content": "",
              "error_message": "missing report target",
              "error_code": "reporting.target.not_found",
              "error_category": "reporting",
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
        assertEquals("reporting.target.not_found", payload.errorContract?.errorCode)
        assertEquals("reporting", payload.errorContract?.errorCategory)
        assertEquals(
            listOf("Try another day.", "Inspect available dates."),
            payload.errorContract?.hints
        )
        assertEquals(false, payload.reportWindowMetadata?.hasRecords)
        assertEquals(0, payload.reportWindowMetadata?.matchedDayCount)
        assertEquals("2026-02-01", payload.reportWindowMetadata?.startDate)
        assertEquals(7, payload.reportWindowMetadata?.requestedDays)
    }

    @Test
    fun parse_leavesStructuredFieldsNull_whenAbsent() {
        val payload = codec.parse("""{"ok":true,"content":"# Report","error_message":""}""")

        assertNull(payload.errorContract)
        assertNull(payload.reportWindowMetadata)
        assertNotNull(payload)
    }
}
