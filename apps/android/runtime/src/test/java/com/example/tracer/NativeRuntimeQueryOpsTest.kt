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
    fun parseInsightsChartContent_parsesCoreStatsFields() {
        val content = """
            {
              "schema_version": 1,
              "roots": ["sleep", "study"],
              "selected_root": "study",
              "lookback_days": 7,
              "average_duration_seconds": 4500,
              "total_occurrence_count": 7,
              "average_duration_per_occurrence_seconds": 4500,
              "mode_duration_seconds": null,
              "median_duration_seconds": 4500.0,
              "minimum_duration_seconds": 0.0,
              "maximum_duration_seconds": 7200.0,
              "total_duration_seconds": 31500,
              "active_days": 5,
              "range_days": 7,
              "series": [
                {"date": "2026-02-10", "duration_seconds": 3600, "epoch_day": 20494},
                {"date": "2026-02-11", "duration_seconds": 5400}
              ]
            }
        """.trimIndent()

        val parsed = parseInsightsChartContent(content)
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
        assertEquals(7L, data.totalOccurrenceCount)
        assertEquals(4500L, data.averageDurationPerOccurrenceSeconds)
        assertNull(data.modeDurationSeconds)
        assertEquals(4500.0, data.medianDurationSeconds)
        assertEquals(0.0, data.minimumDurationSeconds)
        assertEquals(7200.0, data.maximumDurationSeconds)
        assertEquals(31500L, data.totalDurationSeconds)
        assertEquals(5, data.activeDays)
        assertEquals(7, data.rangeDays)
        assertEquals(1, data.schemaVersion)
        assertEquals(false, data.usesSchemaVersionFallback)
    }

    @Test
    fun parseInsightsChartContent_missingStatsFields_returnsNull() {
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

        val parsed = parseInsightsChartContent(content)
        assertNull(parsed)
    }

    @Test
    fun parseInsightsChartContent_newerSchemaVersion_marksCompatibilityFallback() {
        val content = """
            {
              "schema_version": 2,
              "roots": ["study"],
              "selected_root": "study",
              "lookback_days": 7,
              "average_duration_seconds": 1200,
              "total_occurrence_count": 1,
              "average_duration_per_occurrence_seconds": 1200,
              "total_duration_seconds": 1200,
              "active_days": 1,
              "range_days": 7,
              "series": [
                {"date": "2026-02-10", "duration_seconds": 1200}
              ]
            }
        """.trimIndent()

        val parsed = parseInsightsChartContent(content)
        assertNotNull(parsed)
        val data = checkNotNull(parsed)

        assertEquals(2, data.schemaVersion)
        assertEquals(true, data.usesSchemaVersionFallback)
    }

    @Test
    fun parseInsightsChartContent_missingStatsAndSeries_returnsNull() {
        val content = """
            {
              "roots": ["study"],
              "selected_root": "study",
              "lookback_days": 21,
              "series": []
            }
        """.trimIndent()

        val parsed = parseInsightsChartContent(content)
        assertNull(parsed)
    }

    @Test
    fun parseInsightsChartContent_missingStats_returnsNull() {
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

        val parsed = parseInsightsChartContent(content)
        assertNull(parsed)
    }

    @Test
    fun parseInsightsCompositionContent_parsesWeightedTree() {
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
                  "average_duration_per_occurrence_seconds": 1800,
                  "average_occurrence_count": 0.75,
                  "average_occurrence_ratio": 0.75,
                  "children": [
                    {"name": "math", "duration_seconds": 5400, "occurrence_count": 3,
                     "average_duration_seconds": 1350,
                     "average_duration_per_occurrence_seconds": 1800,
                     "average_occurrence_ratio": 1.0,
                     "children": []}
                  ]
                },
                {"name": "sleep", "duration_seconds": 3600, "children": []}
              ]
            }
        """.trimIndent()

        val parsed = parseInsightsCompositionContent(content)
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
        assertEquals(1_800L, data.tree.first().averageDurationPerOccurrenceSeconds)
        assertEquals(0.75, data.tree.first().averageOccurrenceCount)
        assertEquals(0.75, data.tree.first().averageOccurrenceRatio)
        assertEquals("math", data.tree.first().children.single().name)
    }
}
