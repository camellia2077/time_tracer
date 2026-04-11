package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Test

class NativeTxtRuntimeCodecTest {
    private val codec = NativeTxtRuntimeCodec()

    @Test
    fun parseDayMarker_readsNormalizedMarkerFromNativeResponse() {
        val payload = codec.parseDayMarker(
            """{"ok":true,"normalized_day_marker":"0228","error_message":""}"""
        )

        assertTrue(payload.ok)
        assertEquals("0228", payload.normalizedDayMarker)
        assertEquals("", payload.message)
    }

    @Test
    fun parseResolve_readsFoundStateAndIsoDate() {
        val payload = codec.parseResolve(
            """
                {
                  "ok": true,
                  "normalized_day_marker": "0102",
                  "found": true,
                  "is_marker_valid": true,
                  "can_save": true,
                  "day_body": "0656w\n2207minecraft\n",
                  "day_content_iso_date": "2025-01-02",
                  "error_message": ""
                }
            """.trimIndent()
        )

        assertTrue(payload.ok)
        assertEquals("0102", payload.normalizedDayMarker)
        assertTrue(payload.found)
        assertTrue(payload.isMarkerValid)
        assertTrue(payload.canSave)
        assertEquals("0656w\n2207minecraft\n", payload.dayBody)
        assertEquals("2025-01-02", payload.dayContentIsoDate)
    }

    @Test
    fun parseResolve_keepsMissingIsoDateAsNull() {
        val payload = codec.parseResolve(
            """
                {
                  "ok": true,
                  "normalized_day_marker": "0103",
                  "found": false,
                  "is_marker_valid": true,
                  "can_save": false,
                  "day_body": "",
                  "error_message": ""
                }
            """.trimIndent()
        )

        assertTrue(payload.ok)
        assertFalse(payload.found)
        assertTrue(payload.isMarkerValid)
        assertFalse(payload.canSave)
        assertNull(payload.dayContentIsoDate)
    }

    @Test
    fun parseReplace_readsUpdatedContent() {
        val payload = codec.parseReplace(
            """
                {
                  "ok": true,
                  "normalized_day_marker": "0102",
                  "found": true,
                  "is_marker_valid": true,
                  "updated_content": "0102\n1111work\n",
                  "error_message": ""
                }
            """.trimIndent()
        )

        assertTrue(payload.ok)
        assertEquals("0102", payload.normalizedDayMarker)
        assertTrue(payload.found)
        assertTrue(payload.isMarkerValid)
        assertEquals("0102\n1111work\n", payload.updatedContent)
    }

    @Test
    fun parseResolve_returnsFailurePayloadWhenJsonIsInvalid() {
        val payload = codec.parseResolve("not-json")

        assertFalse(payload.ok)
        assertFalse(payload.found)
        assertFalse(payload.isMarkerValid)
        assertEquals("Invalid native TXT response.", payload.message)
    }
}
