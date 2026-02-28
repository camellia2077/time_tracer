package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertNull
import org.junit.Test

class NativeRuntimeQueryOpsTest {
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
    }

    @Test
    fun parseTreeQueryContent_invalidPayload_returnsNull() {
        val parsed = parseTreeQueryContent("{bad json")
        assertNull(parsed)
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
    fun parseReportChartContent_missingStats_usesRangeDaysAsAverageDenominator() {
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
        assertEquals(1800L, data.averageDurationSeconds)
    }
}
