package com.example.tracer

import org.json.JSONObject

internal data class StructuredInsightsWireRecord(
    val logicalId: Long,
    val recordKind: String,
    val startTime: String,
    val endTime: String,
    val activityName: String,
    val durationSeconds: Long,
    val remark: String?,
    val parentColor: String?
)

internal data class StructuredInsightsWirePayload(
    val isDaily: Boolean,
    val date: String,
    val totalDurationSeconds: Long,
    val dayRemark: String,
    val statuses: List<InsightsStatusValue>,
    val records: List<StructuredInsightsWireRecord>,
    val activityDays: List<StructuredInsightsWireActivityDay>,
    val projectTree: List<StructuredInsightsWireProjectNode>
)

internal data class StructuredInsightsWireActivityDay(
    val date: String,
    val totalDurationSeconds: Long,
    val records: List<StructuredInsightsWireRecord>
)

internal data class StructuredInsightsWireProjectNode(
    val name: String,
    val durationSeconds: Long,
    val children: List<StructuredInsightsWireProjectNode>
)

internal class StructuredInsightsJsonDecoder {
    fun decode(rawResponse: String): StructuredInsightsWirePayload {
        val root = JSONObject(rawResponse)
        val insights = root.getJSONObject("insights")
        val isDaily = root.optString("insights_kind", "day") == "day"
        val metadata = insights.optJSONObject("metadata")
        val statuses = buildList {
            val statusesJson = if (isDaily) {
                metadata?.optJSONArray("statuses")
            } else {
                insights.optJSONArray("statuses")
            }
            if (statusesJson != null) {
                for (index in 0 until statusesJson.length()) {
                    val status = statusesJson.optJSONObject(index) ?: continue
                    val id = status.optString("id").trim()
                    if (id.isNotBlank()) {
                        add(
                            InsightsStatusValue(
                                id = id,
                                label = status.optString("label", id),
                                occurrenceCount = status.optInt("occurrence_count", 0),
                                totalDurationSeconds = status.optLong("total_duration", 0L)
                            )
                        )
                    }
                }
            }
        }
        val records = decodeRecords(insights)
        val activityDays = buildList {
            val daysJson = insights.optJSONArray("activity_days")
            if (daysJson != null) {
                for (index in 0 until daysJson.length()) {
                    val day = daysJson.optJSONObject(index) ?: continue
                    add(
                        StructuredInsightsWireActivityDay(
                            date = day.optString("date"),
                            totalDurationSeconds = day.optLong("total_duration", 0L),
                            records = decodeRecords(day)
                        )
                    )
                }
            }
        }
        val projectTree = decodeProjectTree(insights.optJSONObject("project_tree"))
        return StructuredInsightsWirePayload(
            isDaily = isDaily,
            date = insights.optString("date"),
            totalDurationSeconds = insights.optLong("total_duration", 0L),
            dayRemark = metadata?.optString("remark", "").orEmpty(),
            statuses = statuses,
            records = records,
            activityDays = activityDays,
            projectTree = projectTree
        )
    }

    private fun decodeRecords(container: JSONObject): List<StructuredInsightsWireRecord> {
        val recordsJson = container.optJSONArray("detailed_records")
        return buildList {
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
                                .ifBlank { null },
                            parentColor = record.optString("parent_color", "")
                                .trim()
                                .ifBlank { null }
                        )
                    )
                }
            }
        }
    }

    private fun decodeProjectTree(container: JSONObject?): List<StructuredInsightsWireProjectNode> {
        if (container == null) return emptyList()
        return container.keys().asSequence().map { name ->
            val node = container.optJSONObject(name) ?: JSONObject()
            StructuredInsightsWireProjectNode(
                name = name,
                durationSeconds = node.optLong("duration", 0L),
                children = decodeProjectTree(node.optJSONObject("children"))
            )
        }.toList()
    }
}
