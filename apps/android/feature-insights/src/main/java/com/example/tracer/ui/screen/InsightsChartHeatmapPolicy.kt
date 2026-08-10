package com.example.tracer

import java.time.LocalDate
import java.time.YearMonth
import java.time.temporal.ChronoUnit

/**
 * Central policy for which heatmap representation a insights window uses.
 *
 * The selector exposes one logical `Heatmap` option for Range/Recent. The
 * concrete single-month or multi-month renderer is resolved from the query
 * window here, so the UI does not need to duplicate date-span rules.
 */
internal object InsightsChartHeatmapPolicy {
    fun availableVisualModes(insightsMode: InsightsMode): List<InsightsChartVisualMode> =
        when (insightsMode) {
            InsightsMode.WEEK,
            InsightsMode.MONTH -> modesWithout(InsightsChartVisualMode.HEATMAP_MULTI_MONTH)
            InsightsMode.YEAR -> modesWithout(InsightsChartVisualMode.HEATMAP_MONTH)
            InsightsMode.RANGE,
            InsightsMode.RECENT -> modesWithout(InsightsChartVisualMode.HEATMAP_MULTI_MONTH)
            else -> InsightsChartVisualMode.entries
        }

    fun resolveVisualMode(
        insightsMode: InsightsMode,
        requestedMode: InsightsChartVisualMode,
        points: List<InsightsChartPoint>,
        fromDateIso: String? = null,
        toDateIso: String? = null
    ): InsightsChartVisualMode {
        if (!requestedMode.isHeatmap()) {
            return requestedMode
        }
        return when (insightsMode) {
            InsightsMode.WEEK,
            InsightsMode.MONTH,
            InsightsMode.DAY -> InsightsChartVisualMode.HEATMAP_MONTH
            InsightsMode.YEAR -> InsightsChartVisualMode.HEATMAP_MULTI_MONTH
            InsightsMode.RANGE,
            InsightsMode.RECENT -> if (monthCount(fromDateIso, toDateIso, points) > 1) {
                InsightsChartVisualMode.HEATMAP_MULTI_MONTH
            } else {
                InsightsChartVisualMode.HEATMAP_MONTH
            }
        }
    }

    private fun modesWithout(excluded: InsightsChartVisualMode): List<InsightsChartVisualMode> =
        InsightsChartVisualMode.entries.filter { it != excluded }

    private fun monthCount(
        fromDateIso: String?,
        toDateIso: String?,
        points: List<InsightsChartPoint>
    ): Int {
        // Prefer the requested window because chart points may omit empty days
        // or entire months. Point-derived months are only a fallback for modes
        // such as Recent, where the request may not carry explicit dates.
        val start = parseDate(fromDateIso)
        val end = parseDate(toDateIso)
        if (start != null && end != null && !end.isBefore(start)) {
            return ChronoUnit.MONTHS.between(
                YearMonth.from(start),
                YearMonth.from(end)
            ).toInt() + 1
        }

        return points.mapNotNull { parseDate(it.date) }
            .map(YearMonth::from)
            .distinct()
            .size
    }

    private fun parseDate(value: String?): LocalDate? =
        runCatching { value?.let(LocalDate::parse) }.getOrNull()
}

private fun InsightsChartVisualMode.isHeatmap(): Boolean =
    this == InsightsChartVisualMode.HEATMAP_MONTH ||
        this == InsightsChartVisualMode.HEATMAP_MULTI_MONTH

internal fun availableInsightsChartVisualModes(
    insightsMode: InsightsMode
): List<InsightsChartVisualMode> = InsightsChartHeatmapPolicy.availableVisualModes(insightsMode)

internal fun resolveInsightsChartVisualMode(
    insightsMode: InsightsMode,
    requestedMode: InsightsChartVisualMode,
    points: List<InsightsChartPoint>,
    fromDateIso: String? = null,
    toDateIso: String? = null
): InsightsChartVisualMode = InsightsChartHeatmapPolicy.resolveVisualMode(
    insightsMode = insightsMode,
    requestedMode = requestedMode,
    points = points,
    fromDateIso = fromDateIso,
    toDateIso = toDateIso
)
