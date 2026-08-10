package com.example.tracer

import com.example.tracer.feature.insights.R

enum class InsightsCompositionVisualMode {
    PIE,
    HORIZONTAL_BAR,
    TREEMAP
}

internal fun InsightsCompositionVisualMode.labelRes(): Int =
    when (this) {
        InsightsCompositionVisualMode.PIE -> R.string.insights_chart_visual_composition_pie
        InsightsCompositionVisualMode.HORIZONTAL_BAR ->
            R.string.insights_chart_visual_composition_bar
        InsightsCompositionVisualMode.TREEMAP ->
            R.string.insights_chart_visual_composition_treemap
    }
