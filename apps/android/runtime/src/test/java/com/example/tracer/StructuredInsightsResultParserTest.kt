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

    @Test
    fun parse_mapsPeriodStatusStatistics() {
        val result = parser.parse(
            InsightsCallResult(
                initialized = true,
                operationOk = true,
                outputText = "",
                rawResponse = """{"ok":true,"insights_kind":"period","insights":{"statuses":[{"id":"study","label":"Study","occurrence_count":3,"total_duration":7200},{"id":"exercise","label":"Exercise","occurrence_count":0,"total_duration":0}]}}"""
            )
        )

        assertTrue(result.operationOk)
        assertEquals(null, result.insights)
        assertEquals(
            listOf(
                InsightsStatusValue(id = "study", label = "Study", occurrenceCount = 3, totalDurationSeconds = 7200),
                InsightsStatusValue(id = "exercise", label = "Exercise", occurrenceCount = 0, totalDurationSeconds = 0)
            ),
            result.statuses
        )
    }

    @Test
    fun parse_mapsPeriodActivityDaysAndPreservesEndOnlyRecords() {
        val result = parser.parse(
            InsightsCallResult(
                initialized = true,
                operationOk = true,
                outputText = "",
                rawResponse = """{"ok":true,"insights_kind":"period","insights":{"activity_days":[{"date":"2026-03-20","total_duration":3600,"detailed_records":[{"logical_id":7,"record_kind":"interval","start_time":"09:00","end_time":"10:00","project_path":"study_math","duration_seconds":3600,"activity_remark":""}]},{"date":"2026-03-19","total_duration":0,"detailed_records":[{"logical_id":8,"record_kind":"end_only","start_time":"","end_time":"17:40","project_path":"study_checkpoint","duration_seconds":0,"activity_remark":""}]}]}}"""
            )
        )

        assertTrue(result.operationOk)
        assertEquals(listOf("2026-03-20", "2026-03-19"), result.activityDays.map { it.date })
        assertEquals("study_math", result.activityDays.first().activities.single().activityName)
        assertEquals(
            ActivityTimelineRecordKind.END_ONLY,
            result.activityDays.last().activities.single().kind
        )
    }

    @Test
    fun parse_mapsPeriodProjectTree() {
        val result = parser.parse(
            InsightsCallResult(
                initialized = true,
                operationOk = true,
                outputText = "",
                rawResponse = """{"ok":true,"insights_kind":"period","insights":{"project_tree":{"study":{"duration":7200,"children":{"math":{"duration":5400,"children":{}}}}}}}"""
            )
        )

        assertTrue(result.operationOk)
        assertEquals("study", result.projectTree.single().name)
        assertEquals(7200L, result.projectTree.single().durationSeconds)
        assertEquals("math", result.projectTree.single().children.single().name)
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
