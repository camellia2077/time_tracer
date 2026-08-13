package com.example.tracer.ui.theme

import androidx.compose.runtime.staticCompositionLocalOf
import androidx.compose.ui.graphics.Color

/** Shared semantic colors for period-over-period changes, independent of the selected palette. */
object InsightsComparisonColorTokens {
    val lightIncrease = Color(0xFF1A7F37)
    val lightDecrease = Color(0xFFCF222E)
    val lightNeutral = Color(0xFF57606A)
    val darkIncrease = Color(0xFF3FB950)
    val darkDecrease = Color(0xFFF85149)
    val darkNeutral = Color(0xFF8C959F)
}

data class InsightsColorTokens(
    val treeHierarchy: Color,
    val treeProgress: Color,
    val timelineDuration: Color,
    val track: Color,
    val gap: Color,
    val comparisonIncrease: Color,
    val comparisonDecrease: Color,
    val comparisonNeutral: Color
)

val LocalInsightsColorTokens = staticCompositionLocalOf<InsightsColorTokens> {
    InsightsColorTokens(
        treeHierarchy = Color(0xFF4F46E5),
        treeProgress = Color(0xFF2563EB),
        timelineDuration = Color(0xFF0284C7),
        track = Color(0xFFE2E8F0),
        gap = Color(0xFFF8FAFC),
        comparisonIncrease = InsightsComparisonColorTokens.lightIncrease,
        comparisonDecrease = InsightsComparisonColorTokens.lightDecrease,
        comparisonNeutral = InsightsComparisonColorTokens.lightNeutral
    )
}
