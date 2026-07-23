package com.example.tracer

import com.example.tracer.feature.report.R

enum class ReportChartVisualMode {
    LINE,
    BAR,
    HEATMAP_MONTH,
    HEATMAP_MULTI_MONTH
}

internal fun ReportChartVisualMode.supportsAverageLineToggle(): Boolean =
    this == ReportChartVisualMode.LINE || this == ReportChartVisualMode.BAR

internal fun ReportChartVisualMode.labelRes(): Int =
    when (this) {
        ReportChartVisualMode.LINE -> R.string.report_chart_visual_line
        ReportChartVisualMode.BAR -> R.string.report_chart_visual_bar
        ReportChartVisualMode.HEATMAP_MONTH,
        ReportChartVisualMode.HEATMAP_MULTI_MONTH -> R.string.report_chart_visual_heatmap
    }
