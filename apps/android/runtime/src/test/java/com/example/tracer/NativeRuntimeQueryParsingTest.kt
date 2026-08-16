package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Test

class NativeRuntimeQueryParsingTest {
    @Test
    fun parseLatestActivityRecord_readsActivityAndBoundaries() {
        val parsed = parseLatestActivityRecordContent(
            """
                {
                  "schema_version": 1,
                  "action": "latest_activity_record",
                  "output_mode": "semantic_json",
                  "found": true,
                  "date": "2026-08-15",
                  "activity": "study",
                  "record_kind": "interval",
                  "start_time": "10:00:00",
                  "end_time": "12:00:00",
                  "duration_seconds": 7200
                }
            """.trimIndent()
        )

        assertTrue(parsed!!.found)
        assertEquals("study", parsed.record!!.activity)
        assertEquals("10:00:00", parsed.record.startTime)
        assertEquals("12:00:00", parsed.record.endTime)
        assertEquals(7200, parsed.record.durationSeconds)
    }

    @Test
    fun parsePreviousActivityTail_readsFoundBoundary() {
        val parsed = parsePreviousActivityTailContent(
            """
                {
                  "schema_version": 1,
                  "action": "previous_activity_tail",
                  "output_mode": "semantic_json",
                  "found": true,
                  "date": "2026-08-15",
                  "end_time": "12:00:00"
                }
            """.trimIndent()
        )

        assertTrue(parsed!!.found)
        assertEquals("2026-08-15", parsed.tail!!.dateIso)
        assertEquals("12:00:00", parsed.tail.endTime)
    }

    @Test
    fun parsePreviousActivityTail_keepsSuccessfulEmptyResult() {
        val parsed = parsePreviousActivityTailContent(
            """
                {
                  "schema_version": 1,
                  "action": "previous_activity_tail",
                  "output_mode": "semantic_json",
                  "found": false
                }
            """.trimIndent()
        )

        assertFalse(parsed!!.found)
        assertNull(parsed.tail)
    }

    @Test
    fun parseLatestActivityRecord_rejectsInvalidIsoClockTime() {
        val parsed = parseLatestActivityRecordContent(
            """
                {
                  "action": "latest_activity_record",
                  "output_mode": "semantic_json",
                  "found": true,
                  "date": "2026-08-15",
                  "activity": "study",
                  "record_kind": "interval",
                  "start_time": "24:00:00",
                  "end_time": "12:00:00",
                  "duration_seconds": 7200
                }
            """.trimIndent()
        )

        assertNull(parsed)
    }
}
