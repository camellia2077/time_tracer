package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertNull
import org.junit.Test

class NativeRuntimeQueryOpsTest {
    @Test
    fun parseSemanticListContent_parsesCoreCalendarLists() {
        val years = parseSemanticListContent(
            """{"schema_version":1,"action":"years","output_mode":"semantic_json","items":["2024","2025"],"total_count":2}""",
            "years"
        )
        val months = parseSemanticListContent(
            """{"schema_version":1,"action":"months","output_mode":"semantic_json","items":["2025-01","2025-03"],"total_count":2}""",
            "months"
        )

        assertEquals(listOf("2024", "2025"), years)
        assertEquals(listOf("2025-01", "2025-03"), months)
    }

    @Test
    fun parseSemanticListContent_wrongActionOrMode_returnsNull() {
        assertNull(
            parseSemanticListContent(
                """{"action":"months","output_mode":"semantic_json","items":["2025-01"]}""",
                "years"
            )
        )
        assertNull(
            parseSemanticListContent(
                """{"action":"years","output_mode":"text","items":["2025"]}""",
                "years"
            )
        )
    }

    @Test
    fun parseTreeQueryContent_parsesStructuredTreePayload() {
        val content = """
            {
              "ok": true,
              "found": true,
              "error_message": "",
              "roots": ["study", "sleep"],
              "nodes": [
                {
                  "name": "study",
                  "path": "study",
                  "duration_seconds": 7200,
                  "children": [
                    {
                      "name": "math",
                      "path": "study_math",
                      "duration_seconds": 3600,
                      "parent_duration_percent": 50.0,
                      "children": []
                    }
                  ]
                }
              ]
            }
        """.trimIndent()

        val parsed = parseTreeQueryContent(content)
        assertNotNull(parsed)
        val payload = checkNotNull(parsed)

        assertEquals(true, payload.ok)
        assertEquals(true, payload.found)
        assertEquals(listOf("study", "sleep"), payload.roots)
        assertEquals(1, payload.nodes.size)
        assertEquals("study", payload.nodes[0].name)
        assertEquals("study", payload.nodes[0].path)
        assertEquals(7200L, payload.nodes[0].durationSeconds)
        assertEquals(1, payload.nodes[0].children.size)
        assertEquals("math", payload.nodes[0].children[0].name)
        assertEquals("study_math", payload.nodes[0].children[0].path)
        assertEquals(3600L, payload.nodes[0].children[0].durationSeconds)
        assertEquals(50f, payload.nodes[0].children[0].parentDurationPercent)
    }

    @Test
    fun parseTreeQueryContent_invalidPayload_returnsNull() {
        val parsed = parseTreeQueryContent("{bad json")
        assertNull(parsed)
    }

    @Test
    fun parseTreeQueryContent_parsesSemanticTreePayload() {
        val content = """
            {
              "schema_version": 1,
              "action": "tree",
              "output_mode": "semantic_json",
              "max_available_depth": 2,
              "roots": [
                {
                  "name": "study",
                  "duration_seconds": 7200,
                  "children": [
                    {
                      "name": "math",
                      "duration_seconds": 3600,
                      "parent_duration_percent": 50.0,
                      "children": []
                    }
                  ]
                }
              ]
            }
        """.trimIndent()

        val payload = checkNotNull(parseTreeQueryContent(content))

        assertEquals(true, payload.ok)
        assertEquals(true, payload.found)
        assertEquals(2, payload.maxAvailableDepth)
        assertEquals(listOf("study"), payload.roots)
        assertEquals("study", payload.nodes[0].path)
        assertEquals(7200L, payload.nodes[0].durationSeconds)
        assertEquals("study_math", payload.nodes[0].children[0].path)
        assertEquals(50f, payload.nodes[0].children[0].parentDurationPercent)
    }

    @Test
    fun parseReportChartContent_parsesCoreStatsFields() {
        val content = """
            {
              "schema_version": 1,
              "roots": ["sleep", "study"],
              "selected_root": "study",
              "lookback_days": 7,
              "average_duration_seconds": 4500,
              "total_duration_seconds": 31500,
              "active_days": 5,
              "range_days": 7,
              "series": [
                {"date": "2026-02-10", "duration_seconds": 3600, "epoch_day": 20494},
                {"date": "2026-02-11", "duration_seconds": 5400}
              ]
            }
        """.trimIndent()

        val parsed = parseReportChartContent(content)
        assertNotNull(parsed)
        val data = checkNotNull(parsed)

        assertEquals(listOf("sleep", "study"), data.roots)
        assertEquals("study", data.selectedRoot)
        assertEquals(7, data.lookbackDays)
        assertEquals(2, data.points.size)
        assertEquals("2026-02-10", data.points[0].date)
        assertEquals(3600L, data.points[0].durationSeconds)
        assertEquals(20494L, data.points[0].epochDay)
        assertEquals(20495L, data.points[1].epochDay)
        assertEquals(4500L, data.averageDurationSeconds)
        assertEquals(31500L, data.totalDurationSeconds)
        assertEquals(5, data.activeDays)
        assertEquals(7, data.rangeDays)
        assertEquals(false, data.usesLegacyStatsFallback)
        assertEquals(1, data.schemaVersion)
        assertEquals(false, data.usesSchemaVersionFallback)
    }

    @Test
    fun parseReportChartContent_missingStatsFields_resolvesFallbackStats() {
        val content = """
            {
              "roots": ["sleep", "", "sleep"],
              "selected_root": "",
              "lookback_days": 14,
              "series": [
                {"date": "2026-02-10", "duration_seconds": 0},
                {"date": "", "duration_seconds": 100}
              ]
            }
        """.trimIndent()

        val parsed = parseReportChartContent(content)
        assertNotNull(parsed)
        val data = checkNotNull(parsed)

        assertEquals(listOf("sleep"), data.roots)
        assertEquals("", data.selectedRoot)
        assertEquals(14, data.lookbackDays)
        assertEquals(1, data.points.size)
        assertEquals("2026-02-10", data.points[0].date)
        assertEquals(0L, data.points[0].durationSeconds)
        assertEquals(20494L, data.points[0].epochDay)
        assertEquals(0L, data.averageDurationSeconds)
        assertEquals(0L, data.totalDurationSeconds)
        assertEquals(0, data.activeDays)
        assertEquals(1, data.rangeDays)
        assertEquals(true, data.usesLegacyStatsFallback)
        assertNull(data.schemaVersion)
        assertEquals(true, data.usesSchemaVersionFallback)
    }

    @Test
    fun parseReportChartContent_newerSchemaVersion_marksCompatibilityFallback() {
        val content = """
            {
              "schema_version": 2,
              "roots": ["study"],
              "selected_root": "study",
              "lookback_days": 7,
              "series": [
                {"date": "2026-02-10", "duration_seconds": 1200}
              ]
            }
        """.trimIndent()

        val parsed = parseReportChartContent(content)
        assertNotNull(parsed)
        val data = checkNotNull(parsed)

        assertEquals(2, data.schemaVersion)
        assertEquals(true, data.usesSchemaVersionFallback)
    }

    @Test
    fun parseReportChartContent_missingStatsAndSeries_usesLookbackForRangeFallback() {
        val content = """
            {
              "roots": ["study"],
              "selected_root": "study",
              "lookback_days": 21,
              "series": []
            }
        """.trimIndent()

        val parsed = parseReportChartContent(content)
        assertNotNull(parsed)
        val data = checkNotNull(parsed)

        assertEquals(0L, data.averageDurationSeconds)
        assertEquals(0L, data.totalDurationSeconds)
        assertEquals(0, data.activeDays)
        assertEquals(21, data.rangeDays)
        assertEquals(true, data.usesLegacyStatsFallback)
    }

    @Test
    fun parseReportChartContent_missingStats_usesActiveDaysAsAverageDenominator() {
        val content = """
            {
              "roots": ["study"],
              "selected_root": "study",
              "lookback_days": 7,
              "series": [
                {"date": "2026-02-10", "duration_seconds": 3600},
                {"date": "2026-02-11", "duration_seconds": 0}
              ]
            }
        """.trimIndent()

        val parsed = parseReportChartContent(content)
        assertNotNull(parsed)
        val data = checkNotNull(parsed)

        assertEquals(3600L, data.totalDurationSeconds)
        assertEquals(1, data.activeDays)
        assertEquals(2, data.rangeDays)
        assertEquals(3600L, data.averageDurationSeconds)
    }

    @Test
    fun parseReportCompositionContent_parsesWeightedTree() {
        val content = """
            {
              "total_duration_seconds": 9000,
              "active_root_count": 3,
              "active_days": 4,
              "range_days": 7,
              "display_level": 1,
              "display_path": ["study"],
              "tree": [
                {
                  "name": "study",
                  "duration_seconds": 5400,
                  "occurrence_count": 3,
                  "average_duration_seconds": 1350,
                  "average_occurrence_count": 0.75,
                  "average_occurrence_ratio": 0.75,
                  "children": [
                    {"name": "math", "duration_seconds": 5400, "occurrence_count": 3,
                     "average_duration_seconds": 1350, "average_occurrence_ratio": 1.0,
                     "children": []}
                  ]
                },
                {"name": "sleep", "duration_seconds": 3600, "children": []}
              ]
            }
        """.trimIndent()

        val parsed = parseReportCompositionContent(content)
        assertNotNull(parsed)
        val data = checkNotNull(parsed)

        assertEquals(9000L, data.totalDurationSeconds)
        assertEquals(3, data.activeRootCount)
        assertEquals(4, data.activeDays)
        assertEquals(7, data.rangeDays)
        assertEquals(1, data.displayLevel)
        assertEquals(listOf("study"), data.displayPath)
        assertEquals(2, data.tree.size)
        assertEquals("study", data.tree.first().name)
        assertEquals(3L, data.tree.first().occurrenceCount)
        assertEquals(1_350L, data.tree.first().averageDurationSeconds)
        assertEquals(0.75, data.tree.first().averageOccurrenceCount)
        assertEquals(0.75, data.tree.first().averageOccurrenceRatio)
        assertEquals("math", data.tree.first().children.single().name)
    }
}
