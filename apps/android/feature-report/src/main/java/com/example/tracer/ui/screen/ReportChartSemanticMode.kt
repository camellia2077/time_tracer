package com.example.tracer

import com.example.tracer.feature.report.R

enum class ReportChartSemanticMode {
    COMPOSITION,
    TREND
}

internal fun ReportChartSemanticMode.labelRes(): Int =
    when (this) {
        ReportChartSemanticMode.TREND -> R.string.report_chart_semantic_trend
        ReportChartSemanticMode.COMPOSITION -> R.string.report_chart_semantic_composition
    }

internal fun ReportChartSemanticMode.normalizeForReportMode(
    reportMode: ReportMode
): ReportChartSemanticMode = if (reportMode == ReportMode.DAY) {
    ReportChartSemanticMode.COMPOSITION
} else {
    this
}
