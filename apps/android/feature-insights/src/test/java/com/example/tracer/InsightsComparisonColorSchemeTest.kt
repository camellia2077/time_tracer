package com.example.tracer

import androidx.compose.ui.graphics.Color
import com.example.tracer.ui.theme.InsightsColorTokens
import org.junit.Assert.assertEquals
import org.junit.Test

class InsightsComparisonColorSchemeTest {
    private val tokens = InsightsColorTokens(
        treeHierarchy = Color.Unspecified,
        treeProgress = Color.Unspecified,
        timelineDuration = Color.Unspecified,
        track = Color.Unspecified,
        gap = Color.Unspecified,
        comparisonIncrease = Color(0xFF1A7F37),
        comparisonDecrease = Color(0xFFCF222E),
        comparisonNeutral = Color.Unspecified
    )

    @Test
    fun comparisonColorSchemes_resolveRequestedIncreaseAndDecreasePairs() {
        assertEquals(
            Color(0xFFCF222E) to Color(0xFF1A7F37),
            resolveComparisonColors(
                InsightsComparisonColorScheme.RED_GREEN,
                tokens,
                Color(0xFF123456)
            )
        )
        assertEquals(
            Color(0xFF123456) to Color(0xFF9CA3AF),
            resolveComparisonColors(
                InsightsComparisonColorScheme.THEME_ACCENT_NEUTRAL,
                tokens,
                Color(0xFF123456)
            )
        )
        assertEquals(
            Color(0xFF2563EB) to Color(0xFFF97316),
            resolveComparisonColors(
                InsightsComparisonColorScheme.BLUE_ORANGE,
                tokens,
                Color(0xFF123456)
            )
        )
    }
}
