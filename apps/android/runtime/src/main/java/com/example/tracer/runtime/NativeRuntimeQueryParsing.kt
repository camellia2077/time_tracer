package com.example.tracer

import android.util.Log
import org.json.JSONArray
import org.json.JSONObject
import java.time.LocalDate

internal fun parseInsightsChartContent(content: String): InsightsChartData? {
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
            schemaVersion != INSIGHTS_CHART_SCHEMA_VERSION_V1
        val averageDurationSeconds = payload.optNullableLong("average_duration_seconds")
        val totalDurationSeconds = payload.optNullableLong("total_duration_seconds")
        val activeDays = payload.optNullableInt("active_days")
        val rangeDays = payload.optNullableInt("range_days")
        val averageDayBasis = when (payload.optString("average_day_basis", "active_days")) {
            "calendar_days" -> InsightsAverageDayBasis.CALENDAR_DAYS
            else -> InsightsAverageDayBasis.ACTIVE_DAYS
        }
        val averageDenominatorDays = payload.optNullableInt("average_denominator_days")
        val seriesArray = payload.optJSONArray("series")
        val points = mutableListOf<InsightsChartPoint>()
        if (seriesArray != null) {
            for (index in 0 until seriesArray.length()) {
                val row = seriesArray.optJSONObject(index) ?: continue
                val date = row.optString("date", "").trim()
                if (date.isEmpty()) {
                    continue
                }
                val durationSeconds = row.optLong("duration_seconds", 0L).coerceAtLeast(0L)
                val epochDay = row.optNullableLong("epoch_day") ?: parseEpochDayOrNull(date)
                points += InsightsChartPoint(
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
            averageDurationSeconds?.coerceAtLeast(0L) ?: if ((averageDenominatorDays
                    ?: if (averageDayBasis == InsightsAverageDayBasis.CALENDAR_DAYS) {
                        resolvedRangeDays
                    } else {
                        resolvedActiveDays
                    }) > 0
            ) {
                resolvedTotalDurationSeconds / (averageDenominatorDays
                    ?: if (averageDayBasis == InsightsAverageDayBasis.CALENDAR_DAYS) {
                        resolvedRangeDays
                    } else {
                        resolvedActiveDays
                    })
            } else {
                0L
            }

        InsightsChartData(
            roots = roots.toList(),
            selectedRoot = selectedRoot,
            lookbackDays = lookbackDays,
            points = points,
            averageDurationSeconds = resolvedAverageDurationSeconds,
            totalDurationSeconds = resolvedTotalDurationSeconds,
            activeDays = resolvedActiveDays,
            rangeDays = resolvedRangeDays,
            averageDayBasis = averageDayBasis,
            averageDenominatorDays = averageDenominatorDays,
            usesLegacyStatsFallback = !hasCoreStats,
            schemaVersion = schemaVersion,
            usesSchemaVersionFallback = usesSchemaVersionFallback
        )
    } catch (_: Exception) {
        null
    }
}

internal fun parseInsightsCompositionContent(content: String): InsightsCompositionData? {
    if (content.isBlank()) {
        logInsightsCompositionWarning("Insights composition payload is blank")
        return null
    }

    return try {
        val payload = JSONObject(content)
        val totalDurationSeconds = payload.optLong("total_duration_seconds", 0L).coerceAtLeast(0L)
        val activeRootCount = payload.optInt("active_root_count", 0).coerceAtLeast(0)
        val activeDays = payload.optInt("active_days", 0).coerceAtLeast(0)
        val rangeDays = payload.optInt("range_days", 0).coerceAtLeast(0)
        val averageDayBasis = when (payload.optString("average_day_basis", "active_days")) {
            "calendar_days" -> InsightsAverageDayBasis.CALENDAR_DAYS
            else -> InsightsAverageDayBasis.ACTIVE_DAYS
        }
        val averageDenominatorDays = payload.optInt("average_denominator_days", 0)
        val displayLevel = payload.optInt("display_level", 0).coerceAtLeast(0)
        val displayPath = parseStringArray(payload.optJSONArray("display_path"))
        val tree = parseTreeNodes(payload.optJSONArray("tree") ?: return null)

        val occurrenceFieldNodeCount = tree.countNodes { it.occurrenceCount != null }
        val positiveOccurrenceNodeCount = tree.countNodes {
            (it.occurrenceCount ?: 0L) > 0L
        }
        logInsightsCompositionInfo(
            "Parsed insights composition: roots=${tree.size}, " +
                "nodes=${countTreeNodes(tree)}, " +
                "occurrenceFieldNodes=$occurrenceFieldNodeCount, " +
                "positiveOccurrenceNodes=$positiveOccurrenceNodeCount"
        )

        InsightsCompositionData(
            totalDurationSeconds = totalDurationSeconds,
            activeRootCount = activeRootCount,
            activeDays = activeDays,
            rangeDays = rangeDays,
            averageDayBasis = averageDayBasis,
            averageDenominatorDays = averageDenominatorDays,
            displayLevel = displayLevel,
            displayPath = displayPath,
            tree = tree
        )
    } catch (error: Exception) {
        logInsightsCompositionWarning("Invalid insights composition payload", error)
        null
    }
}

internal fun parseTreeQueryContent(content: String): ParsedTreeQueryPayload? {
    if (content.isBlank()) {
        return null
    }

    return try {
        val payload = JSONObject(content)
        val legacyNodes = payload.optJSONArray("nodes")
        val isSemanticTree = legacyNodes == null && payload.optString("action") == "tree"
        val nodes = if (isSemanticTree) {
            parseTreeNodes(payload.optJSONArray("roots"))
        } else {
            parseTreeNodes(legacyNodes)
        }
        val roots = if (isSemanticTree) {
            nodes.map(TreeNode::name)
        } else {
            parseStringArray(payload.optJSONArray("roots"))
        }
        ParsedTreeQueryPayload(
            ok = if (isSemanticTree) true else payload.optBoolean("ok", false),
            found = if (isSemanticTree) nodes.isNotEmpty() else payload.optBoolean("found", true),
            roots = roots,
            nodes = nodes,
            errorMessage = payload.optString("error_message", ""),
            maxAvailableDepth = payload.optInt("max_available_depth", 0).coerceAtLeast(0)
        )
    } catch (_: Exception) {
        null
    }
}

internal fun parseFrequentActivities(content: String): List<String> {
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

internal fun parseSemanticListContent(content: String, expectedAction: String): List<String>? {
    if (content.isBlank()) {
        return null
    }

    return try {
        val payload = JSONObject(content)
        if (payload.optString("action") != expectedAction ||
            payload.optString("output_mode") != DataQueryOutputMode.SEMANTIC_JSON
        ) {
            return null
        }
        val items = payload.optJSONArray("items") ?: return null
        parseStringArray(items)
    } catch (_: Exception) {
        null
    }
}

internal fun parseActivityHierarchyLeafMappingsContent(content: String): List<ActivityHierarchyLeafMappingEntry> {
    if (content.isBlank()) {
        return emptyList()
    }

    return try {
        val payload = JSONObject(content)
        val entriesArray = payload.optJSONArray("entries") ?: return emptyList()
        val entries = mutableListOf<ActivityHierarchyLeafMappingEntry>()
        for (index in 0 until entriesArray.length()) {
            val entry = entriesArray.optJSONObject(index) ?: continue
            val alias = entry.optString("alias", "").trim()
            val canonical = entry.optString("canonical", "").trim()
            if (alias.isEmpty() || canonical.isEmpty()) {
                continue
            }
            entries += ActivityHierarchyLeafMappingEntry(
                alias = alias,
                canonical = canonical
            )
        }
        entries
    } catch (_: Exception) {
        emptyList()
    }
}

internal fun normalizeFrequentActivities(
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

private fun parseTreeNodes(
    jsonArray: JSONArray?,
    parentPath: String = ""
): List<TreeNode> {
    if (jsonArray == null) {
        return emptyList()
    }
    val nodes = mutableListOf<TreeNode>()
    for (index in 0 until jsonArray.length()) {
        val node = jsonArray.optJSONObject(index) ?: continue
        parseTreeNode(node, parentPath)?.let(nodes::add)
    }
    return nodes
}

private fun parseTreeNode(node: JSONObject, parentPath: String): TreeNode? {
    val name = node.optString("name", "").trim()
    if (name.isEmpty()) {
        return null
    }
    val path = node.optString("path", "").trim().ifEmpty {
        listOf(parentPath, name).filter { it.isNotBlank() }.joinToString("_")
    }
    val durationSeconds = node.optNullableLong("duration_seconds")
    val occurrenceCount = node.optNullableLong("occurrence_count")
    val averageDurationSeconds = node.optNullableLong("average_duration_seconds")
    val averageOccurrenceCount = node.optNullableDouble("average_occurrence_count")
        ?.takeIf { it.isFinite() }
        ?.coerceAtLeast(0.0)
    val averageOccurrenceRatio = node.optNullableDouble("average_occurrence_ratio")
        ?.takeIf { it.isFinite() }
        ?.coerceIn(0.0, 1.0)
    val parentDurationPercent = node.optNullableDouble("parent_duration_percent")
        ?.toFloat()
        ?.takeIf { it.isFinite() }
    val children = parseTreeNodes(node.optJSONArray("children"), path)
    return TreeNode(
        name = name,
        path = path,
        durationSeconds = durationSeconds,
        occurrenceCount = occurrenceCount,
        averageDurationSeconds = averageDurationSeconds,
        averageOccurrenceCount = averageOccurrenceCount,
        averageOccurrenceRatio = averageOccurrenceRatio,
        parentDurationPercent = parentDurationPercent,
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

private fun List<TreeNode>.countNodes(predicate: (TreeNode) -> Boolean): Int = sumOf { node ->
    (if (predicate(node)) 1 else 0) + node.children.countNodes(predicate)
}

private const val INSIGHTS_COMPOSITION_LOG_TAG = "TracerComposition"

private fun logInsightsCompositionInfo(message: String) {
    runCatching { Log.i(INSIGHTS_COMPOSITION_LOG_TAG, message) }
}

private fun logInsightsCompositionWarning(message: String, error: Throwable? = null) {
    runCatching {
        if (error == null) {
            Log.w(INSIGHTS_COMPOSITION_LOG_TAG, message)
        } else {
            Log.w(INSIGHTS_COMPOSITION_LOG_TAG, message, error)
        }
    }
}

private fun JSONObject.optNullableDouble(fieldName: String): Double? {
    if (!has(fieldName) || isNull(fieldName)) {
        return null
    }
    val raw = opt(fieldName)
    return if (raw is Number) raw.toDouble() else null
}

private fun parseEpochDayOrNull(dateIso: String): Long? =
    try {
        LocalDate.parse(dateIso).toEpochDay()
    } catch (_: Exception) {
        null
    }
