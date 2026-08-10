package com.example.tracer

import org.json.JSONObject

internal data class StructuredInsightsWireRecord(
    val logicalId: Long,
    val recordKind: String,
    val startTime: String,
    val endTime: String,
    val activityName: String,
    val durationSeconds: Long,
    val remark: String?
)

internal data class StructuredInsightsWirePayload(
    val date: String,
    val totalDurationSeconds: Long,
    val dayRemark: String,
    val statuses: List<DailyStatusValue>,
    val records: List<StructuredInsightsWireRecord>
)

internal class StructuredInsightsJsonDecoder {
    fun decode(rawResponse: String): StructuredInsightsWirePayload {
        val root = JSONObject(rawResponse)
        val insights = root.getJSONObject("insights")
        val metadata = insights.optJSONObject("metadata")
        val statuses = buildList {
            val statusesJson = metadata?.optJSONArray("statuses")
            if (statusesJson != null) {
                for (index in 0 until statusesJson.length()) {
                    val status = statusesJson.optJSONObject(index) ?: continue
                    val id = status.optString("id").trim()
                    if (id.isNotBlank()) {
                        add(
                            DailyStatusValue(
                                id = id,
                                label = status.optString("label", id),
                                value = status.optBoolean("value", false)
                            )
                        )
                    }
                }
            }
        }
        val recordsJson = insights.optJSONArray("detailed_records")
        val records = buildList {
            if (recordsJson != null) {
                for (index in 0 until recordsJson.length()) {
                    val record = recordsJson.getJSONObject(index)
                    add(
                        StructuredInsightsWireRecord(
                            logicalId = record.getLong("logical_id"),
                            recordKind = record.getString("record_kind"),
                            startTime = record.getString("start_time"),
                            endTime = record.getString("end_time"),
                            activityName = record.getString("project_path"),
                            durationSeconds = record.getLong("duration_seconds"),
                            remark = record.optString("activity_remark", "")
                                .ifBlank { null }
                        )
                    )
                }
            }
        }
        return StructuredInsightsWirePayload(
            date = insights.getString("date"),
            totalDurationSeconds = insights.getLong("total_duration"),
            dayRemark = metadata?.optString("remark", "").orEmpty(),
            statuses = statuses,
            records = records
        )
    }
}
