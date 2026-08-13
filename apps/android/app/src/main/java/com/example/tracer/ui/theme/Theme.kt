package com.example.tracer.ui.theme

import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.ColorScheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.CompositionLocalProvider
import androidx.compose.ui.graphics.Color
import com.example.tracer.data.DarkThemeStyle
import com.example.tracer.data.ThemeConfig
import com.example.tracer.data.ThemeMode
import com.example.tracer.data.ThemePalette

private fun ThemeColorTokens.toLightColorScheme(): ColorScheme = lightColorScheme(
    primary = primary,
    onPrimary = onPrimary,
    primaryContainer = primaryContainer,
    onPrimaryContainer = onPrimaryContainer,
    secondary = secondary,
    onSecondary = onSecondary,
    secondaryContainer = secondaryContainer,
    onSecondaryContainer = onSecondaryContainer,
    tertiary = tertiary,
    onTertiary = onTertiary,
    tertiaryContainer = tertiaryContainer,
    onTertiaryContainer = onTertiaryContainer,
    background = background,
    onBackground = onBackground,
    surface = surface,
    onSurface = onSurface,
    surfaceVariant = surfaceVariant,
    onSurfaceVariant = onSurfaceVariant,
    outline = outline,
    outlineVariant = outlineVariant,
    surfaceContainerLowest = surfaceContainerLowest,
    surfaceContainerLow = surfaceContainerLow,
    surfaceContainer = surfaceContainer,
    surfaceContainerHigh = surfaceContainerHigh,
    surfaceContainerHighest = surfaceContainerHighest
)

private fun ThemeColorTokens.toDarkColorScheme(): ColorScheme = darkColorScheme(
    primary = primary,
    onPrimary = onPrimary,
    primaryContainer = primaryContainer,
    onPrimaryContainer = onPrimaryContainer,
    secondary = secondary,
    onSecondary = onSecondary,
    secondaryContainer = secondaryContainer,
    onSecondaryContainer = onSecondaryContainer,
    tertiary = tertiary,
    onTertiary = onTertiary,
    tertiaryContainer = tertiaryContainer,
    onTertiaryContainer = onTertiaryContainer,
    background = background,
    onBackground = onBackground,
    surface = surface,
    onSurface = onSurface,
    surfaceVariant = surfaceVariant,
    onSurfaceVariant = onSurfaceVariant,
    outline = outline,
    outlineVariant = outlineVariant,
    surfaceContainerLowest = surfaceContainerLowest,
    surfaceContainerLow = surfaceContainerLow,
    surfaceContainer = surfaceContainer,
    surfaceContainerHigh = surfaceContainerHigh,
    surfaceContainerHighest = surfaceContainerHighest
)

private fun buildLightColorScheme(palette: ThemePalette): ColorScheme =
    palette.definition().light.toLightColorScheme()

private fun buildDarkColorScheme(style: DarkThemeStyle, palette: ThemePalette): ColorScheme {
    val definition = palette.definition()
    val tokens = when {
        style == DarkThemeStyle.Black -> definition.dark.copy(
            background = Color.Black,
            surface = Color.Black,
            surfaceContainerLowest = Color.Black
        )
        style == DarkThemeStyle.Neutral && palette == ThemePalette.Indigo ->
            definition.dark.copy(secondary = Neutral400)
        else -> definition.dark
    }
    return tokens.toDarkColorScheme()
}

@Composable
fun TracerTheme(
    themeConfig: ThemeConfig = ThemeConfig(ThemeMode.System, DarkThemeStyle.Tinted),
    content: @Composable () -> Unit
) {
    val darkTheme = if (themeConfig.palette.supportsLightDarkMode) {
        when (themeConfig.themeMode) {
            ThemeMode.System -> isSystemInDarkTheme()
            ThemeMode.Light -> false
            ThemeMode.Dark -> true
        }
    } else {
        false
    }

    val definition = themeConfig.palette.definition()
    val paletteInsightsColors = when {
        !darkTheme -> definition.insightsLight
        themeConfig.darkThemeStyle == DarkThemeStyle.Neutral &&
            themeConfig.palette == ThemePalette.Indigo ->
            definition.insightsDark.copy(treeProgress = Neutral400)
        else -> definition.insightsDark
    }
    val insightsColors = paletteInsightsColors.copy(
        comparisonIncrease = if (darkTheme) {
            InsightsComparisonColorTokens.darkIncrease
        } else {
            InsightsComparisonColorTokens.lightIncrease
        },
        comparisonDecrease = if (darkTheme) {
            InsightsComparisonColorTokens.darkDecrease
        } else {
            InsightsComparisonColorTokens.lightDecrease
        },
        comparisonNeutral = if (darkTheme) {
            InsightsComparisonColorTokens.darkNeutral
        } else {
            InsightsComparisonColorTokens.lightNeutral
        }
    )

    CompositionLocalProvider(LocalInsightsColorTokens provides insightsColors) {
        MaterialTheme(
            colorScheme = if (darkTheme) {
                buildDarkColorScheme(themeConfig.darkThemeStyle, themeConfig.palette)
            } else {
                buildLightColorScheme(themeConfig.palette)
            },
            typography = Typography,
            content = content
        )
    }
}
