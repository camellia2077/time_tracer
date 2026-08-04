package com.example.tracer.ui.theme

import androidx.compose.ui.graphics.Color

internal data class ThemeColorTokens(
    val primary: Color,
    val onPrimary: Color,
    val primaryContainer: Color,
    val onPrimaryContainer: Color,
    val secondary: Color,
    val onSecondary: Color,
    val secondaryContainer: Color,
    val onSecondaryContainer: Color,
    val tertiary: Color,
    val onTertiary: Color,
    val tertiaryContainer: Color,
    val onTertiaryContainer: Color,
    val background: Color,
    val onBackground: Color,
    val surface: Color,
    val onSurface: Color,
    val surfaceVariant: Color,
    val onSurfaceVariant: Color,
    val outline: Color,
    val outlineVariant: Color,
    val surfaceContainerLowest: Color,
    val surfaceContainerLow: Color,
    val surfaceContainer: Color,
    val surfaceContainerHigh: Color,
    val surfaceContainerHighest: Color
)

internal data class ThemePreviewColors(
    val primary: Color,
    val accent: Color,
    val surface: Color
)

internal data class ThemePaletteDefinition(
    val light: ThemeColorTokens,
    val dark: ThemeColorTokens,
    val preview: ThemePreviewColors,
    val reportLight: ReportColorTokens,
    val reportDark: ReportColorTokens
)

internal fun ThemeColorTokens.toReportColorTokens(): ReportColorTokens =
    ReportColorTokens(
        treeHierarchy = primary,
        treeProgress = secondary,
        timelineDuration = tertiary,
        track = outlineVariant,
        gap = surfaceVariant
    )

