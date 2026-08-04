package com.example.tracer

import android.util.Log
import java.time.Clock

private const val SUGGESTION_LOG_TAG = "TimeTracerSuggestions"
private const val SECONDS_PER_MINUTE = 60
private const val SECONDS_PER_HOUR = 60 * SECONDS_PER_MINUTE
internal fun formatClockDuration(value: String): String? {
    val seconds = clockDurationToSeconds(value) ?: return null
    return formatDurationUnits(seconds)
}

internal fun clockDurationToSeconds(value: String): Int? {
    val parts = value.split(":")
    if (parts.size !in 2..3) {
        return null
    }
    val hours = parts[0].toIntOrNull() ?: return null
    val minutes = parts[1].toIntOrNull() ?: return null
    val seconds = if (parts.size == 3) parts[2].toIntOrNull() ?: return null else 0
    if (hours < 0 || minutes !in 0..59 || seconds !in 0..59) {
        return null
    }
    return hours * SECONDS_PER_HOUR + minutes * SECONDS_PER_MINUTE + seconds
}

internal fun formatDurationClock(seconds: Int): String {
    val boundedSeconds = seconds.coerceAtLeast(0)
    val hours = boundedSeconds / SECONDS_PER_HOUR
    val minutes = (boundedSeconds % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE
    val remainingSeconds = boundedSeconds % SECONDS_PER_MINUTE
    return if (remainingSeconds == 0) {
        "%02d:%02d".format(hours, minutes)
    } else {
        "%02d:%02d:%02d".format(hours, minutes, remainingSeconds)
    }
}

internal fun formatDurationUnits(totalSeconds: Int): String {
    val boundedSeconds = totalSeconds.coerceAtLeast(0)
    val hours = boundedSeconds / SECONDS_PER_HOUR
    val minutes = (boundedSeconds % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE
    val seconds = boundedSeconds % SECONDS_PER_MINUTE
    return buildList {
        if (hours > 0) add("${hours}h")
        if (minutes > 0) add("${minutes}m")
        if (seconds > 0 || isEmpty()) add("${seconds}s")
    }.joinToString(" ")
}

internal fun logActivitySuggestionsRequestStart(
    logicalDayTarget: RecordLogicalDayTarget,
    lookbackDays: Int,
    topN: Int,
    anchorDateIso: String?
) {
    logSuggestions(
        message = buildString {
            append("stage=record.activity_suggestions.request")
            append(" action=start")
            append(" target=")
            append(logicalDayTarget.name.lowercase())
            append(" lookbackDays=")
            append(lookbackDays)
            append(" topN=")
            append(topN)
            append(" anchorDateIso=")
            append(anchorDateIso ?: "-")
        }
    )
}

internal fun logActivitySuggestionsRequestResult(
    logicalDayTarget: RecordLogicalDayTarget,
    lookbackDays: Int,
    topN: Int,
    anchorDateIso: String?,
    result: ActivitySuggestionResult
) {
    logSuggestions(
        message = buildString {
            append("stage=record.activity_suggestions.request")
            append(" action=finish")
            append(" target=")
            append(logicalDayTarget.name.lowercase())
            append(" lookbackDays=")
            append(lookbackDays)
            append(" topN=")
            append(topN)
            append(" anchorDateIso=")
            append(anchorDateIso ?: "-")
            append(" ok=")
            append(result.ok)
            append(" op=")
            append(result.operationId.ifBlank { "-" })
            append(" suggestionCount=")
            append(result.suggestions.size)
            append(" suggestions=")
            append(result.suggestions.toDiagnosticSample())
            append(" status=")
            append(result.message.replaceLineBreaks())
        }
    )
}

internal fun logSuggestions(message: String) {
    try {
        Log.i(SUGGESTION_LOG_TAG, message)
    } catch (_: Throwable) {
        // Local JVM tests may use the Android stub jar where Log methods are unavailable.
    }
}

internal fun logSuggestedActivityApply(
    canonicalActivityName: String,
    outputMode: RecordSuggestionOutputMode,
    appliedToken: String?,
    status: String
) {
    logSuggestions(
        message = buildString {
            append("stage=record.activity_suggestions.apply")
            append(" canonical=")
            append(canonicalActivityName.ifBlank { "-" })
            append(" outputMode=")
            append(outputMode.name.lowercase())
            append(" appliedToken=")
            append(appliedToken?.takeIf { it.isNotBlank() } ?: "-")
            append(" status=")
            append(status.replaceLineBreaks())
        }
    )
}

internal fun List<String>.toDiagnosticSample(maxItems: Int = 5): String =
    take(maxItems).joinToString(prefix = "[", postfix = "]", separator = ",") {
        it.replaceLineBreaks()
    }

internal fun List<ActivityAliasMappingEntry>.firstAliasByCanonical(): Map<String, String> {
    val aliasesByCanonical = linkedMapOf<String, String>()
    for (entry in this) {
        val canonical = entry.canonical.trim()
        val alias = entry.alias.trim()
        if (canonical.isEmpty() || alias.isEmpty() || aliasesByCanonical.containsKey(canonical)) {
            continue
        }
        aliasesByCanonical[canonical] = alias
    }
    return aliasesByCanonical
}

internal fun String.replaceLineBreaks(): String =
    replace('\n', ' ').replace('\r', ' ')

internal data class ActivityTokenSummary(
    val canonicalToken: String,
    val aliasToken: String
)

internal data class RecordSuccessSummary(
    val canonicalToken: String,
    val aliasToken: String,
    val inputDurationText: String,
    val statusText: String
)

