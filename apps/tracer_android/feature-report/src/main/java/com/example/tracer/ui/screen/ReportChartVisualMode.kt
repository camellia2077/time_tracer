package com.example.tracer

import com.example.tracer.feature.report.R

internal enum class ReportChartVisualMode {
    LINE,
    BAR,
    PIE,
    HEATMAP_MONTH,
    HEATMAP_YEAR
}

internal fun ReportChartVisualMode.supportsAverageLineToggle(): Boolean =
    this == ReportChartVisualMode.LINE || this == ReportChartVisualMode.BAR

internal fun ReportChartVisualMode.labelRes(): Int =
    when (this) {
        ReportChartVisualMode.LINE -> R.string.report_chart_visual_line
        ReportChartVisualMode.BAR -> R.string.report_chart_visual_bar
        ReportChartVisualMode.PIE -> R.string.report_chart_visual_pie
        ReportChartVisualMode.HEATMAP_MONTH -> R.string.report_chart_visual_heatmap_month
        ReportChartVisualMode.HEATMAP_YEAR -> R.string.report_chart_visual_heatmap_year
    }
