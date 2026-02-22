package com.example.tracer

import org.json.JSONObject
import java.time.LocalDate
import java.time.format.DateTimeParseException

internal data class DataQueryRequest(
    val action: Int,
    val outputMode: String? = null,
    val year: Int? = null,
    val month: Int? = null,
    val fromDateIso: String? = null,
    val toDateIso: String? = null,
    val remark: String? = null,
    val dayRemark: String? = null,
    val project: String? = null,
    val root: String? = null,
    val exercise: Int? = null,
    val status: Int? = null,
    val overnight: Boolean = false,
    val reverse: Boolean = false,
    val limit: Int? = null,
    val topN: Int? = null,
    val lookbackDays: Int? = null,
    val scoreByDuration: Boolean = false,
    val treePeriod: String? = null,
    val treePeriodArgument: String? = null,
    val treeMaxDepth: Int? = null
)

internal object DataQueryOutputMode {
    const val TEXT: String = "text"
    const val SEMANTIC_JSON: String = "semantic_json"
}

internal data class PeriodArgumentValidationResult(
    val argument: String,
    val error: DataQueryTextResult?
)

internal fun validateReportChartQueryParams(
    lookbackDays: Int,
    fromDateIso: String?,
    toDateIso: String?
): ReportChartQueryResult? {
    val hasFrom = !fromDateIso.isNullOrBlank()
    val hasTo = !toDateIso.isNullOrBlank()
    if (hasFrom != hasTo) {
        return ReportChartQueryResult(
            ok = false,
            data = null,
            message = "fromDateIso and toDateIso must be provided together."
        )
    }

    if (hasFrom && hasTo) {
        val start = parseIsoDateOrNull(fromDateIso)
            ?: return ReportChartQueryResult(
                ok = false,
                data = null,
                message = "fromDateIso must be YYYY-MM-DD."
            )
        val end = parseIsoDateOrNull(toDateIso)
            ?: return ReportChartQueryResult(
                ok = false,
                data = null,
                message = "toDateIso must be YYYY-MM-DD."
            )
        if (start.isAfter(end)) {
            return ReportChartQueryResult(
                ok = false,
                data = null,
                message = "fromDateIso must be less than or equal to toDateIso."
            )
        }
        return null
    }

    if (lookbackDays <= 0) {
        return ReportChartQueryResult(
            ok = false,
            data = null,
            message = "lookbackDays must be greater than 0."
        )
    }
    return null
}

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
        val averageDurationSeconds = payload.optNullableLong("average_duration_seconds")
        val totalDurationSeconds = payload.optNullableLong("total_duration_seconds")
        val activeDays = payload.optNullableInt("active_days")
        val rangeDays = payload.optNullableInt("range_days")
        val hasCoreStats = averageDurationSeconds != null &&
            totalDurationSeconds != null &&
            activeDays != null &&
            rangeDays != null

        val seriesArray = payload.optJSONArray("series")
        val points = mutableListOf<ReportChartPoint>()
        if (seriesArray != null) {
            for (index in 0 until seriesArray.length()) {
                val row = seriesArray.optJSONObject(index) ?: continue
                val date = row.optString("date", "").trim()
                if (date.isEmpty()) {
                    continue
                }
                val durationSeconds = row.optLong("duration_seconds", 0L)
                points += ReportChartPoint(
                    date = date,
                    durationSeconds = durationSeconds
                )
            }
        }

        ReportChartData(
            roots = roots.toList(),
            selectedRoot = selectedRoot,
            lookbackDays = lookbackDays,
            points = points,
            averageDurationSeconds = averageDurationSeconds,
            totalDurationSeconds = totalDurationSeconds,
            activeDays = activeDays,
            rangeDays = rangeDays,
            usesLegacyStatsFallback = !hasCoreStats
        )
    } catch (_: Exception) {
        null
    }
}

internal fun buildReportChartResultMessage(
    pointCount: Int
): String {
    return if (pointCount <= 0) {
        "No chart points."
    } else {
        "Loaded $pointCount chart point(s)."
    }
}

internal fun normalizePeriodArgument(periodArgument: String?): String {
    return periodArgument?.trim().orEmpty()
}

internal fun validatePeriodArgument(
    period: DataTreePeriod,
    normalizedPeriodArgument: String
): DataQueryTextResult? {
    if (period == DataTreePeriod.RECENT || normalizedPeriodArgument.isNotBlank()) {
        return null
    }
    return DataQueryTextResult(
        ok = false,
        outputText = "",
        message = "periodArgument is required for period ${period.wireValue}."
    )
}

internal fun validateAndNormalizePeriodArgument(
    period: DataTreePeriod,
    periodArgument: String?
): PeriodArgumentValidationResult {
    val normalizedArgument = normalizePeriodArgument(periodArgument)
    val validationError = validatePeriodArgument(period, normalizedArgument)
    return PeriodArgumentValidationResult(
        argument = normalizedArgument,
        error = validationError
    )
}

internal fun extractNativeInitFailureMessage(
    rawResponse: String,
    responseCodec: NativeResponseCodec
): String {
    return responseCodec.parse(rawResponse).errorMessage.ifEmpty { "native init failed." }
}

internal fun validateSuggestionQueryParams(
    lookbackDays: Int,
    topN: Int
): ActivitySuggestionResult? {
    if (lookbackDays <= 0) {
        return ActivitySuggestionResult(
            ok = false,
            suggestions = emptyList(),
            message = "lookbackDays must be greater than 0."
        )
    }
    if (topN <= 0) {
        return ActivitySuggestionResult(
            ok = false,
            suggestions = emptyList(),
            message = "topN must be greater than 0."
        )
    }
    return null
}

internal fun buildSuggestionResultMessage(
    suggestions: List<String>,
    lookbackDays: Int
): String {
    return if (suggestions.isEmpty()) {
        "No activity suggestions in recent $lookbackDays days."
    } else {
        "Loaded ${suggestions.size} activity suggestion(s)."
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

private fun parseIsoDateOrNull(value: String?): LocalDate? {
    if (value.isNullOrBlank()) {
        return null
    }
    return try {
        LocalDate.parse(value)
    } catch (_: DateTimeParseException) {
        null
    }
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
