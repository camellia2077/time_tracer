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
        val response = """{"ok":true,"content":${org.json.JSONObject.quote(markdown)},"error_message":"","report_hash_sha256":"$runtimeHash"}"""

        val result = translator.fromNativeResponse(
            response = response,
            operationId = "op-md-pass-through"
        )

        assertTrue(result.initialized)
        assertTrue(result.operationOk)
        assertEquals(markdown, result.outputText)

        val payload = codec.parse(response)
        assertEquals(runtimeHash, payload.reportHashSha256)
    }

    @Test
    fun fromNativeResponse_failure_usesContextMessage() {
        val response = """{"ok":false,"content":"","error_message":"runtime report failed."}"""

        val result = translator.fromNativeResponse(
            response = response,
            operationId = "op-md-failure"
        )

        assertTrue(result.initialized)
        assertFalse(result.operationOk)
        assertTrue(result.outputText.contains("runtime report failed."))
        assertTrue(result.outputText.contains("op=op-md-failure"))
    }
}
