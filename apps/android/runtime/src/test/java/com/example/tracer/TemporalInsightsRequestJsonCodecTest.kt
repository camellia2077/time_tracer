package com.example.tracer

import org.json.JSONObject
import org.junit.Assert.assertEquals
import org.junit.Test

class TemporalInsightsRequestJsonCodecTest {
    private val codec = TemporalInsightsRequestJsonCodec()

    @Test
    fun encodeQuery_writesTriStateSelectionFields() {
        val json = codec.encodeQuery(
            TemporalInsightsQueryRequest(
                displayMode = InsightsDisplayMode.WEEK,
                selection = TemporalSelectionPayload(
                    kind = TemporalSelectionKind.DATE_RANGE,
                    startDate = "2026-03-02",
                    endDate = "2026-03-08"
                ),
                locale = "zh"
            )
        )

        val payload = JSONObject(json)
        assertEquals("query", payload.getString("operation_kind"))
        assertEquals("week", payload.getString("display_mode"))
        assertEquals("date_range", payload.getString("selection_kind"))
        assertEquals("2026-03-02", payload.getString("start_date"))
        assertEquals("2026-03-08", payload.getString("end_date"))
        assertEquals("markdown", payload.getString("format"))
        assertEquals("zh", payload.getString("locale"))
    }

    @Test
    fun encodeStructuredQuery_writesDaySelectionWithoutMarkdownFormat() {
        val json = codec.encodeStructuredQuery(
            TemporalInsightsQueryRequest(
                displayMode = InsightsDisplayMode.DAY,
                selection = TemporalSelectionPayload(
                    kind = TemporalSelectionKind.SINGLE_DAY,
                    date = "2026-03-08"
                )
            )
        )

        val payload = JSONObject(json)
        assertEquals("structured_query", payload.getString("operation_kind"))
        assertEquals("day", payload.getString("display_mode"))
        assertEquals("single_day", payload.getString("selection_kind"))
        assertEquals("2026-03-08", payload.getString("date"))
    }

    @Test
    fun encodeExport_writesScopeAndRecentDaysList() {
        val json = codec.encodeExport(
            TemporalInsightsExportRequest(
                displayMode = InsightsDisplayMode.RECENT,
                exportScope = InsightsExportScope.BATCH_RECENT_LIST,
                recentDaysList = listOf(7, 14, 30)
            )
        )

        val payload = JSONObject(json)
        assertEquals("export", payload.getString("operation_kind"))
        assertEquals("recent", payload.getString("display_mode"))
        assertEquals("batch_recent_list", payload.getString("export_scope"))
        assertEquals(3, payload.getJSONArray("recent_days_list").length())
    }
}
