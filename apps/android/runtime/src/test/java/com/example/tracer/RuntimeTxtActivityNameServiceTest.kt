package com.example.tracer

import kotlinx.coroutines.runBlocking
import org.json.JSONObject
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class RuntimeTxtActivityNameServiceTest {
    @Test
    fun convertTxtActivityNames_buildsSharedTxtRequestAndParsesContent() = runBlocking {
        var request = JSONObject()
        val service = RuntimeTxtActivityNameService(
            initializeRuntimeInternal = {
                NativeCallResult(
                    initialized = true,
                    operationOk = true,
                    rawResponse = "{}"
                )
            },
            nativeTxt = { requestJson ->
                request = JSONObject(requestJson)
                JSONObject()
                    .put("ok", true)
                    .put("converted_content", "canonical-month")
                    .put("error_message", "")
                    .toString()
            },
            codec = NativeTxtRuntimeCodec()
        )

        val result = service.convertTxtActivityNames(
            content = "alias-month",
            direction = TxtActivityNameMappingDirection.ALIAS_TO_CANONICAL
        )

        assertTrue(result.ok)
        assertEquals("convert_activity_names", request.getString("action"))
        assertEquals("alias-month", request.getString("content"))
        assertEquals("alias_to_canonical", request.getString("direction"))
        assertEquals("canonical-month", result.convertedContent)
    }

    @Test
    fun replaceCanonicalActivityNames_buildsExactReplacementRequest() = runBlocking {
        var request = JSONObject()
        val service = RuntimeTxtActivityNameService(
            initializeRuntimeInternal = {
                NativeCallResult(initialized = true, operationOk = true, rawResponse = "{}")
            },
            nativeTxt = { requestJson ->
                request = JSONObject(requestJson)
                JSONObject()
                    .put("ok", true)
                    .put("updated_content", "migrated-month")
                    .put("error_message", "")
                    .toString()
            },
            codec = NativeTxtRuntimeCodec()
        )

        val result = service.replaceTxtCanonicalActivityNames(
            content = "old-month",
            replacements = listOf(
                CanonicalActivityNameReplacement("exercise_walk", "exercise_cardio_walk")
            )
        )

        assertTrue(result.ok)
        assertEquals("replace_canonical_activity_names", request.getString("action"))
        assertEquals("exercise_walk", request.getJSONArray("replacements")
            .getJSONObject(0).getString("old_canonical"))
        assertEquals("exercise_cardio_walk", request.getJSONArray("replacements")
            .getJSONObject(0).getString("new_canonical"))
        assertEquals("migrated-month", result.updatedContent)
    }
}
