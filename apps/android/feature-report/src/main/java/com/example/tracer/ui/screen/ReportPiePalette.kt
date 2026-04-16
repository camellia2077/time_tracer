package com.example.tracer

import android.graphics.Color as AndroidColor
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.graphics.Color

private const val OTHERS_ROOT = "Others"

@Composable
internal fun rememberCompositionSliceColors(
    slices: List<ReportCompositionSlice>,
    palettePreset: ReportPiePalettePreset
): List<Color> = remember(slices, palettePreset) {
        slices.map { slice -> resolveCompositionSliceColor(slice, palettePreset) }
    }

internal fun resolveCompositionSliceColor(
    slice: ReportCompositionSlice,
    palettePreset: ReportPiePalettePreset
): Color {
    if (slice.root == OTHERS_ROOT) {
        return parsePiePaletteHexColor(reportPiePaletteOthersHexColor()) ?: Color(0xFF94A3B8)
    }
    // Pie and bar share the same root-color mapping so a root keeps one stable color
    // across day composition views instead of drifting when users switch visuals.
    val palette = reportPiePaletteHexColors(palettePreset)
        .mapNotNull(::parsePiePaletteHexColor)
        .ifEmpty { listOf(Color(0xFF4F46E5)) }
    val paletteIndex = slice.root.hashCode().mod(palette.size)
    return palette[paletteIndex]
}

@Composable
internal fun rememberPieSliceColors(
    slices: List<ReportCompositionSlice>,
    palettePreset: ReportPiePalettePreset
): List<Color> = rememberCompositionSliceColors(
    slices = slices,
    palettePreset = palettePreset
)

internal fun resolvePieSliceColor(
    slice: ReportCompositionSlice,
    palettePreset: ReportPiePalettePreset
): Color = resolveCompositionSliceColor(slice = slice, palettePreset = palettePreset)

internal fun resolvePiePalettePreviewColors(
    palettePreset: ReportPiePalettePreset
): List<Color> = reportPiePaletteHexColors(palettePreset)
    .mapNotNull(::parsePiePaletteHexColor)

private fun parsePiePaletteHexColor(raw: String): Color? =
    runCatching { Color(AndroidColor.parseColor(raw.trim())) }.getOrNull()
