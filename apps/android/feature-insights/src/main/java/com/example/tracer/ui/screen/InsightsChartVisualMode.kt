package com.example.tracer

import com.example.tracer.feature.insights.R

enum class InsightsChartVisualMode {
    LINE,
    BAR,
    HEATMAP_MONTH,
    HEATMAP_MULTI_MONTH
}

internal fun InsightsChartVisualMode.supportsAverageLineToggle(): Boolean =
    this == InsightsChartVisualMode.LINE || this == InsightsChartVisualMode.BAR

internal fun InsightsChartVisualMode.labelRes(): Int =
    when (this) {
        InsightsChartVisualMode.LINE -> R.string.insights_chart_visual_line
        InsightsChartVisualMode.BAR -> R.string.insights_chart_visual_bar
        InsightsChartVisualMode.HEATMAP_MONTH,
        InsightsChartVisualMode.HEATMAP_MULTI_MONTH -> R.string.insights_chart_visual_heatmap
    }
