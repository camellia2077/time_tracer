package com.example.tracer.ui.theme

import androidx.compose.ui.graphics.Color
import com.example.tracer.data.ThemePalette

private val IndigoLightTokens = ThemeColorTokens(
    primary = Indigo600,
    onPrimary = Color.White,
    primaryContainer = Slate200,
    onPrimaryContainer = Slate900,
    secondary = Color(0xFF2563EB),
    onSecondary = Color.White,
    secondaryContainer = Slate200,
    onSecondaryContainer = Slate900,
    tertiary = Sky600,
    onTertiary = Color.White,
    tertiaryContainer = Slate200,
    onTertiaryContainer = Slate900,
    background = Slate100,
    onBackground = Slate900,
    surface = Color.White,
    onSurface = Slate900,
    surfaceVariant = Slate50,
    onSurfaceVariant = Slate600,
    outline = Slate300,
    outlineVariant = Slate200,
    surfaceContainerLowest = Color.White,
    surfaceContainerLow = Color.White,
    surfaceContainer = Slate50,
    surfaceContainerHigh = Slate100,
    surfaceContainerHighest = Slate200
)

private val IndigoDarkTokens = ThemeColorTokens(
    primary = Indigo400,
    onPrimary = Indigo900,
    primaryContainer = Slate700,
    onPrimaryContainer = Slate100,
    secondary = Color(0xFF60A5FA),
    onSecondary = Slate950,
    secondaryContainer = Slate700,
    onSecondaryContainer = Slate100,
    tertiary = Sky400,
    onTertiary = Slate900,
    tertiaryContainer = Slate700,
    onTertiaryContainer = Slate100,
    background = Slate950,
    onBackground = Slate100,
    surface = Slate900,
    onSurface = Slate100,
    surfaceVariant = Slate800,
    onSurfaceVariant = Slate300,
    outline = Slate500,
    outlineVariant = Slate700,
    surfaceContainerLowest = Slate950,
    surfaceContainerLow = Slate900,
    surfaceContainer = Slate800,
    surfaceContainerHigh = Slate700,
    surfaceContainerHighest = Slate600
)

private val GraphiteLightTokens = ThemeColorTokens(
    primary = Color(0xFF3F3F46),
    onPrimary = Color.White,
    primaryContainer = Slate200,
    onPrimaryContainer = Slate900,
    secondary = Color(0xFFD97706),
    onSecondary = Slate950,
    secondaryContainer = Slate200,
    onSecondaryContainer = Slate900,
    tertiary = Color(0xFFD97706),
    onTertiary = Slate950,
    tertiaryContainer = Slate200,
    onTertiaryContainer = Slate900,
    background = Color(0xFFF4F4F5),
    onBackground = Slate900,
    surface = Color.White,
    onSurface = Slate900,
    surfaceVariant = Color(0xFFFAFAFA),
    onSurfaceVariant = Slate600,
    outline = Slate300,
    outlineVariant = Slate200,
    surfaceContainerLowest = Color.White,
    surfaceContainerLow = Color.White,
    surfaceContainer = Color(0xFFFAFAFA),
    surfaceContainerHigh = Color(0xFFF4F4F5),
    surfaceContainerHighest = Color(0xFFE4E4E7)
)

private val GraphiteDarkTokens = ThemeColorTokens(
    primary = Color(0xFFD4D4D8),
    onPrimary = Slate900,
    primaryContainer = Color(0xFF3F3F46),
    onPrimaryContainer = Slate100,
    secondary = Color(0xFFFBBF24),
    onSecondary = Slate900,
    secondaryContainer = Color(0xFF3F3F46),
    onSecondaryContainer = Slate100,
    tertiary = Color(0xFFFBBF24),
    onTertiary = Slate900,
    tertiaryContainer = Color(0xFF3F3F46),
    onTertiaryContainer = Slate100,
    background = Color(0xFF09090B),
    onBackground = Slate100,
    surface = Color(0xFF18181B),
    onSurface = Slate100,
    surfaceVariant = Color(0xFF27272A),
    onSurfaceVariant = Slate300,
    outline = Color(0xFF71717A),
    outlineVariant = Color(0xFF3F3F46),
    surfaceContainerLowest = Color(0xFF09090B),
    surfaceContainerLow = Color(0xFF18181B),
    surfaceContainer = Color(0xFF27272A),
    surfaceContainerHigh = Color(0xFF3F3F46),
    surfaceContainerHighest = Color(0xFF52525B)
)

private val TealLightTokens = ThemeColorTokens(
    primary = Color(0xFF0F766E),
    onPrimary = Color.White,
    primaryContainer = Slate200,
    onPrimaryContainer = Slate900,
    secondary = Color(0xFF0D9488),
    onSecondary = Slate950,
    secondaryContainer = Slate200,
    onSecondaryContainer = Slate900,
    tertiary = Color(0xFF0D9488),
    onTertiary = Slate950,
    tertiaryContainer = Slate200,
    onTertiaryContainer = Slate900,
    background = Color(0xFFF0FDFA),
    onBackground = Slate900,
    surface = Color.White,
    onSurface = Slate900,
    surfaceVariant = Color(0xFFF0FDFA),
    onSurfaceVariant = Slate600,
    outline = Slate300,
    outlineVariant = Slate200,
    surfaceContainerLowest = Color.White,
    surfaceContainerLow = Color.White,
    surfaceContainer = Color(0xFFF0FDFA),
    surfaceContainerHigh = Color(0xFFCCFBF1),
    surfaceContainerHighest = Color(0xFF99F6E4)
)

private val TealDarkTokens = ThemeColorTokens(
    primary = Color(0xFF5EEAD4),
    onPrimary = Slate950,
    primaryContainer = Color(0xFF134E4A),
    onPrimaryContainer = Slate100,
    secondary = Color(0xFF2DD4BF),
    onSecondary = Slate950,
    secondaryContainer = Color(0xFF134E4A),
    onSecondaryContainer = Slate100,
    tertiary = Color(0xFF2DD4BF),
    onTertiary = Slate900,
    tertiaryContainer = Color(0xFF134E4A),
    onTertiaryContainer = Slate100,
    background = Color(0xFF042F2E),
    onBackground = Slate100,
    surface = Color(0xFF0F3D3A),
    onSurface = Slate100,
    surfaceVariant = Color(0xFF134E4A),
    onSurfaceVariant = Slate300,
    outline = Slate500,
    outlineVariant = Color(0xFF134E4A),
    surfaceContainerLowest = Color(0xFF042F2E),
    surfaceContainerLow = Color(0xFF0F3D3A),
    surfaceContainer = Color(0xFF134E4A),
    surfaceContainerHigh = Color(0xFF134E4A),
    surfaceContainerHighest = Color(0xFF115E59)
)

// Orange and Rose use the same restrained Slate surface system as Indigo. Only
// the semantic emphasis colors carry the theme hue.
private val OrangeLightTokens = ThemeColorTokens(
    primary = Color(0xFFC2410C),
    onPrimary = Color.White,
    primaryContainer = Slate200,
    onPrimaryContainer = Slate900,
    secondary = Color(0xFFEA580C),
    onSecondary = Color.White,
    secondaryContainer = Slate200,
    onSecondaryContainer = Slate900,
    tertiary = Color(0xFFF97316),
    onTertiary = Color.White,
    tertiaryContainer = Slate200,
    onTertiaryContainer = Slate900,
    background = Slate100,
    onBackground = Slate900,
    surface = Color.White,
    onSurface = Slate900,
    surfaceVariant = Slate50,
    onSurfaceVariant = Slate600,
    outline = Slate300,
    outlineVariant = Slate200,
    surfaceContainerLowest = Color.White,
    surfaceContainerLow = Color.White,
    surfaceContainer = Slate50,
    surfaceContainerHigh = Slate100,
    surfaceContainerHighest = Slate200
)

private val OrangeDarkTokens = ThemeColorTokens(
    primary = Color(0xFFFDBA74),
    onPrimary = Slate950,
    primaryContainer = Slate700,
    onPrimaryContainer = Slate100,
    secondary = Color(0xFFFB923C),
    onSecondary = Slate950,
    secondaryContainer = Slate700,
    onSecondaryContainer = Slate100,
    tertiary = Color(0xFFFDBA74),
    onTertiary = Slate950,
    tertiaryContainer = Slate700,
    onTertiaryContainer = Slate100,
    background = Slate950,
    onBackground = Slate100,
    surface = Slate900,
    onSurface = Slate100,
    surfaceVariant = Slate800,
    onSurfaceVariant = Slate300,
    outline = Slate500,
    outlineVariant = Slate700,
    surfaceContainerLowest = Slate950,
    surfaceContainerLow = Slate900,
    surfaceContainer = Slate800,
    surfaceContainerHigh = Slate700,
    surfaceContainerHighest = Slate600
)

private val RoseLightTokens = ThemeColorTokens(
    primary = Color(0xFFBE123C),
    onPrimary = Color.White,
    primaryContainer = Slate200,
    onPrimaryContainer = Slate900,
    secondary = Color(0xFFE11D48),
    onSecondary = Color.White,
    secondaryContainer = Slate200,
    onSecondaryContainer = Slate900,
    tertiary = Color(0xFFF43F5E),
    onTertiary = Color.White,
    tertiaryContainer = Slate200,
    onTertiaryContainer = Slate900,
    background = Slate100,
    onBackground = Slate900,
    surface = Color.White,
    onSurface = Slate900,
    surfaceVariant = Slate50,
    onSurfaceVariant = Slate600,
    outline = Slate300,
    outlineVariant = Slate200,
    surfaceContainerLowest = Color.White,
    surfaceContainerLow = Color.White,
    surfaceContainer = Slate50,
    surfaceContainerHigh = Slate100,
    surfaceContainerHighest = Slate200
)

private val RoseDarkTokens = ThemeColorTokens(
    primary = Color(0xFFFDA4AF),
    onPrimary = Slate950,
    primaryContainer = Slate700,
    onPrimaryContainer = Slate100,
    secondary = Color(0xFFFB7185),
    onSecondary = Slate950,
    secondaryContainer = Slate700,
    onSecondaryContainer = Slate100,
    tertiary = Color(0xFFFDA4AF),
    onTertiary = Slate950,
    tertiaryContainer = Slate700,
    onTertiaryContainer = Slate100,
    background = Slate950,
    onBackground = Slate100,
    surface = Slate900,
    onSurface = Slate100,
    surfaceVariant = Slate800,
    onSurfaceVariant = Slate300,
    outline = Slate500,
    outlineVariant = Slate700,
    surfaceContainerLowest = Slate950,
    surfaceContainerLow = Slate900,
    surfaceContainer = Slate800,
    surfaceContainerHigh = Slate700,
    surfaceContainerHighest = Slate600
)

private val AmberLightTokens = ThemeColorTokens(
    primary = Color(0xFFB45309),
    onPrimary = Color.White,
    primaryContainer = Slate200,
    onPrimaryContainer = Slate900,
    secondary = Color(0xFFD97706),
    onSecondary = Color.White,
    secondaryContainer = Slate200,
    onSecondaryContainer = Slate900,
    tertiary = Color(0xFFF59E0B),
    onTertiary = Slate950,
    tertiaryContainer = Slate200,
    onTertiaryContainer = Slate900,
    background = Slate100,
    onBackground = Slate900,
    surface = Color.White,
    onSurface = Slate900,
    surfaceVariant = Slate50,
    onSurfaceVariant = Slate600,
    outline = Slate300,
    outlineVariant = Slate200,
    surfaceContainerLowest = Color.White,
    surfaceContainerLow = Color.White,
    surfaceContainer = Slate50,
    surfaceContainerHigh = Slate100,
    surfaceContainerHighest = Slate200
)

private val AmberDarkTokens = ThemeColorTokens(
    primary = Color(0xFFFCD34D),
    onPrimary = Slate950,
    primaryContainer = Slate700,
    onPrimaryContainer = Slate100,
    secondary = Color(0xFFFBBF24),
    onSecondary = Slate950,
    secondaryContainer = Slate700,
    onSecondaryContainer = Slate100,
    tertiary = Color(0xFFFCD34D),
    onTertiary = Slate950,
    tertiaryContainer = Slate700,
    onTertiaryContainer = Slate100,
    background = Slate950,
    onBackground = Slate100,
    surface = Slate900,
    onSurface = Slate100,
    surfaceVariant = Slate800,
    onSurfaceVariant = Slate300,
    outline = Slate500,
    outlineVariant = Slate700,
    surfaceContainerLowest = Slate950,
    surfaceContainerLow = Slate900,
    surfaceContainer = Slate800,
    surfaceContainerHigh = Slate700,
    surfaceContainerHighest = Slate600
)

private val ParchmentTokens = ThemeColorTokens(
    primary = Color(0xFF9E1B1B),
    onPrimary = Color.White,
    primaryContainer = Color(0xFFF0D8CC),
    onPrimaryContainer = Color(0xFF4D0B0B),
    secondary = Color(0xFFC78C25),
    onSecondary = Color(0xFF382F24),
    secondaryContainer = Color(0xFFF1D8A9),
    onSecondaryContainer = Color(0xFF513800),
    tertiary = Color(0xFFC78C25),
    onTertiary = Color(0xFF382F24),
    tertiaryContainer = Color(0xFFF1D8A9),
    onTertiaryContainer = Color(0xFF513800),
    background = Color(0xFFE8E2D0),
    onBackground = Color(0xFF382F24),
    surface = Color(0xFFF3EEDC),
    onSurface = Color(0xFF382F24),
    surfaceVariant = Color(0xFFEDE5D1),
    onSurfaceVariant = Color(0xFF6D5B45),
    outline = Color(0xFFB9A98C),
    outlineVariant = Color(0xFFD7CBB3),
    surfaceContainerLowest = Color(0xFFF8F3E5),
    surfaceContainerLow = Color(0xFFF3EEDC),
    surfaceContainer = Color(0xFFEDE5D1),
    surfaceContainerHigh = Color(0xFFE8E2D0),
    surfaceContainerHighest = Color(0xFFDCD2BA)
)

private val SnowfieldTokens = ThemeColorTokens(
    primary = Color(0xFF4C566A),
    onPrimary = Color.White,
    primaryContainer = Color(0xFFD8DEE9),
    onPrimaryContainer = Color(0xFF2E3440),
    secondary = Color(0xFF2E3440),
    onSecondary = Color.White,
    secondaryContainer = Color(0xFFD8DEE9),
    onSecondaryContainer = Color(0xFF2E3440),
    tertiary = Color(0xFF4C566A),
    onTertiary = Color.White,
    tertiaryContainer = Color(0xFFD8DEE9),
    onTertiaryContainer = Color(0xFF2E3440),
    background = Color(0xFFE5E9F0),
    onBackground = Color(0xFF2E3440),
    surface = Color(0xFFECEFF4),
    onSurface = Color(0xFF2E3440),
    surfaceVariant = Color(0xFFD8DEE9),
    onSurfaceVariant = Color(0xFF4C566A),
    outline = Color(0xFFB8C0CC),
    outlineVariant = Color(0xFFC8D0DC),
    surfaceContainerLowest = Color(0xFFF4F6F9),
    surfaceContainerLow = Color(0xFFECEFF4),
    surfaceContainer = Color(0xFFE5E9F0),
    surfaceContainerHigh = Color(0xFFD8DEE9),
    surfaceContainerHighest = Color(0xFFC8D0DC)
)

private val BlueprintTokens = ThemeColorTokens(
    primary = Color(0xFF8FD3FF),
    onPrimary = Color(0xFF0B1F33),
    primaryContainer = Color(0xFF285B7A),
    onPrimaryContainer = Color(0xFFEAF6FF),
    secondary = Color(0xFF4EA5D9),
    onSecondary = Color(0xFF071522),
    secondaryContainer = Color(0xFF1F4563),
    onSecondaryContainer = Color(0xFFD7F0FF),
    tertiary = Color(0xFFB7E3FF),
    onTertiary = Color(0xFF0B1F33),
    tertiaryContainer = Color(0xFF285B7A),
    onTertiaryContainer = Color(0xFFEAF6FF),
    background = Color(0xFF0B1F33),
    onBackground = Color(0xFFEAF6FF),
    surface = Color(0xFF12304A),
    onSurface = Color(0xFFEAF6FF),
    surfaceVariant = Color(0xFF164463),
    onSurfaceVariant = Color(0xFFBFDFF2),
    outline = Color(0xFF5B91B1),
    outlineVariant = Color(0xFF2C5C78),
    surfaceContainerLowest = Color(0xFF071522),
    surfaceContainerLow = Color(0xFF12304A),
    surfaceContainer = Color(0xFF164463),
    surfaceContainerHigh = Color(0xFF1F4563),
    surfaceContainerHighest = Color(0xFF285B7A)
)

private val NewsprintTokens = ThemeColorTokens(
    primary = Color(0xFF30343B),
    onPrimary = Color.White,
    primaryContainer = Color(0xFFC9C9C4),
    onPrimaryContainer = Color(0xFF202124),
    secondary = Color(0xFFA33F3F),
    onSecondary = Color.White,
    secondaryContainer = Color(0xFFE0C3C0),
    onSecondaryContainer = Color(0xFF4A1D1D),
    tertiary = Color(0xFF6A7078),
    onTertiary = Color.White,
    tertiaryContainer = Color(0xFFD5D6D4),
    onTertiaryContainer = Color(0xFF30343B),
    background = Color(0xFFD9D9D6),
    onBackground = Color(0xFF202124),
    surface = Color(0xFFF1F1ED),
    onSurface = Color(0xFF202124),
    surfaceVariant = Color(0xFFC9C9C4),
    onSurfaceVariant = Color(0xFF565A60),
    outline = Color(0xFF96999B),
    outlineVariant = Color(0xFFB7B8B5),
    surfaceContainerLowest = Color(0xFFFAFAF7),
    surfaceContainerLow = Color(0xFFF1F1ED),
    surfaceContainer = Color(0xFFD9D9D6),
    surfaceContainerHigh = Color(0xFFC9C9C4),
    surfaceContainerHighest = Color(0xFFB7B8B5)
)

private val InkWashTokens = ThemeColorTokens(
    primary = Color(0xFF263238),
    onPrimary = Color.White,
    primaryContainer = Color(0xFFD9E0E3),
    onPrimaryContainer = Color(0xFF182126),
    secondary = Color(0xFFB23A2B),
    onSecondary = Color.White,
    secondaryContainer = Color(0xFFE7C8C2),
    onSecondaryContainer = Color(0xFF531B14),
    tertiary = Color(0xFF7B5E57),
    onTertiary = Color.White,
    tertiaryContainer = Color(0xFFDCD2CE),
    onTertiaryContainer = Color(0xFF3E302D),
    background = Color(0xFFF2EFE6),
    onBackground = Color(0xFF263238),
    surface = Color(0xFFFAF8F2),
    onSurface = Color(0xFF263238),
    surfaceVariant = Color(0xFFE5E0D4),
    onSurfaceVariant = Color(0xFF5F686C),
    outline = Color(0xFFA7A8A0),
    outlineVariant = Color(0xFFC9C6BC),
    surfaceContainerLowest = Color.White,
    surfaceContainerLow = Color(0xFFFAF8F2),
    surfaceContainer = Color(0xFFF2EFE6),
    surfaceContainerHigh = Color(0xFFE5E0D4),
    surfaceContainerHighest = Color(0xFFD9D5CA)
)

private val KraftTokens = ThemeColorTokens(
    primary = Color(0xFF3E3025),
    onPrimary = Color(0xFFF7E7C6),
    primaryContainer = Color(0xFF8C6A43),
    onPrimaryContainer = Color(0xFFFFF1D2),
    secondary = Color(0xFF9A5B2F),
    onSecondary = Color.White,
    secondaryContainer = Color(0xFFD9B47A),
    onSecondaryContainer = Color(0xFF432815),
    tertiary = Color(0xFF526B45),
    onTertiary = Color.White,
    tertiaryContainer = Color(0xFFB8C3A5),
    onTertiaryContainer = Color(0xFF27351F),
    background = Color(0xFFC9A875),
    onBackground = Color(0xFF3E3025),
    surface = Color(0xFFE4C99A),
    onSurface = Color(0xFF3E3025),
    surfaceVariant = Color(0xFFD7B987),
    onSurfaceVariant = Color(0xFF6A4D32),
    outline = Color(0xFF8C6A43),
    outlineVariant = Color(0xFFB18A59),
    surfaceContainerLowest = Color(0xFFF1DDB5),
    surfaceContainerLow = Color(0xFFE4C99A),
    surfaceContainer = Color(0xFFD7B987),
    surfaceContainerHigh = Color(0xFFC9A875),
    surfaceContainerHighest = Color(0xFFB18A59)
)

internal fun ThemePalette.definition(): ThemePaletteDefinition = when (this) {
    ThemePalette.Indigo -> ThemePaletteDefinition(
        light = IndigoLightTokens,
        dark = IndigoDarkTokens,
        preview = ThemePreviewColors(
            primary = IndigoLightTokens.primary,
            accent = IndigoLightTokens.secondary,
            surface = IndigoLightTokens.background
        ),
        reportLight = IndigoLightTokens.toReportColorTokens(),
        reportDark = IndigoDarkTokens.toReportColorTokens()
    )
    ThemePalette.GraphiteAmber -> ThemePaletteDefinition(
        light = GraphiteLightTokens,
        dark = GraphiteDarkTokens,
        preview = ThemePreviewColors(
            primary = GraphiteLightTokens.primary,
            accent = GraphiteLightTokens.secondary,
            surface = GraphiteLightTokens.background
        ),
        reportLight = GraphiteLightTokens.toReportColorTokens(),
        reportDark = GraphiteDarkTokens.toReportColorTokens()
    )
    ThemePalette.Teal -> ThemePaletteDefinition(
        light = TealLightTokens,
        dark = TealDarkTokens,
        preview = ThemePreviewColors(
            primary = TealLightTokens.primary,
            accent = TealLightTokens.secondary,
            surface = TealLightTokens.background
        ),
        reportLight = TealLightTokens.toReportColorTokens(),
        reportDark = TealDarkTokens.toReportColorTokens()
    )
    ThemePalette.Orange -> ThemePaletteDefinition(
        light = OrangeLightTokens,
        dark = OrangeDarkTokens,
        preview = ThemePreviewColors(
            primary = OrangeLightTokens.primary,
            accent = OrangeLightTokens.secondary,
            surface = OrangeLightTokens.background
        ),
        reportLight = OrangeLightTokens.toReportColorTokens(),
        reportDark = OrangeDarkTokens.toReportColorTokens()
    )
    ThemePalette.Rose -> ThemePaletteDefinition(
        light = RoseLightTokens,
        dark = RoseDarkTokens,
        preview = ThemePreviewColors(
            primary = RoseLightTokens.primary,
            accent = RoseLightTokens.secondary,
            surface = RoseLightTokens.background
        ),
        reportLight = RoseLightTokens.toReportColorTokens(),
        reportDark = RoseDarkTokens.toReportColorTokens()
    )
    ThemePalette.Amber -> ThemePaletteDefinition(
        light = AmberLightTokens,
        dark = AmberDarkTokens,
        preview = ThemePreviewColors(
            primary = AmberLightTokens.primary,
            accent = AmberLightTokens.secondary,
            surface = AmberLightTokens.background
        ),
        reportLight = AmberLightTokens.toReportColorTokens(),
        reportDark = AmberDarkTokens.toReportColorTokens()
    )
    ThemePalette.Parchment -> ThemePaletteDefinition(
        light = ParchmentTokens,
        // Parchment is intentionally a fixed visual theme. Keeping both fields equal lets
        // components consume the normal palette contract without manufacturing a dark variant.
        dark = ParchmentTokens,
        preview = ThemePreviewColors(
            primary = ParchmentTokens.primary,
            accent = ParchmentTokens.onSurfaceVariant,
            surface = ParchmentTokens.background
        ),
        reportLight = ParchmentTokens.toReportColorTokens(),
        reportDark = ParchmentTokens.toReportColorTokens()
    )
    ThemePalette.Snowfield -> ThemePaletteDefinition(
        light = SnowfieldTokens,
        // Snowfield is intentionally fixed; both branches use the same tokens.
        dark = SnowfieldTokens,
        preview = ThemePreviewColors(
            primary = SnowfieldTokens.primary,
            accent = Color(0xFF81A1C1),
            surface = SnowfieldTokens.background
        ),
        reportLight = SnowfieldTokens.toReportColorTokens(),
        reportDark = SnowfieldTokens.toReportColorTokens()
    )
    ThemePalette.Blueprint -> ThemePaletteDefinition(
        light = BlueprintTokens,
        dark = BlueprintTokens,
        preview = ThemePreviewColors(
            primary = BlueprintTokens.primary,
            accent = BlueprintTokens.tertiary,
            surface = BlueprintTokens.background
        ),
        reportLight = BlueprintTokens.toReportColorTokens(),
        reportDark = BlueprintTokens.toReportColorTokens()
    )
    ThemePalette.Newsprint -> ThemePaletteDefinition(
        light = NewsprintTokens,
        dark = NewsprintTokens,
        preview = ThemePreviewColors(
            primary = NewsprintTokens.primary,
            accent = NewsprintTokens.tertiary,
            surface = NewsprintTokens.background
        ),
        reportLight = NewsprintTokens.toReportColorTokens(),
        reportDark = NewsprintTokens.toReportColorTokens()
    )
    ThemePalette.InkWash -> ThemePaletteDefinition(
        light = InkWashTokens,
        dark = InkWashTokens,
        preview = ThemePreviewColors(
            primary = InkWashTokens.primary,
            accent = InkWashTokens.tertiary,
            surface = InkWashTokens.background
        ),
        reportLight = InkWashTokens.toReportColorTokens(),
        reportDark = InkWashTokens.toReportColorTokens()
    )
    ThemePalette.Kraft -> ThemePaletteDefinition(
        light = KraftTokens,
        dark = KraftTokens,
        preview = ThemePreviewColors(
            primary = KraftTokens.primary,
            accent = KraftTokens.tertiary,
            surface = KraftTokens.background
        ),
        reportLight = KraftTokens.toReportColorTokens(),
        reportDark = KraftTokens.toReportColorTokens()
    )
}
