package com.example.tracer

import com.example.tracer.feature.report.R

enum class ReportCompositionVisualMode {
    PIE,
    HORIZONTAL_BAR,
    TREEMAP
}

internal fun ReportCompositionVisualMode.labelRes(): Int =
    when (this) {
        ReportCompositionVisualMode.PIE -> R.string.report_chart_visual_composition_pie
        ReportCompositionVisualMode.HORIZONTAL_BAR ->
            R.string.report_chart_visual_composition_bar
        ReportCompositionVisualMode.TREEMAP ->
            R.string.report_chart_visual_composition_treemap
    }

internal fun ReportCompositionVisualMode.normalizeForReportMode(
    reportMode: ReportMode
): ReportCompositionVisualMode =
    if (reportMode == ReportMode.DAY || this != ReportCompositionVisualMode.TREEMAP) {
        this
    } else {
        ReportCompositionVisualMode.PIE
    }
