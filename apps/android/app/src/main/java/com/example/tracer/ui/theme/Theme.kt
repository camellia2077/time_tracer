package com.example.tracer.ui.theme

import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.ColorScheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.CompositionLocalProvider
import androidx.compose.ui.graphics.Color
import com.example.tracer.data.DarkSurfaceStyle
import com.example.tracer.data.LightSurfaceStyle
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

private fun buildLightColorScheme(style: LightSurfaceStyle, palette: ThemePalette): ColorScheme {
    val tokens = if (style == LightSurfaceStyle.Elevated && palette.supportsLightDarkMode) {
        palette.definition().light.copy(
            background = Color(0xFFE2E8F0),
            surface = Color.White,
            surfaceVariant = Color(0xFFF1F5F9),
            surfaceContainerLowest = Color.White,
            surfaceContainerLow = Color(0xFFF8FAFC),
            surfaceContainer = Color.White,
            surfaceContainerHigh = Color(0xFFF1F5F9),
            surfaceContainerHighest = Color(0xFFE2E8F0)
        )
    } else {
        palette.definition().light
    }
    return tokens.toLightColorScheme()
}

private fun buildDarkColorScheme(style: DarkSurfaceStyle, palette: ThemePalette): ColorScheme {
    val definition = palette.definition()
    val tokens = when {
        style == DarkSurfaceStyle.Black -> definition.dark.copy(
            background = Color.Black,
            // Keep the page background pure black, but lift cards enough to
            // make their boundaries readable in the Black surface style.
            surface = Color(0xFF161616),
            surfaceVariant = Color(0xFF202020),
            surfaceContainerLowest = Color.Black,
            // ElevatedCard defaults to surfaceContainerLow, so this level
            // needs a clear lift from the page background to remain visible.
            surfaceContainerLow = Color(0xFF242424),
            surfaceContainer = Color(0xFF2C2C2C),
            surfaceContainerHigh = Color(0xFF363636),
            surfaceContainerHighest = Color(0xFF404040)
        )
        else -> definition.dark
    }
    return tokens.toDarkColorScheme()
}

@Composable
fun TracerTheme(
    themeConfig: ThemeConfig = ThemeConfig(ThemeMode.System, DarkSurfaceStyle.Neutral),
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
                buildDarkColorScheme(themeConfig.darkSurfaceStyle, themeConfig.palette)
            } else {
                buildLightColorScheme(themeConfig.lightSurfaceStyle, themeConfig.palette)
            },
            typography = Typography,
            content = content
        )
    }
}
