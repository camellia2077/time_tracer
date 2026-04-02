package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class NativeReportTranslatorTest {
    private val codec = NativeResponseCodec()
    private val translator = NativeReportTranslator(codec)

    @Test
    fun fromNativeResponse_success_preservesRawMarkdownText() {
        val markdown = "  # Daily Report\\n\\n- item\\n\\n"
        val runtimeHash = "1111111111111111111111111111111111111111111111111111111111111111"
        val response = """{"ok":true,"content":${org.json.JSONObject.quote(markdown)},"error_message":"","report_hash_sha256":"$runtimeHash","has_records":false,"matched_day_count":0,"matched_record_count":0,"start_date":"2026-02-01","end_date":"2026-02-07","requested_days":7}"""

        val result = translator.fromNativeResponse(
            response = response,
            operationId = "op-md-pass-through"
        )

        assertTrue(result.initialized)
        assertTrue(result.operationOk)
        assertEquals(markdown, result.outputText)
        assertEquals(false, result.reportWindowMetadata?.hasRecords)
        assertEquals(7, result.reportWindowMetadata?.requestedDays)

        val payload = codec.parse(response)
        assertEquals(runtimeHash, payload.reportHashSha256)
        assertEquals(0, payload.reportWindowMetadata?.matchedRecordCount)
    }

    @Test
    fun fromNativeResponse_failure_usesContextMessage() {
        val response = """{"ok":false,"content":"","error_message":"runtime report failed.","error_code":"reporting.target.not_found","error_category":"reporting","hints":["Try a different day."]}"""

        val result = translator.fromNativeResponse(
            response = response,
            operationId = "op-md-failure"
        )

        assertTrue(result.initialized)
        assertFalse(result.operationOk)
        assertTrue(result.outputText.contains("runtime report failed."))
        assertTrue(result.outputText.contains("op=op-md-failure"))
        assertEquals("reporting.target.not_found", result.errorContract?.errorCode)
        assertEquals("reporting", result.errorContract?.errorCategory)
        assertEquals(listOf("Try a different day."), result.errorContract?.hints)
    }
}
