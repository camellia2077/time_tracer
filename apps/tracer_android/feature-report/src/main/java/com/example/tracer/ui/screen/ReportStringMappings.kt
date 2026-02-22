package com.example.tracer

import androidx.annotation.StringRes
import com.example.tracer.feature.report.R

@StringRes
internal fun ReportMode.labelRes(): Int =
    when (this) {
        ReportMode.DAY -> R.string.report_mode_day
        ReportMode.MONTH -> R.string.report_mode_month
        ReportMode.WEEK -> R.string.report_mode_week
        ReportMode.YEAR -> R.string.report_mode_year
        ReportMode.RANGE -> R.string.report_mode_range
        ReportMode.RECENT -> R.string.report_mode_recent
    }

@StringRes
internal fun DataTreePeriod.labelRes(): Int =
    when (this) {
        DataTreePeriod.DAY -> R.string.report_mode_day
        DataTreePeriod.MONTH -> R.string.report_mode_month
        DataTreePeriod.WEEK -> R.string.report_mode_week
        DataTreePeriod.YEAR -> R.string.report_mode_year
        DataTreePeriod.RANGE -> R.string.report_mode_range
        DataTreePeriod.RECENT -> R.string.report_mode_recent
    }

@StringRes
internal fun ReportResultDisplayMode.labelRes(): Int =
    when (this) {
        ReportResultDisplayMode.TEXT -> R.string.report_result_mode_text
        ReportResultDisplayMode.CHART -> R.string.report_result_mode_chart
    }
