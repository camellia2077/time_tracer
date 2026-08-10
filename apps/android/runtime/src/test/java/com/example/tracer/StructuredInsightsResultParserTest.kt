package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class StructuredInsightsResultParserTest {
    private val parser = StructuredInsightsResultParser()

    @Test
    fun parse_mapsCoreRecordKindToTimelineModel() {
        val result = parser.parse(
            InsightsCallResult(
                initialized = true,
                operationOk = true,
                outputText = "",
                rawResponse = structuredResponse("end_only")
            )
        )

        assertTrue(result.operationOk)
        assertEquals(
            ActivityTimelineRecordKind.END_ONLY,
            result.insights?.activities?.single()?.kind
        )
        assertEquals(0L, result.insights?.activities?.single()?.durationSeconds)
    }

    @Test
    fun parse_rejectsStructuredRecordWithoutRecordKind() {
        val result = parser.parse(
            InsightsCallResult(
                initialized = true,
                operationOk = true,
                outputText = "",
                rawResponse = structuredResponse(null)
            )
        )

        assertTrue(!result.operationOk)
        assertTrue(result.errorMessage.isNotBlank())
    }

    private fun structuredResponse(recordKind: String?): String {
        val recordFields = buildString {
            append("\"logical_id\":1,")
            recordKind?.let { append("\"record_kind\":\"$it\",") }
            append("\"start_time\":\"\",")
            append("\"end_time\":\"17:40:30\",")
            append("\"project_path\":\"study_math\",")
            append("\"duration_seconds\":0,")
            append("\"activity_remark\":\"\"")
        }
        return """
            {"ok":true,"insights":{"date":"2026-03-20","total_duration":0,
            "metadata":{"remark":""},"detailed_records":[{$recordFields}]}}
        """.trimIndent().replace("\n", "")
    }
}
