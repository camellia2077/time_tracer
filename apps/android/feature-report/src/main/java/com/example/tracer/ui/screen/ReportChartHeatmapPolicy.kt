package com.example.tracer

import java.time.LocalDate
import java.time.YearMonth
import java.time.temporal.ChronoUnit

/**
 * Central policy for which heatmap representation a report window uses.
 *
 * The selector exposes one logical `Heatmap` option for Range/Recent. The
 * concrete single-month or multi-month renderer is resolved from the query
 * window here, so the UI does not need to duplicate date-span rules.
 */
internal object ReportChartHeatmapPolicy {
    fun availableVisualModes(reportMode: ReportMode): List<ReportChartVisualMode> =
        when (reportMode) {
            ReportMode.WEEK,
            ReportMode.MONTH -> modesWithout(ReportChartVisualMode.HEATMAP_MULTI_MONTH)
            ReportMode.YEAR -> modesWithout(ReportChartVisualMode.HEATMAP_MONTH)
            ReportMode.RANGE,
            ReportMode.RECENT -> modesWithout(ReportChartVisualMode.HEATMAP_MULTI_MONTH)
            else -> ReportChartVisualMode.entries
        }

    fun resolveVisualMode(
        reportMode: ReportMode,
        requestedMode: ReportChartVisualMode,
        points: List<ReportChartPoint>,
        fromDateIso: String? = null,
        toDateIso: String? = null
    ): ReportChartVisualMode {
        if (!requestedMode.isHeatmap()) {
            return requestedMode
        }
        return when (reportMode) {
            ReportMode.WEEK,
            ReportMode.MONTH,
            ReportMode.DAY -> ReportChartVisualMode.HEATMAP_MONTH
            ReportMode.YEAR -> ReportChartVisualMode.HEATMAP_MULTI_MONTH
            ReportMode.RANGE,
            ReportMode.RECENT -> if (monthCount(fromDateIso, toDateIso, points) > 1) {
                ReportChartVisualMode.HEATMAP_MULTI_MONTH
            } else {
                ReportChartVisualMode.HEATMAP_MONTH
            }
        }
    }

    private fun modesWithout(excluded: ReportChartVisualMode): List<ReportChartVisualMode> =
        ReportChartVisualMode.entries.filter { it != excluded }

    private fun monthCount(
        fromDateIso: String?,
        toDateIso: String?,
        points: List<ReportChartPoint>
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

private fun ReportChartVisualMode.isHeatmap(): Boolean =
    this == ReportChartVisualMode.HEATMAP_MONTH ||
        this == ReportChartVisualMode.HEATMAP_MULTI_MONTH

internal fun availableReportChartVisualModes(
    reportMode: ReportMode
): List<ReportChartVisualMode> = ReportChartHeatmapPolicy.availableVisualModes(reportMode)

internal fun resolveReportChartVisualMode(
    reportMode: ReportMode,
    requestedMode: ReportChartVisualMode,
    points: List<ReportChartPoint>,
    fromDateIso: String? = null,
    toDateIso: String? = null
): ReportChartVisualMode = ReportChartHeatmapPolicy.resolveVisualMode(
    reportMode = reportMode,
    requestedMode = requestedMode,
    points = points,
    fromDateIso = fromDateIso,
    toDateIso = toDateIso
)
