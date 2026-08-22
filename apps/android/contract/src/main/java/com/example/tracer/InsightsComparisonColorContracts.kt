package com.example.tracer

/** Color presets for period-over-period activity changes in Insights Overview. */
enum class InsightsComparisonColorScheme {
    GREEN_RED,
    RED_GREEN,
    THEME_ACCENT_NEUTRAL,
    BLUE_ORANGE
}

/** Symbol presets for period-over-period activity changes in Insights Overview. */
enum class InsightsComparisonIndicatorStyle {
    ARROWS,
    TREND_LINES,
    SIGNS
}

fun defaultInsightsComparisonColorScheme(): InsightsComparisonColorScheme =
    InsightsComparisonColorScheme.GREEN_RED

fun defaultInsightsComparisonIndicatorStyle(): InsightsComparisonIndicatorStyle =
    InsightsComparisonIndicatorStyle.ARROWS
