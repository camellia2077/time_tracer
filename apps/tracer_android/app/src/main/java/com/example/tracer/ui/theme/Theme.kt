package com.example.tracer.ui.theme

import android.app.Activity
import android.os.Build
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.ColorScheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.dynamicDarkColorScheme
import androidx.compose.material3.dynamicLightColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.SideEffect
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.lerp
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalView
import androidx.core.view.WindowCompat
import com.example.tracer.data.ThemeColor
import com.example.tracer.data.ThemeConfig
import com.example.tracer.data.ThemeMode

private const val SurfaceContainerTintRatio = 0.10f

private data class PrimaryPalette(
    val p100: Color,
    val p400: Color,
    val p600: Color,
    val p900: Color
)

private data class NeutralPalette(
    val n50: Color,
    val n100: Color,
    val n200: Color,
    val n300: Color,
    val n400: Color,
    val n500: Color,
    val n600: Color,
    val n700: Color,
    val n800: Color,
    val n900: Color,
    val n950: Color
)

private fun getPrimaryPalette(themeColor: ThemeColor): PrimaryPalette =
    when (themeColor) {
        ThemeColor.Rose -> PrimaryPalette(Rose100, Rose400, Rose600, Rose900)
        ThemeColor.Orange -> PrimaryPalette(Orange100, Orange400, Orange600, Orange900)
        ThemeColor.Peach -> PrimaryPalette(Peach100, Peach400, Peach600, Peach900)
        ThemeColor.Gold -> PrimaryPalette(Gold100, Gold400, Gold600, Gold900)
        ThemeColor.Mint -> PrimaryPalette(Mint100, Mint400, Mint600, Mint900)
        ThemeColor.Emerald -> PrimaryPalette(Emerald100, Emerald400, Emerald600, Emerald900)
        ThemeColor.Teal -> PrimaryPalette(Teal100, Teal400, Teal600, Teal900)
        ThemeColor.Cyan -> PrimaryPalette(Cyan100, Cyan400, Cyan600, Cyan900)
        ThemeColor.Sky -> PrimaryPalette(Sky100, Sky400, Sky600, Sky900)
        ThemeColor.Lavender -> PrimaryPalette(Lavender100, Lavender400, Lavender600, Lavender900)
        ThemeColor.Violet -> PrimaryPalette(Violet100, Violet400, Violet600, Violet900)
        ThemeColor.Pink -> PrimaryPalette(Pink100, Pink400, Pink600, Pink900)
        ThemeColor.Sakura -> PrimaryPalette(Sakura100, Sakura400, Sakura600, Sakura900)
        ThemeColor.Magenta -> PrimaryPalette(Magenta100, Magenta400, Magenta600, Magenta900)
        ThemeColor.Slate -> PrimaryPalette(Indigo100, Indigo400, Indigo600, Indigo900)
    }

private fun tintSurface(base: Color, themePrimary: Color): Color =
    lerp(base, themePrimary, SurfaceContainerTintRatio)

private fun getNeutralPalette(themeColor: ThemeColor): NeutralPalette =
    when (themeColor) {
        ThemeColor.Rose -> NeutralPalette(
            n50 = Stone50, n100 = Stone100, n200 = Stone200, n300 = Stone300,
            n400 = Stone400, n500 = Stone500, n600 = Stone600, n700 = Stone700,
            n800 = Stone800, n900 = Stone900, n950 = Stone950
        )
        // Warm colors use Stone neutral
        ThemeColor.Peach, ThemeColor.Gold, ThemeColor.Orange, ThemeColor.Sakura -> NeutralPalette(
            n50 = Stone50, n100 = Stone100, n200 = Stone200, n300 = Stone300,
            n400 = Stone400, n500 = Stone500, n600 = Stone600, n700 = Stone700,
            n800 = Stone800, n900 = Stone900, n950 = Stone950
        )
        ThemeColor.Emerald, ThemeColor.Mint, ThemeColor.Teal, ThemeColor.Cyan -> NeutralPalette(
            n50 = Neutral50, n100 = Neutral100, n200 = Neutral200, n300 = Neutral300,
            n400 = Neutral400, n500 = Neutral500, n600 = Neutral600, n700 = Neutral700,
            n800 = Neutral800, n900 = Neutral900, n950 = Neutral950
        )
        ThemeColor.Sky -> NeutralPalette(
            n50 = SkyNeutral50, n100 = SkyNeutral100, n200 = SkyNeutral200, n300 = SkyNeutral300,
            n400 = SkyNeutral400, n500 = SkyNeutral500, n600 = SkyNeutral600, n700 = SkyNeutral700,
            n800 = SkyNeutral800, n900 = SkyNeutral900, n950 = SkyNeutral950
        )
        ThemeColor.Violet, ThemeColor.Lavender, ThemeColor.Slate -> NeutralPalette(
            n50 = Slate50, n100 = Slate100, n200 = Slate200, n300 = Slate300,
            n400 = Slate400, n500 = Slate500, n600 = Slate600, n700 = Slate700,
            n800 = Slate800, n900 = Slate900, n950 = Slate950
        )
        // Pink, Magenta default to Slate (Cool/Neutral)
        else -> NeutralPalette(
            n50 = Slate50, n100 = Slate100, n200 = Slate200, n300 = Slate300,
            n400 = Slate400, n500 = Slate500, n600 = Slate600, n700 = Slate700,
            n800 = Slate800, n900 = Slate900, n950 = Slate950
        )
    }

private fun getLightColorScheme(themeColor: ThemeColor): ColorScheme {
    val primary = getPrimaryPalette(themeColor)
    val neutrals = getNeutralPalette(themeColor)

    return lightColorScheme(
        primary = primary.p600,
        onPrimary = LightSurface,
        primaryContainer = primary.p100,
        onPrimaryContainer = primary.p900,
        inversePrimary = primary.p400,
        secondary = neutrals.n600,
        onSecondary = neutrals.n50,
        secondaryContainer = neutrals.n200,
        onSecondaryContainer = neutrals.n900,
        tertiary = neutrals.n500,
        onTertiary = neutrals.n50,
        tertiaryContainer = neutrals.n100,
        onTertiaryContainer = neutrals.n900,
        background = neutrals.n100,
        onBackground = neutrals.n900,
        surface = neutrals.n50,
        onSurface = neutrals.n900,
        surfaceVariant = neutrals.n200,
        onSurfaceVariant = neutrals.n600,
        surfaceTint = primary.p600,
        inverseSurface = neutrals.n800,
        inverseOnSurface = neutrals.n100,
        outline = neutrals.n400,
        outlineVariant = neutrals.n300,
        scrim = Color.Black,
        surfaceBright = LightSurface,
        surfaceContainerLowest = tintSurface(LightSurface, primary.p600),
        surfaceContainerLow = tintSurface(neutrals.n50, primary.p600),
        surfaceContainer = tintSurface(neutrals.n100, primary.p600),
        surfaceContainerHigh = tintSurface(neutrals.n200, primary.p600),
        surfaceContainerHighest = tintSurface(neutrals.n300, primary.p600),
        surfaceDim = neutrals.n200,
        primaryFixed = primary.p100,
        primaryFixedDim = primary.p400,
        onPrimaryFixed = primary.p900,
        onPrimaryFixedVariant = primary.p600,
        secondaryFixed = neutrals.n200,
        secondaryFixedDim = neutrals.n400,
        onSecondaryFixed = neutrals.n900,
        onSecondaryFixedVariant = neutrals.n700,
        tertiaryFixed = neutrals.n200,
        tertiaryFixedDim = neutrals.n400,
        onTertiaryFixed = neutrals.n900,
        onTertiaryFixedVariant = neutrals.n700
    )
}

private fun getDarkColorScheme(
    themeColor: ThemeColor,
    darkThemeStyle: com.example.tracer.data.DarkThemeStyle
): ColorScheme {
    val primary = getPrimaryPalette(themeColor)
    val neutrals = if (darkThemeStyle == com.example.tracer.data.DarkThemeStyle.Neutral) {
        // Google Grey (Neutral Palette)
        NeutralPalette(
            n50 = Neutral50, n100 = Neutral100, n200 = Neutral200, n300 = Neutral300,
            n400 = Neutral400, n500 = Neutral500, n600 = Neutral600, n700 = Neutral700,
            n800 = Neutral800, n900 = Neutral900, n950 = Neutral950
        )
    } else {
        // Tinted (Default)
        getNeutralPalette(themeColor)
    }

    // Pure Black Support
    val surfaceColor = if (darkThemeStyle == com.example.tracer.data.DarkThemeStyle.Black) Color.Black else neutrals.n900
    val backgroundColor = if (darkThemeStyle == com.example.tracer.data.DarkThemeStyle.Black) Color.Black else neutrals.n950

    return darkColorScheme(
        primary = primary.p400,
        onPrimary = neutrals.n900,
        primaryContainer = primary.p900,
        onPrimaryContainer = primary.p100,
        inversePrimary = primary.p600,
        secondary = neutrals.n400,
        onSecondary = neutrals.n950,
        secondaryContainer = neutrals.n700,
        onSecondaryContainer = neutrals.n100,
        tertiary = neutrals.n500,
        onTertiary = neutrals.n950,
        tertiaryContainer = neutrals.n700,
        onTertiaryContainer = neutrals.n100,
        background = backgroundColor,
        onBackground = neutrals.n100,
        surface = surfaceColor,
        onSurface = neutrals.n100,
        surfaceVariant = neutrals.n800,
        onSurfaceVariant = neutrals.n300,
        surfaceTint = primary.p400,
        inverseSurface = neutrals.n100,
        inverseOnSurface = neutrals.n900,
        outline = neutrals.n500,
        outlineVariant = neutrals.n700,
        scrim = Color.Black,
        surfaceBright = neutrals.n800,
        surfaceContainerLowest = tintSurface(neutrals.n950, primary.p400),
        surfaceContainerLow = tintSurface(neutrals.n900, primary.p400),
        surfaceContainer = tintSurface(neutrals.n800, primary.p400),
        surfaceContainerHigh = tintSurface(neutrals.n700, primary.p400),
        surfaceContainerHighest = tintSurface(neutrals.n600, primary.p400),
        surfaceDim = neutrals.n950,
        primaryFixed = primary.p100,
        primaryFixedDim = primary.p400,
        onPrimaryFixed = primary.p900,
        onPrimaryFixedVariant = primary.p600,
        secondaryFixed = neutrals.n400,
        secondaryFixedDim = neutrals.n500,
        onSecondaryFixed = neutrals.n900,
        onSecondaryFixedVariant = neutrals.n700,
        tertiaryFixed = neutrals.n400,
        tertiaryFixedDim = neutrals.n500,
        onTertiaryFixed = neutrals.n900,
        onTertiaryFixedVariant = neutrals.n700
    )
}

@Composable
fun TracerTheme(
    themeConfig: ThemeConfig = ThemeConfig(ThemeColor.Slate, ThemeMode.System, false),
    content: @Composable () -> Unit
) {
    val darkTheme = when (themeConfig.themeMode) {
        ThemeMode.System -> isSystemInDarkTheme()
        ThemeMode.Light -> false
        ThemeMode.Dark -> true
    }

    // Dynamic color is available on Android 12+
    val dynamicColor = themeConfig.useDynamicColor
    
    val colorScheme = when {
        dynamicColor && Build.VERSION.SDK_INT >= Build.VERSION_CODES.S -> {
            val context = LocalContext.current
            if (darkTheme) dynamicDarkColorScheme(context) else dynamicLightColorScheme(context)
        }
        darkTheme -> getDarkColorScheme(themeConfig.themeColor, themeConfig.darkThemeStyle)
        else -> getLightColorScheme(themeConfig.themeColor)
    }

    val view = LocalView.current
    if (!view.isInEditMode) {
        SideEffect {
            val window = (view.context as Activity).window
            window.statusBarColor = colorScheme.background.toArgb() // Immersive feel
            WindowCompat.getInsetsController(window, view).isAppearanceLightStatusBars = !darkTheme
        }
    }

    MaterialTheme(
        colorScheme = colorScheme,
        typography = Typography,
        content = content
    )
}
