package com.example.tracer

import kotlinx.coroutines.runBlocking
import org.json.JSONObject
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class RuntimeQueryDelegateDiagnosticsTest {
    @Test
    fun queryActivitySuggestions_recordsDiagnosticSummary() = runBlocking {
        val recorder = RuntimeDiagnosticsRecorder(runtimePathsProvider = { null })
        val delegate = RuntimeQueryDelegate(
            queryTranslator = NativeQueryTranslator(NativeResponseCodec()),
            executeNativeDataQuery = { request, _ ->
                when (request.action) {
                    NativeBridge.QUERY_ACTION_ACTIVITY_SUGGEST -> NativeCallResult(
                        initialized = true,
                        operationOk = true,
                        rawResponse = nativeContentResponse("study_cpp | count=2\nmeal | count=1\n"),
                        operationId = "op-suggest"
                    )

                    NativeBridge.QUERY_ACTION_AUTHORABLE_EVENT_TOKENS -> NativeCallResult(
                        initialized = true,
                        operationOk = true,
                        rawResponse = nativeContentResponse(
                            JSONObject()
                                .put("names", listOf("study_cpp"))
                                .toString()
                        ),
                        operationId = "op-authorable"
                    )

                    else -> NativeCallResult(
                        initialized = true,
                        operationOk = false,
                        rawResponse = nativeFailureResponse("unexpected action")
                    )
                }
            },
            ensureConfigTomlStorage = { error("config storage not expected in this test") },
            diagnosticsRecorder = recorder,
            nextOperationId = { "generated-$it" }
        )

        val result = delegate.queryActivitySuggestions(
            lookbackDays = 7,
            topN = 5,
            anchorDateIso = "2026-03-31"
        )

        assertEquals(listOf("study_cpp", "meal"), result.suggestions)
        assertEquals("op-suggest", result.operationId)
        val diagnostic = recorder.recent(limit = 1).single()
        assertEquals("op-suggest", diagnostic.operationId)
        assertEquals("query.activity_suggestions", diagnostic.stage)
        assertTrue(diagnostic.ok)
        assertTrue(diagnostic.message.contains("lookbackDays=7"))
        assertTrue(diagnostic.message.contains("topN=5"))
        assertTrue(diagnostic.message.contains("anchorDateIso=2026-03-31"))
        assertTrue(diagnostic.message.contains("nativeOk=true"))
        assertTrue(diagnostic.message.contains("rawCount=2"))
        assertTrue(diagnostic.message.contains("authorableOk=true"))
        assertTrue(diagnostic.message.contains("authorableCount=1"))
        assertTrue(diagnostic.message.contains("finalCount=2"))
        assertTrue(diagnostic.message.contains("rawSample=[study_cpp,meal]"))
        assertTrue(diagnostic.message.contains("finalSample=[study_cpp,meal]"))
    }

    private fun nativeContentResponse(content: String): String =
        JSONObject()
            .put("ok", true)
            .put("content", content)
            .put("error_message", "")
            .toString()

    private fun nativeFailureResponse(message: String): String =
        JSONObject()
            .put("ok", false)
            .put("content", "")
            .put("error_message", message)
            .toString()
}
