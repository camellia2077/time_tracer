package com.example.tracer

import com.example.tracer.feature.report.R

enum class ReportChartSemanticMode {
    TREND,
    COMPOSITION
}

internal fun ReportChartSemanticMode.labelRes(): Int =
    when (this) {
        ReportChartSemanticMode.TREND -> R.string.report_chart_semantic_trend
        ReportChartSemanticMode.COMPOSITION -> R.string.report_chart_semantic_composition
    }

internal fun defaultChartSemanticMode(reportMode: ReportMode): ReportChartSemanticMode =
    if (reportMode == ReportMode.DAY) {
        ReportChartSemanticMode.COMPOSITION
    } else {
        ReportChartSemanticMode.TREND
    }

internal fun ReportChartSemanticMode.normalizeForReportMode(
    reportMode: ReportMode
): ReportChartSemanticMode = if (reportMode == ReportMode.DAY) {
    ReportChartSemanticMode.COMPOSITION
} else {
    this
}
