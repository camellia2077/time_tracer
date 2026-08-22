package com.example.tracer

import androidx.compose.runtime.Composable
import androidx.compose.material3.MaterialTheme
import androidx.compose.ui.graphics.Color
import com.example.tracer.ui.theme.InsightsColorTokens
import com.example.tracer.ui.theme.LocalInsightsColorTokens

internal data class InsightsSemanticColors(
    val root: Color,
    val child: Color,
    val node: Color,
    val treeProgressAccent: Color,
    val track: Color,
    val progress: Color,
    val gap: Color,
    val comparisonIncrease: Color,
    val comparisonDecrease: Color,
    val comparisonNeutral: Color
)

@Composable
internal fun insightsSemanticColors(
    comparisonColorScheme: InsightsComparisonColorScheme =
        defaultInsightsComparisonColorScheme()
): InsightsSemanticColors {
    val tokens = LocalInsightsColorTokens.current
    val (comparisonIncrease, comparisonDecrease) = resolveComparisonColors(
        colorScheme = comparisonColorScheme,
        tokens = tokens,
        themeAccent = MaterialTheme.colorScheme.secondary
    )
    return InsightsSemanticColors(
        root = tokens.treeHierarchy,
        child = tokens.treeHierarchy.copy(alpha = 0.7f),
        node = tokens.treeHierarchy,
        treeProgressAccent = tokens.treeProgress,
        track = tokens.track,
        progress = tokens.timelineDuration,
        gap = tokens.gap,
        comparisonIncrease = comparisonIncrease,
        comparisonDecrease = comparisonDecrease,
        comparisonNeutral = tokens.comparisonNeutral
    )
}

internal fun resolveComparisonColors(
    colorScheme: InsightsComparisonColorScheme,
    tokens: InsightsColorTokens,
    themeAccent: Color
): Pair<Color, Color> = when (colorScheme) {
        InsightsComparisonColorScheme.GREEN_RED ->
            tokens.comparisonIncrease to tokens.comparisonDecrease
        InsightsComparisonColorScheme.RED_GREEN ->
            tokens.comparisonDecrease to tokens.comparisonIncrease
        InsightsComparisonColorScheme.THEME_ACCENT_NEUTRAL ->
            themeAccent to Color(0xFF9CA3AF)
        InsightsComparisonColorScheme.BLUE_ORANGE ->
            Color(0xFF2563EB) to Color(0xFFF97316)
    }
