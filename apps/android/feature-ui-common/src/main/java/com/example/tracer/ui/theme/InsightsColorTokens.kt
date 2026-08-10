package com.example.tracer.ui.theme

import androidx.compose.runtime.staticCompositionLocalOf
import androidx.compose.ui.graphics.Color

data class InsightsColorTokens(
    val treeHierarchy: Color,
    val treeProgress: Color,
    val timelineDuration: Color,
    val track: Color,
    val gap: Color
)

val LocalInsightsColorTokens = staticCompositionLocalOf<InsightsColorTokens> {
    InsightsColorTokens(
        treeHierarchy = Color(0xFF4F46E5),
        treeProgress = Color(0xFF2563EB),
        timelineDuration = Color(0xFF0284C7),
        track = Color(0xFFE2E8F0),
        gap = Color(0xFFF8FAFC)
    )
}
