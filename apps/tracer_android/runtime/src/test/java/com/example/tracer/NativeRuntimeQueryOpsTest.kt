package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertNull
import org.junit.Test

class NativeRuntimeQueryOpsTest {
    @Test
    fun parseReportChartContent_parsesCoreStatsFields() {
        val content = """
            {
              "roots": ["sleep", "study"],
              "selected_root": "study",
              "lookback_days": 7,
              "average_duration_seconds": 4500,
              "total_duration_seconds": 31500,
              "active_days": 5,
              "range_days": 7,
              "series": [
                {"date": "2026-02-10", "duration_seconds": 3600},
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
        assertEquals(4500L, data.averageDurationSeconds)
        assertEquals(31500L, data.totalDurationSeconds)
        assertEquals(5, data.activeDays)
        assertEquals(7, data.rangeDays)
        assertEquals(false, data.usesLegacyStatsFallback)
    }

    @Test
    fun parseReportChartContent_missingStatsFields_keepsNullForFallback() {
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
        assertNull(data.averageDurationSeconds)
        assertNull(data.totalDurationSeconds)
        assertNull(data.activeDays)
        assertNull(data.rangeDays)
        assertEquals(true, data.usesLegacyStatsFallback)
    }
}
