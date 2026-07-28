package com.example.tracer

import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color
import com.example.tracer.ui.theme.LocalReportColorTokens

internal data class ReportSemanticColors(
    val root: Color,
    val child: Color,
    val node: Color,
    val treeProgressAccent: Color,
    val track: Color,
    val progress: Color,
    val gap: Color
)

@Composable
internal fun reportSemanticColors(): ReportSemanticColors {
    val tokens = LocalReportColorTokens.current
    return ReportSemanticColors(
        root = tokens.treeHierarchy,
        child = tokens.treeHierarchy.copy(alpha = 0.7f),
        node = tokens.treeHierarchy,
        treeProgressAccent = tokens.treeProgress,
        track = tokens.track,
        progress = tokens.timelineDuration,
        gap = tokens.gap
    )
}
