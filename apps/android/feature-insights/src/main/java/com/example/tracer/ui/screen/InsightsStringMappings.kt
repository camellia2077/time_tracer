package com.example.tracer

import androidx.annotation.StringRes
import com.example.tracer.feature.insights.R

@StringRes
internal fun InsightsMode.labelRes(): Int =
    when (this) {
        InsightsMode.DAY -> R.string.insights_mode_day
        InsightsMode.MONTH -> R.string.insights_mode_month
        InsightsMode.WEEK -> R.string.insights_mode_week
        InsightsMode.YEAR -> R.string.insights_mode_year
        InsightsMode.RANGE -> R.string.insights_mode_range
        InsightsMode.RECENT -> R.string.insights_mode_recent
    }

@StringRes
internal fun DataTreePeriod.labelRes(): Int =
    when (this) {
        DataTreePeriod.DAY -> R.string.insights_mode_day
        DataTreePeriod.MONTH -> R.string.insights_mode_month
        DataTreePeriod.WEEK -> R.string.insights_mode_week
        DataTreePeriod.YEAR -> R.string.insights_mode_year
        DataTreePeriod.RANGE -> R.string.insights_mode_range
        DataTreePeriod.RECENT -> R.string.insights_mode_recent
    }

@StringRes
internal fun InsightsResultDisplayMode.labelRes(): Int =
    when (this) {
        InsightsResultDisplayMode.DETAILS -> R.string.insights_result_mode_details
        InsightsResultDisplayMode.CHART -> R.string.insights_result_mode_chart
        InsightsResultDisplayMode.HIERARCHY -> R.string.insights_result_mode_hierarchy
    }
