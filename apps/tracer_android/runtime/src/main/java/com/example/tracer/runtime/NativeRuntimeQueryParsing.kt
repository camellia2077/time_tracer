package com.example.tracer

import org.json.JSONArray
import org.json.JSONObject
import java.time.LocalDate

internal fun parseReportChartContent(content: String): ReportChartData? {
    if (content.isBlank()) {
        return null
    }

    return try {
        val payload = JSONObject(content)

        val rootsArray = payload.optJSONArray("roots")
        val roots = linkedSetOf<String>()
        if (rootsArray != null) {
            for (index in 0 until rootsArray.length()) {
                val value = rootsArray.optString(index, "").trim()
                if (value.isNotEmpty()) {
                    roots += value
                }
            }
        }

        val selectedRoot = payload.optString("selected_root", "").trim()
        val lookbackDays = payload.optInt("lookback_days", 0)
        val schemaVersion = payload.optNullableInt("schema_version")
        val usesSchemaVersionFallback = schemaVersion == null ||
            schemaVersion != REPORT_CHART_SCHEMA_VERSION_V1
        val averageDurationSeconds = payload.optNullableLong("average_duration_seconds")
        val totalDurationSeconds = payload.optNullableLong("total_duration_seconds")
        val activeDays = payload.optNullableInt("active_days")
        val rangeDays = payload.optNullableInt("range_days")
        val seriesArray = payload.optJSONArray("series")
        val points = mutableListOf<ReportChartPoint>()
        if (seriesArray != null) {
            for (index in 0 until seriesArray.length()) {
                val row = seriesArray.optJSONObject(index) ?: continue
                val date = row.optString("date", "").trim()
                if (date.isEmpty()) {
                    continue
                }
                val durationSeconds = row.optLong("duration_seconds", 0L).coerceAtLeast(0L)
                val epochDay = row.optNullableLong("epoch_day") ?: parseEpochDayOrNull(date)
                points += ReportChartPoint(
                    date = date,
                    durationSeconds = durationSeconds,
                    epochDay = epochDay
                )
            }
        }

        val hasCoreStats = averageDurationSeconds != null &&
            totalDurationSeconds != null &&
            activeDays != null &&
            rangeDays != null
        val fallbackTotalDurationSeconds = points.sumOf { it.durationSeconds }
        val fallbackActiveDays = points.count { it.durationSeconds > 0L }
        val fallbackRangeDays = if (points.isNotEmpty()) {
            points.size
        } else {
            lookbackDays.coerceAtLeast(0)
        }
        val resolvedTotalDurationSeconds =
            totalDurationSeconds?.coerceAtLeast(0L) ?: fallbackTotalDurationSeconds
        val resolvedActiveDays = activeDays?.coerceAtLeast(0) ?: fallbackActiveDays
        val resolvedRangeDays = rangeDays?.coerceAtLeast(0) ?: fallbackRangeDays
        val resolvedAverageDurationSeconds =
            averageDurationSeconds?.coerceAtLeast(0L) ?: if (resolvedRangeDays > 0) {
                resolvedTotalDurationSeconds / resolvedRangeDays
            } else {
                0L
            }

        ReportChartData(
            roots = roots.toList(),
            selectedRoot = selectedRoot,
            lookbackDays = lookbackDays,
            points = points,
            averageDurationSeconds = resolvedAverageDurationSeconds,
            totalDurationSeconds = resolvedTotalDurationSeconds,
            activeDays = resolvedActiveDays,
            rangeDays = resolvedRangeDays,
            usesLegacyStatsFallback = !hasCoreStats,
            schemaVersion = schemaVersion,
            usesSchemaVersionFallback = usesSchemaVersionFallback
        )
    } catch (_: Exception) {
        null
    }
}

internal fun parseTreeQueryContent(content: String): ParsedTreeQueryPayload? {
    if (content.isBlank()) {
        return null
    }

    return try {
        val payload = JSONObject(content)
        val roots = parseStringArray(payload.optJSONArray("roots"))
        val nodes = parseTreeNodes(payload.optJSONArray("nodes"))
        ParsedTreeQueryPayload(
            ok = payload.optBoolean("ok", false),
            found = payload.optBoolean("found", true),
            roots = roots,
            nodes = nodes,
            errorMessage = payload.optString("error_message", "")
        )
    } catch (_: Exception) {
        null
    }
}

internal fun parseSuggestedActivities(content: String): List<String> {
    val activities = mutableListOf<String>()
    for (rawLine in content.lineSequence()) {
        val line = rawLine.trim()
        if (line.isEmpty() || line.startsWith("Total:")) {
            continue
        }
        val activity = line.substringBefore("|").trim()
        if (activity.isNotEmpty()) {
            activities += activity
        }
    }
    return activities
}

internal fun parseMappingNamesContent(content: String): List<String> {
    if (content.isBlank()) {
        return emptyList()
    }

    return try {
        val payload = JSONObject(content)
        val namesArray = payload.optJSONArray("names") ?: return emptyList()
        val unique = linkedSetOf<String>()
        for (index in 0 until namesArray.length()) {
            val name = namesArray.optString(index, "").trim()
            if (name.isNotEmpty()) {
                unique += name
            }
        }
        unique.toList()
    } catch (_: Exception) {
        emptyList()
    }
}

internal fun normalizeSuggestedActivities(
    activities: List<String>,
    validActivityNames: Set<String>,
    maxItems: Int
): List<String> {
    val unique = linkedSetOf<String>()
    for (activity in activities) {
        val normalized = activity.trim()
        if (normalized.isEmpty()) {
            continue
        }
        if (validActivityNames.isNotEmpty() && !validActivityNames.contains(normalized)) {
            continue
        }
        unique += normalized
        if (unique.size >= maxItems) {
            break
        }
    }
    return unique.toList()
}

internal fun countTreeNodes(nodes: List<TreeNode>): Int {
    var total = 0
    for (node in nodes) {
        total += 1
        total += countTreeNodes(node.children)
    }
    return total
}

private fun parseStringArray(jsonArray: JSONArray?): List<String> {
    if (jsonArray == null) {
        return emptyList()
    }
    val values = linkedSetOf<String>()
    for (index in 0 until jsonArray.length()) {
        val value = jsonArray.optString(index, "").trim()
        if (value.isNotEmpty()) {
            values += value
        }
    }
    return values.toList()
}

private fun parseTreeNodes(jsonArray: JSONArray?): List<TreeNode> {
    if (jsonArray == null) {
        return emptyList()
    }
    val nodes = mutableListOf<TreeNode>()
    for (index in 0 until jsonArray.length()) {
        val node = jsonArray.optJSONObject(index) ?: continue
        parseTreeNode(node)?.let(nodes::add)
    }
    return nodes
}

private fun parseTreeNode(node: JSONObject): TreeNode? {
    val name = node.optString("name", "").trim()
    if (name.isEmpty()) {
        return null
    }
    val path = node.optString("path", "").trim()
    val durationSeconds = node.optNullableLong("duration_seconds")
    val children = parseTreeNodes(node.optJSONArray("children"))
    return TreeNode(
        name = name,
        path = path,
        durationSeconds = durationSeconds,
        children = children
    )
}

private fun JSONObject.optNullableLong(fieldName: String): Long? {
    if (!has(fieldName) || isNull(fieldName)) {
        return null
    }
    val raw = opt(fieldName)
    return if (raw is Number) raw.toLong() else null
}

private fun JSONObject.optNullableInt(fieldName: String): Int? {
    if (!has(fieldName) || isNull(fieldName)) {
        return null
    }
    val raw = opt(fieldName)
    return if (raw is Number) raw.toInt() else null
}

private fun parseEpochDayOrNull(dateIso: String): Long? =
    try {
        LocalDate.parse(dateIso).toEpochDay()
    } catch (_: Exception) {
        null
    }
