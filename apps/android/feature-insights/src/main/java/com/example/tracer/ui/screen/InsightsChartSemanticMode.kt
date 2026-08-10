package com.example.tracer

import com.example.tracer.feature.insights.R

enum class InsightsChartSemanticMode {
    COMPOSITION,
    TREND
}

internal fun InsightsChartSemanticMode.labelRes(): Int =
    when (this) {
        InsightsChartSemanticMode.TREND -> R.string.insights_chart_semantic_trend
        InsightsChartSemanticMode.COMPOSITION -> R.string.insights_chart_semantic_composition
    }

internal fun InsightsChartSemanticMode.normalizeForInsightsMode(
    insightsMode: InsightsMode
): InsightsChartSemanticMode = if (insightsMode == InsightsMode.DAY) {
    InsightsChartSemanticMode.COMPOSITION
} else {
    this
}
