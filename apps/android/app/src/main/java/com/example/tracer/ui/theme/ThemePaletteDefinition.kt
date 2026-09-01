package com.example.tracer.ui.theme

import androidx.compose.ui.graphics.Color
import com.example.tracer.data.ThemePalette

// Switchable palettes share neutral canvases while retaining their theme accents.
private val COMMON_LIGHT_BACKGROUND = Color(0xFFF4F4F5)
private val COMMON_LIGHT_TAB_CONTAINER = Slate50
private val COMMON_DARK_BACKGROUND = Color(0xFF1C1B1F)

// Dark containers are intentionally more separated from the page background.
private val COMMON_DARK_SURFACE = Color(0xFF1C1B1F)
private val COMMON_DARK_SURFACE_VARIANT = Color(0xFF49454F)
private val COMMON_DARK_SURFACE_CONTAINER_LOWEST = Color(0xFF0F0D13)
private val COMMON_DARK_SURFACE_CONTAINER_LOW = Color(0xFF2B2930)
private val COMMON_DARK_SURFACE_CONTAINER = Color(0xFF36343B)
private val COMMON_DARK_SURFACE_CONTAINER_HIGH = Color(0xFF3F3D44)
private val COMMON_DARK_SURFACE_CONTAINER_HIGHEST = Color(0xFF49454F)

private fun ThemeColorTokens.withSwitchableLightSurface(): ThemeColorTokens = copy(
    background = COMMON_LIGHT_BACKGROUND,
    surface = Color.White,
    surfaceContainer = COMMON_LIGHT_TAB_CONTAINER
)

private fun ThemeColorTokens.withSwitchableDarkSurface(): ThemeColorTokens = copy(
    background = COMMON_DARK_BACKGROUND,
    surface = COMMON_DARK_SURFACE,
    surfaceVariant = COMMON_DARK_SURFACE_VARIANT,
    surfaceContainerLowest = COMMON_DARK_SURFACE_CONTAINER_LOWEST,
    surfaceContainerLow = COMMON_DARK_SURFACE_CONTAINER_LOW,
    surfaceContainer = COMMON_DARK_SURFACE_CONTAINER,
    surfaceContainerHigh = COMMON_DARK_SURFACE_CONTAINER_HIGH,
    surfaceContainerHighest = COMMON_DARK_SURFACE_CONTAINER_HIGHEST
)

private val IndigoLightTokens = ThemeColorTokens(
    primary = Indigo600,
    onPrimary = Color.White,
    primaryContainer = Color(0xFFE0E7FF),
    onPrimaryContainer = Color(0xFF1E1B4B),
    secondary = Color(0xFF2563EB),
    onSecondary = Color.White,
    secondaryContainer = Slate200,
    onSecondaryContainer = Slate900,
    tertiary = Sky600,
    onTertiary = Color.White,
    tertiaryContainer = Color(0xFFBBF7D0),
    onTertiaryContainer = Slate900,
    background = COMMON_LIGHT_BACKGROUND,
    onBackground = Slate900,
    surface = Color.White,
    onSurface = Slate900,
    surfaceVariant = Slate50,
    onSurfaceVariant = Slate600,
    outline = Slate300,
    outlineVariant = Slate200,
    surfaceContainerLowest = Color.White,
    surfaceContainerLow = Color.White,
    surfaceContainer = COMMON_LIGHT_TAB_CONTAINER,
    surfaceContainerHigh = Slate100,
    surfaceContainerHighest = Slate200
)

private val IndigoDarkTokens = ThemeColorTokens(
    primary = Indigo400,
    onPrimary = Indigo900,
    primaryContainer = Color(0xFF3730A3),
    onPrimaryContainer = Color(0xFFE0E7FF),
    secondary = Color(0xFF60A5FA),
    onSecondary = Slate950,
    secondaryContainer = Slate700,
    onSecondaryContainer = Slate100,
    tertiary = Sky400,
    onTertiary = Slate900,
    tertiaryContainer = Slate700,
    onTertiaryContainer = Slate100,
    background = COMMON_DARK_BACKGROUND,
    onBackground = Slate100,
    surface = COMMON_DARK_SURFACE,
    onSurface = Slate100,
    surfaceVariant = COMMON_DARK_SURFACE_VARIANT,
    onSurfaceVariant = Slate300,
    outline = Slate500,
    outlineVariant = Slate700,
    surfaceContainerLowest = COMMON_DARK_SURFACE_CONTAINER_LOWEST,
    surfaceContainerLow = COMMON_DARK_SURFACE_CONTAINER_LOW,
    surfaceContainer = COMMON_DARK_SURFACE_CONTAINER,
    surfaceContainerHigh = COMMON_DARK_SURFACE_CONTAINER_HIGH,
    surfaceContainerHighest = COMMON_DARK_SURFACE_CONTAINER_HIGHEST
)

private val PurpleLightTokens = ThemeColorTokens(
    primary = Color(0xFF7E22CE),
    onPrimary = Color.White,
    primaryContainer = Color(0xFFE9D5FF),
    onPrimaryContainer = Color(0xFF3B0764),
    secondary = Color(0xFFA855F7),
    onSecondary = Slate950,
    secondaryContainer = Color(0xFFF3E8FF),
    onSecondaryContainer = Color(0xFF3B0764),
    tertiary = Color(0xFFC084FC),
    onTertiary = Slate950,
    tertiaryContainer = Color(0xFFF3E8FF),
    onTertiaryContainer = Color(0xFF3B0764),
    background = COMMON_LIGHT_BACKGROUND,
    onBackground = Slate900,
    surface = Color.White,
    onSurface = Slate900,
    surfaceVariant = Color(0xFFF5F3FF),
    onSurfaceVariant = Slate600,
    outline = Slate300,
    outlineVariant = Slate200,
    surfaceContainerLowest = Color.White,
    surfaceContainerLow = Color.White,
    surfaceContainer = COMMON_LIGHT_TAB_CONTAINER,
    surfaceContainerHigh = Slate100,
    surfaceContainerHighest = Slate200
)

private val PurpleDarkTokens = ThemeColorTokens(
    primary = Color(0xFFD8B4FE),
    onPrimary = Slate950,
    primaryContainer = Color(0xFF581C87),
    onPrimaryContainer = Color(0xFFF3E8FF),
    secondary = Color(0xFFC084FC),
    onSecondary = Slate950,
    secondaryContainer = Color(0xFF6B21A8),
    onSecondaryContainer = Color(0xFFF3E8FF),
    tertiary = Color(0xFFE9D5FF),
    onTertiary = Slate950,
    tertiaryContainer = Color(0xFF7E22CE),
    onTertiaryContainer = Color(0xFFF3E8FF),
    background = COMMON_DARK_BACKGROUND,
    onBackground = Slate100,
    surface = COMMON_DARK_SURFACE,
    onSurface = Slate100,
    surfaceVariant = COMMON_DARK_SURFACE_VARIANT,
    onSurfaceVariant = Slate300,
    outline = Slate500,
    outlineVariant = Slate700,
    surfaceContainerLowest = COMMON_DARK_SURFACE_CONTAINER_LOWEST,
    surfaceContainerLow = COMMON_DARK_SURFACE_CONTAINER_LOW,
    surfaceContainer = COMMON_DARK_SURFACE_CONTAINER,
    surfaceContainerHigh = COMMON_DARK_SURFACE_CONTAINER_HIGH,
    surfaceContainerHighest = COMMON_DARK_SURFACE_CONTAINER_HIGHEST
)

private val GreyLightTokens = ThemeColorTokens(
    primary = Color(0xFF3F3F46),
    onPrimary = Color.White,
    primaryContainer = Slate200,
    onPrimaryContainer = Slate900,
    secondary = Color(0xFF71717A),
    onSecondary = Slate950,
    secondaryContainer = Slate200,
    onSecondaryContainer = Slate900,
    tertiary = Color(0xFFD97706),
    onTertiary = Slate950,
    tertiaryContainer = Slate200,
    onTertiaryContainer = Slate900,
    background = COMMON_LIGHT_BACKGROUND,
    onBackground = Slate900,
    surface = Color.White,
    onSurface = Slate900,
    surfaceVariant = Color(0xFFFAFAFA),
    onSurfaceVariant = Slate600,
    outline = Slate300,
    outlineVariant = Slate200,
    surfaceContainerLowest = Color.White,
    surfaceContainerLow = Color.White,
    surfaceContainer = COMMON_LIGHT_TAB_CONTAINER,
    surfaceContainerHigh = Color(0xFFF4F4F5),
    surfaceContainerHighest = Color(0xFFE4E4E7)
)

private val GreyDarkTokens = ThemeColorTokens(
    primary = Color(0xFFD4D4D8),
    onPrimary = Slate900,
    primaryContainer = Color(0xFF3F3F46),
    onPrimaryContainer = Slate100,
    secondary = Color(0xFFA1A1AA),
    onSecondary = Slate900,
    secondaryContainer = Color(0xFF3F3F46),
    onSecondaryContainer = Slate100,
    tertiary = Color(0xFFFBBF24),
    onTertiary = Slate900,
    tertiaryContainer = Color(0xFF3F3F46),
    onTertiaryContainer = Slate100,
    background = COMMON_DARK_BACKGROUND,
    onBackground = Slate100,
    surface = COMMON_DARK_SURFACE,
    onSurface = Slate100,
    surfaceVariant = COMMON_DARK_SURFACE_VARIANT,
    onSurfaceVariant = Slate300,
    outline = Color(0xFF71717A),
    outlineVariant = Color(0xFF3F3F46),
    surfaceContainerLowest = COMMON_DARK_SURFACE_CONTAINER_LOWEST,
    surfaceContainerLow = COMMON_DARK_SURFACE_CONTAINER_LOW,
    surfaceContainer = COMMON_DARK_SURFACE_CONTAINER,
    surfaceContainerHigh = COMMON_DARK_SURFACE_CONTAINER_HIGH,
    surfaceContainerHighest = COMMON_DARK_SURFACE_CONTAINER_HIGHEST
)

private val GreenLightTokens = ThemeColorTokens(
    primary = Color(0xFF15803D),
    onPrimary = Color.White,
    primaryContainer = Color(0xFFDCFCE7),
    onPrimaryContainer = Slate900,
    secondary = Color(0xFF16A34A),
    onSecondary = Color.White,
    secondaryContainer = Color(0xFFDCFCE7),
    onSecondaryContainer = Slate900,
    tertiary = Color(0xFF4ADE80),
    onTertiary = Slate950,
    tertiaryContainer = Color(0xFFBBF7D0),
    onTertiaryContainer = Slate900,
    background = COMMON_LIGHT_BACKGROUND,
    onBackground = Slate900,
    surface = Color.White,
    onSurface = Slate900,
    surfaceVariant = Color(0xFFF0FDF4),
    onSurfaceVariant = Slate600,
    outline = Slate300,
    outlineVariant = Slate200,
    surfaceContainerLowest = Color.White,
    surfaceContainerLow = Color.White,
    surfaceContainer = COMMON_LIGHT_TAB_CONTAINER,
    surfaceContainerHigh = Color(0xFFDCFCE7),
    surfaceContainerHighest = Slate200
)

private val GreenDarkTokens = ThemeColorTokens(
    primary = Color(0xFF86EFAC),
    onPrimary = Slate950,
    primaryContainer = Color(0xFF14532D),
    onPrimaryContainer = Slate100,
    secondary = Color(0xFF4ADE80),
    onSecondary = Slate950,
    secondaryContainer = Color(0xFF166534),
    onSecondaryContainer = Slate100,
    tertiary = Color(0xFFBBF7D0),
    onTertiary = Slate900,
    tertiaryContainer = Color(0xFF166534),
    onTertiaryContainer = Slate100,
    background = COMMON_DARK_BACKGROUND,
    onBackground = Slate100,
    surface = COMMON_DARK_SURFACE,
    onSurface = Slate100,
    surfaceVariant = COMMON_DARK_SURFACE_VARIANT,
    onSurfaceVariant = Slate300,
    outline = Slate500,
    outlineVariant = Color(0xFF365C43),
    surfaceContainerLowest = COMMON_DARK_SURFACE_CONTAINER_LOWEST,
    surfaceContainerLow = COMMON_DARK_SURFACE_CONTAINER_LOW,
    surfaceContainer = COMMON_DARK_SURFACE_CONTAINER,
    surfaceContainerHigh = COMMON_DARK_SURFACE_CONTAINER_HIGH,
    surfaceContainerHighest = COMMON_DARK_SURFACE_CONTAINER_HIGHEST
)

private val BlueLightTokens = ThemeColorTokens(
    primary = Color(0xFF1D4ED8),
    onPrimary = Color.White,
    primaryContainer = Color(0xFFDBEAFE),
    onPrimaryContainer = Color(0xFF172554),
    secondary = Color(0xFF3B82F6),
    onSecondary = Color.White,
    secondaryContainer = Color(0xFFDBEAFE),
    onSecondaryContainer = Color(0xFF172554),
    tertiary = Color(0xFF60A5FA),
    onTertiary = Slate950,
    tertiaryContainer = Color(0xFFE0F2FE),
    onTertiaryContainer = Color(0xFF172554),
    background = COMMON_LIGHT_BACKGROUND,
    onBackground = Slate900,
    surface = Color.White,
    onSurface = Slate900,
    surfaceVariant = Color(0xFFF1F5F9),
    onSurfaceVariant = Slate600,
    outline = Slate300,
    outlineVariant = Slate200,
    surfaceContainerLowest = Color.White,
    surfaceContainerLow = Color.White,
    surfaceContainer = COMMON_LIGHT_TAB_CONTAINER,
    surfaceContainerHigh = Slate100,
    surfaceContainerHighest = Slate200
)

private val BlueDarkTokens = ThemeColorTokens(
    primary = Color(0xFF93C5FD),
    onPrimary = Color(0xFF172554),
    primaryContainer = Color(0xFF1E40AF),
    onPrimaryContainer = Color(0xFFDBEAFE),
    secondary = Color(0xFF60A5FA),
    onSecondary = Color(0xFF172554),
    secondaryContainer = Color(0xFF1D4ED8),
    onSecondaryContainer = Color(0xFFDBEAFE),
    tertiary = Color(0xFFBFDBFE),
    onTertiary = Color(0xFF172554),
    tertiaryContainer = Color(0xFF1E3A8A),
    onTertiaryContainer = Color(0xFFDBEAFE),
    background = COMMON_DARK_BACKGROUND,
    onBackground = Slate100,
    surface = COMMON_DARK_SURFACE,
    onSurface = Slate100,
    surfaceVariant = COMMON_DARK_SURFACE_VARIANT,
    onSurfaceVariant = Slate300,
    outline = Slate500,
    outlineVariant = Slate700,
    surfaceContainerLowest = COMMON_DARK_SURFACE_CONTAINER_LOWEST,
    surfaceContainerLow = COMMON_DARK_SURFACE_CONTAINER_LOW,
    surfaceContainer = COMMON_DARK_SURFACE_CONTAINER,
    surfaceContainerHigh = COMMON_DARK_SURFACE_CONTAINER_HIGH,
    surfaceContainerHighest = COMMON_DARK_SURFACE_CONTAINER_HIGHEST
)

// Orange and Rose use the same restrained Slate surface system as Indigo. Only
// the semantic emphasis colors carry the theme hue.
private val OrangeLightTokens = ThemeColorTokens(
    primary = Color(0xFFC2410C),
    onPrimary = Color.White,
    primaryContainer = Color(0xFFFFEDD5),
    onPrimaryContainer = Color(0xFF7C2D12),
    secondary = Color(0xFFEA580C),
    onSecondary = Color.White,
    secondaryContainer = Slate200,
    onSecondaryContainer = Slate900,
    tertiary = Color(0xFFF97316),
    onTertiary = Color.White,
    tertiaryContainer = Slate200,
    onTertiaryContainer = Slate900,
    background = COMMON_LIGHT_BACKGROUND,
    onBackground = Slate900,
    surface = Color.White,
    onSurface = Slate900,
    surfaceVariant = Slate50,
    onSurfaceVariant = Slate600,
    outline = Slate300,
    outlineVariant = Slate200,
    surfaceContainerLowest = Color.White,
    surfaceContainerLow = Color.White,
    surfaceContainer = COMMON_LIGHT_TAB_CONTAINER,
    surfaceContainerHigh = Slate100,
    surfaceContainerHighest = Slate200
)

private val OrangeDarkTokens = ThemeColorTokens(
    primary = Color(0xFFFDBA74),
    onPrimary = Slate950,
    primaryContainer = Color(0xFF9A3412),
    onPrimaryContainer = Color(0xFFFFEDD5),
    secondary = Color(0xFFFB923C),
    onSecondary = Slate950,
    secondaryContainer = Slate700,
    onSecondaryContainer = Slate100,
    tertiary = Color(0xFFFDBA74),
    onTertiary = Slate950,
    tertiaryContainer = Slate700,
    onTertiaryContainer = Slate100,
    background = COMMON_DARK_BACKGROUND,
    onBackground = Slate100,
    surface = COMMON_DARK_SURFACE,
    onSurface = Slate100,
    surfaceVariant = COMMON_DARK_SURFACE_VARIANT,
    onSurfaceVariant = Slate300,
    outline = Slate500,
    outlineVariant = Slate700,
    surfaceContainerLowest = COMMON_DARK_SURFACE_CONTAINER_LOWEST,
    surfaceContainerLow = COMMON_DARK_SURFACE_CONTAINER_LOW,
    surfaceContainer = COMMON_DARK_SURFACE_CONTAINER,
    surfaceContainerHigh = COMMON_DARK_SURFACE_CONTAINER_HIGH,
    surfaceContainerHighest = COMMON_DARK_SURFACE_CONTAINER_HIGHEST
)

private val RoseLightTokens = ThemeColorTokens(
    primary = Color(0xFFBE123C),
    onPrimary = Color.White,
    primaryContainer = Color(0xFFFFE4E6),
    onPrimaryContainer = Color(0xFF881337),
    secondary = Color(0xFFE11D48),
    onSecondary = Color.White,
    secondaryContainer = Slate200,
    onSecondaryContainer = Slate900,
    tertiary = Color(0xFFF43F5E),
    onTertiary = Color.White,
    tertiaryContainer = Slate200,
    onTertiaryContainer = Slate900,
    background = COMMON_LIGHT_BACKGROUND,
    onBackground = Slate900,
    surface = Color.White,
    onSurface = Slate900,
    surfaceVariant = Slate50,
    onSurfaceVariant = Slate600,
    outline = Slate300,
    outlineVariant = Slate200,
    surfaceContainerLowest = Color.White,
    surfaceContainerLow = Color.White,
    surfaceContainer = COMMON_LIGHT_TAB_CONTAINER,
    surfaceContainerHigh = Slate100,
    surfaceContainerHighest = Slate200
)

private val RoseDarkTokens = ThemeColorTokens(
    primary = Color(0xFFFDA4AF),
    onPrimary = Slate950,
    primaryContainer = Color(0xFF9F1239),
    onPrimaryContainer = Color(0xFFFFE4E6),
    secondary = Color(0xFFFB7185),
    onSecondary = Slate950,
    secondaryContainer = Slate700,
    onSecondaryContainer = Slate100,
    tertiary = Color(0xFFFDA4AF),
    onTertiary = Slate950,
    tertiaryContainer = Slate700,
    onTertiaryContainer = Slate100,
    background = COMMON_DARK_BACKGROUND,
    onBackground = Slate100,
    surface = COMMON_DARK_SURFACE,
    onSurface = Slate100,
    surfaceVariant = COMMON_DARK_SURFACE_VARIANT,
    onSurfaceVariant = Slate300,
    outline = Slate500,
    outlineVariant = Slate700,
    surfaceContainerLowest = COMMON_DARK_SURFACE_CONTAINER_LOWEST,
    surfaceContainerLow = COMMON_DARK_SURFACE_CONTAINER_LOW,
    surfaceContainer = COMMON_DARK_SURFACE_CONTAINER,
    surfaceContainerHigh = COMMON_DARK_SURFACE_CONTAINER_HIGH,
    surfaceContainerHighest = COMMON_DARK_SURFACE_CONTAINER_HIGHEST
)

private val YellowLightTokens = ThemeColorTokens(
    primary = Color(0xFFCA8A04),
    onPrimary = Slate950,
    primaryContainer = Color(0xFFFEF3C7),
    onPrimaryContainer = Color(0xFF422006),
    secondary = Color(0xFFEAB308),
    onSecondary = Slate950,
    secondaryContainer = Color(0xFFFEF9C3),
    onSecondaryContainer = Color(0xFF422006),
    tertiary = Color(0xFFFACC15),
    onTertiary = Slate950,
    tertiaryContainer = Color(0xFFFEF08A),
    onTertiaryContainer = Color(0xFF422006),
    background = COMMON_LIGHT_BACKGROUND,
    onBackground = Slate900,
    surface = Color.White,
    onSurface = Slate900,
    surfaceVariant = Slate50,
    onSurfaceVariant = Slate600,
    outline = Slate300,
    outlineVariant = Slate200,
    surfaceContainerLowest = Color.White,
    surfaceContainerLow = Color.White,
    surfaceContainer = COMMON_LIGHT_TAB_CONTAINER,
    surfaceContainerHigh = Slate100,
    surfaceContainerHighest = Slate200
)

private val YellowDarkTokens = ThemeColorTokens(
    primary = Color(0xFFFDE68A),
    onPrimary = Slate950,
    primaryContainer = Color(0xFF713F12),
    onPrimaryContainer = Color(0xFFFEF3C7),
    secondary = Color(0xFFFACC15),
    onSecondary = Slate950,
    secondaryContainer = Color(0xFF854D0E),
    onSecondaryContainer = Color(0xFFFEF3C7),
    tertiary = Color(0xFFFEF08A),
    onTertiary = Slate950,
    tertiaryContainer = Color(0xFFA16207),
    onTertiaryContainer = Color(0xFFFEF3C7),
    background = COMMON_DARK_BACKGROUND,
    onBackground = Slate100,
    surface = COMMON_DARK_SURFACE,
    onSurface = Slate100,
    surfaceVariant = COMMON_DARK_SURFACE_VARIANT,
    onSurfaceVariant = Slate300,
    outline = Slate500,
    outlineVariant = Slate700,
    surfaceContainerLowest = COMMON_DARK_SURFACE_CONTAINER_LOWEST,
    surfaceContainerLow = COMMON_DARK_SURFACE_CONTAINER_LOW,
    surfaceContainer = COMMON_DARK_SURFACE_CONTAINER,
    surfaceContainerHigh = COMMON_DARK_SURFACE_CONTAINER_HIGH,
    surfaceContainerHighest = COMMON_DARK_SURFACE_CONTAINER_HIGHEST
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

private val LinenTokens = ThemeColorTokens(
    primary = Color(0xFF5B5147),
    onPrimary = Color.White,
    primaryContainer = Color(0xFFE4DDD3),
    onPrimaryContainer = Color(0xFF2D2823),
    secondary = Color(0xFF8B6F52),
    onSecondary = Color.White,
    secondaryContainer = Color(0xFFE8D9C8),
    onSecondaryContainer = Color(0xFF34271B),
    tertiary = Color(0xFF65705F),
    onTertiary = Color.White,
    tertiaryContainer = Color(0xFFDCE5D7),
    onTertiaryContainer = Color(0xFF20281E),
    background = Color(0xFFE7E1D8),
    onBackground = Color(0xFF302A25),
    surface = Color(0xFFF7F3EC),
    onSurface = Color(0xFF302A25),
    surfaceVariant = Color(0xFFDDD5C9),
    onSurfaceVariant = Color(0xFF6B6259),
    outline = Color(0xFFA69B8E),
    outlineVariant = Color(0xFFC9BFB2),
    surfaceContainerLowest = Color(0xFFFCF9F4),
    surfaceContainerLow = Color(0xFFF7F3EC),
    surfaceContainer = Color(0xFFEDE7DE),
    surfaceContainerHigh = Color(0xFFE2DBD1),
    surfaceContainerHighest = Color(0xFFD5CCC0)
)

private val MintTokens = ThemeColorTokens(
    primary = Color(0xFF247A57),
    onPrimary = Color.White,
    primaryContainer = Color(0xFFCBEAD8),
    onPrimaryContainer = Color(0xFF073B25),
    secondary = Color(0xFF3B8D6E),
    onSecondary = Color.White,
    secondaryContainer = Color(0xFFD5EEE1),
    onSecondaryContainer = Color(0xFF123B2A),
    tertiary = Color(0xFF5B7F6B),
    onTertiary = Color.White,
    tertiaryContainer = Color(0xFFD8E8DD),
    onTertiaryContainer = Color(0xFF1D3326),
    background = Color(0xFFE8F5EE),
    onBackground = Color(0xFF1E3027),
    surface = Color(0xFFF7FCF9),
    onSurface = Color(0xFF1E3027),
    surfaceVariant = Color(0xFFD4EBDD),
    onSurfaceVariant = Color(0xFF4F6B5B),
    outline = Color(0xFF8EAA9A),
    outlineVariant = Color(0xFFBBD5C5),
    surfaceContainerLowest = Color(0xFFFCFFFD),
    surfaceContainerLow = Color(0xFFF7FCF9),
    surfaceContainer = Color(0xFFEFF8F2),
    surfaceContainerHigh = Color(0xFFE1F1E7),
    surfaceContainerHighest = Color(0xFFD4EBDD)
)

internal fun ThemePalette.definition(): ThemePaletteDefinition = when (this) {
    ThemePalette.Indigo -> ThemePaletteDefinition(
        light = IndigoLightTokens.withSwitchableLightSurface(),
        dark = IndigoDarkTokens.withSwitchableDarkSurface(),
        preview = ThemePreviewColors(
            primary = IndigoLightTokens.primary,
            accent = IndigoLightTokens.secondary,
            surface = IndigoLightTokens.background
        ),
        insightsLight = IndigoLightTokens.toInsightsColorTokens(),
        insightsDark = IndigoDarkTokens.toInsightsColorTokens()
    )
    ThemePalette.Purple -> ThemePaletteDefinition(
        light = PurpleLightTokens.withSwitchableLightSurface(),
        dark = PurpleDarkTokens.withSwitchableDarkSurface(),
        preview = ThemePreviewColors(
            primary = PurpleLightTokens.primary,
            accent = PurpleLightTokens.secondary,
            surface = PurpleLightTokens.background
        ),
        insightsLight = PurpleLightTokens.toInsightsColorTokens(),
        insightsDark = PurpleDarkTokens.toInsightsColorTokens()
    )
    ThemePalette.Grey -> ThemePaletteDefinition(
        light = GreyLightTokens.withSwitchableLightSurface(),
        dark = GreyDarkTokens.withSwitchableDarkSurface(),
        preview = ThemePreviewColors(
            primary = GreyLightTokens.primary,
            accent = GreyLightTokens.secondary,
            surface = GreyLightTokens.background
        ),
        insightsLight = GreyLightTokens.toInsightsColorTokens(),
        insightsDark = GreyDarkTokens.toInsightsColorTokens()
    )
    ThemePalette.Green -> ThemePaletteDefinition(
        light = GreenLightTokens.withSwitchableLightSurface(),
        dark = GreenDarkTokens.withSwitchableDarkSurface(),
        preview = ThemePreviewColors(
            primary = GreenLightTokens.primary,
            accent = GreenLightTokens.secondary,
            surface = GreenLightTokens.background
        ),
        insightsLight = GreenLightTokens.toInsightsColorTokens(),
        insightsDark = GreenDarkTokens.toInsightsColorTokens()
    )
    ThemePalette.Blue -> ThemePaletteDefinition(
        light = BlueLightTokens.withSwitchableLightSurface(),
        dark = BlueDarkTokens.withSwitchableDarkSurface(),
        preview = ThemePreviewColors(
            primary = BlueLightTokens.primary,
            accent = BlueLightTokens.secondary,
            surface = BlueLightTokens.background
        ),
        insightsLight = BlueLightTokens.toInsightsColorTokens(),
        insightsDark = BlueDarkTokens.toInsightsColorTokens()
    )
    ThemePalette.Orange -> ThemePaletteDefinition(
        light = OrangeLightTokens.withSwitchableLightSurface(),
        dark = OrangeDarkTokens.withSwitchableDarkSurface(),
        preview = ThemePreviewColors(
            primary = OrangeLightTokens.primary,
            accent = OrangeLightTokens.secondary,
            surface = OrangeLightTokens.background
        ),
        insightsLight = OrangeLightTokens.toInsightsColorTokens(),
        insightsDark = OrangeDarkTokens.toInsightsColorTokens()
    )
    ThemePalette.Rose -> ThemePaletteDefinition(
        light = RoseLightTokens.withSwitchableLightSurface(),
        dark = RoseDarkTokens.withSwitchableDarkSurface(),
        preview = ThemePreviewColors(
            primary = RoseLightTokens.primary,
            accent = RoseLightTokens.secondary,
            surface = RoseLightTokens.background
        ),
        insightsLight = RoseLightTokens.toInsightsColorTokens(),
        insightsDark = RoseDarkTokens.toInsightsColorTokens()
    )
    ThemePalette.Yellow -> ThemePaletteDefinition(
        light = YellowLightTokens.withSwitchableLightSurface(),
        dark = YellowDarkTokens.withSwitchableDarkSurface(),
        preview = ThemePreviewColors(
            primary = YellowLightTokens.primary,
            accent = YellowLightTokens.secondary,
            surface = YellowLightTokens.background
        ),
        insightsLight = YellowLightTokens.toInsightsColorTokens(),
        insightsDark = YellowDarkTokens.toInsightsColorTokens()
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
        insightsLight = ParchmentTokens.toInsightsColorTokens(),
        insightsDark = ParchmentTokens.toInsightsColorTokens()
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
        insightsLight = SnowfieldTokens.toInsightsColorTokens(),
        insightsDark = SnowfieldTokens.toInsightsColorTokens()
    )
    ThemePalette.Blueprint -> ThemePaletteDefinition(
        light = BlueprintTokens,
        dark = BlueprintTokens,
        preview = ThemePreviewColors(
            primary = BlueprintTokens.primary,
            accent = BlueprintTokens.tertiary,
            surface = BlueprintTokens.background
        ),
        insightsLight = BlueprintTokens.toInsightsColorTokens(),
        insightsDark = BlueprintTokens.toInsightsColorTokens()
    )
    ThemePalette.Newsprint -> ThemePaletteDefinition(
        light = NewsprintTokens,
        dark = NewsprintTokens,
        preview = ThemePreviewColors(
            primary = NewsprintTokens.primary,
            accent = NewsprintTokens.tertiary,
            surface = NewsprintTokens.background
        ),
        insightsLight = NewsprintTokens.toInsightsColorTokens(),
        insightsDark = NewsprintTokens.toInsightsColorTokens()
    )
    ThemePalette.InkWash -> ThemePaletteDefinition(
        light = InkWashTokens,
        dark = InkWashTokens,
        preview = ThemePreviewColors(
            primary = InkWashTokens.primary,
            accent = InkWashTokens.tertiary,
            surface = InkWashTokens.background
        ),
        insightsLight = InkWashTokens.toInsightsColorTokens(),
        insightsDark = InkWashTokens.toInsightsColorTokens()
    )
    ThemePalette.Kraft -> ThemePaletteDefinition(
        light = KraftTokens,
        dark = KraftTokens,
        preview = ThemePreviewColors(
            primary = KraftTokens.primary,
            accent = KraftTokens.tertiary,
            surface = KraftTokens.background
        ),
        insightsLight = KraftTokens.toInsightsColorTokens(),
        insightsDark = KraftTokens.toInsightsColorTokens()
    )
    ThemePalette.Linen -> ThemePaletteDefinition(
        light = LinenTokens,
        dark = LinenTokens,
        preview = ThemePreviewColors(
            primary = LinenTokens.primary,
            accent = LinenTokens.tertiary,
            surface = LinenTokens.background
        ),
        insightsLight = LinenTokens.toInsightsColorTokens(),
        insightsDark = LinenTokens.toInsightsColorTokens()
    )
    ThemePalette.Mint -> ThemePaletteDefinition(
        light = MintTokens,
        dark = MintTokens,
        preview = ThemePreviewColors(
            primary = MintTokens.primary,
            accent = MintTokens.tertiary,
            surface = MintTokens.background
        ),
        insightsLight = MintTokens.toInsightsColorTokens(),
        insightsDark = MintTokens.toInsightsColorTokens()
    )
}
