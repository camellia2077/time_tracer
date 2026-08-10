package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class NativeInsightsTranslatorTest {
    private val codec = NativeResponseCodec()
    private val translator = NativeInsightsTranslator(codec)

    @Test
    fun fromNativeResponse_success_preservesRawMarkdownText() {
        val markdown = "  # Daily Insights\\n\\n- item\\n\\n"
        val runtimeHash = "1111111111111111111111111111111111111111111111111111111111111111"
        val response = """{"ok":true,"content":${org.json.JSONObject.quote(markdown)},"error_message":"","insights_hash_sha256":"$runtimeHash","has_records":false,"matched_day_count":0,"matched_record_count":0,"start_date":"2026-02-01","end_date":"2026-02-07","requested_days":7}"""

        val result = translator.fromNativeResponse(
            response = response,
            operationId = "op-md-pass-through"
        )

        assertTrue(result.initialized)
        assertTrue(result.operationOk)
        assertEquals(markdown, result.outputText)
        assertEquals(false, result.insightsWindowMetadata?.hasRecords)
        assertEquals(7, result.insightsWindowMetadata?.requestedDays)

        val payload = codec.parse(response)
        assertEquals(runtimeHash, payload.insightsHashSha256)
        assertEquals(0, payload.insightsWindowMetadata?.matchedRecordCount)
    }

    @Test
    fun fromNativeResponse_failure_usesContextMessage() {
        val response = """{"ok":false,"content":"","error_message":"runtime insights failed.","error_code":"insights.target.not_found","error_category":"insights","hints":["Try a different day."]}"""

        val result = translator.fromNativeResponse(
            response = response,
            operationId = "op-md-failure"
        )

        assertTrue(result.initialized)
        assertFalse(result.operationOk)
        assertTrue(result.outputText.contains("runtime insights failed."))
        assertTrue(result.outputText.contains("op=op-md-failure"))
        assertEquals("insights.target.not_found", result.errorContract?.errorCode)
        assertEquals("insights", result.errorContract?.errorCategory)
        assertEquals(listOf("Try a different day."), result.errorContract?.hints)
    }
}
