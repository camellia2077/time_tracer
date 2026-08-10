package com.example.tracer

import android.graphics.Color as AndroidColor
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.toArgb

private const val OTHERS_ROOT = "Others"

@Composable
internal fun rememberCompositionSliceColors(
    slices: List<InsightsCompositionSlice>,
    palettePreset: InsightsPiePalettePreset
): List<Color> = remember(slices, palettePreset) {
        slices.map { slice -> resolveCompositionSliceColor(slice, palettePreset) }
    }

internal fun resolveCompositionSliceColor(
    slice: InsightsCompositionSlice,
    palettePreset: InsightsPiePalettePreset
): Color {
    if (slice.root == OTHERS_ROOT) {
        return parsePiePaletteHexColor(insightsPiePaletteOthersHexColor()) ?: Color(0xFF94A3B8)
    }
    // Pie and bar share the same root-color mapping so a root keeps one stable color
    // across day composition views instead of drifting when users switch visuals.
    val palette = resolveInsightsBreakdownPaletteColors(palettePreset)
    val paletteIndex = slice.root.hashCode().mod(palette.size)
    return palette[paletteIndex]
}

@Composable
internal fun rememberPieSliceColors(
    slices: List<InsightsCompositionSlice>,
    palettePreset: InsightsPiePalettePreset
): List<Color> = remember(slices, palettePreset) {
    resolveAdjacentPieSliceColors(slices, palettePreset)
}

internal fun resolvePieSliceColor(
    slice: InsightsCompositionSlice,
    palettePreset: InsightsPiePalettePreset
): Color = resolveCompositionSliceColor(slice = slice, palettePreset = palettePreset)

internal fun resolveAdjacentPieSliceColors(
    slices: List<InsightsCompositionSlice>,
    palettePreset: InsightsPiePalettePreset
): List<Color> {
    val palette = resolveInsightsBreakdownPaletteColors(palettePreset)
    // Preserve draw order while deriving palette positions from the actual weight.
    // Ties keep their input order, making equal slices deterministic as well.
    val rankBySliceIndex = IntArray(slices.size)
    slices.indices
        .sortedWith(compareByDescending<Int> { slices[it].durationSeconds }.thenBy { it })
        .forEachIndexed { rank, sliceIndex -> rankBySliceIndex[sliceIndex] = rank }
    return buildList {
        slices.forEachIndexed { index, slice ->
            val baseColor = palette[rankBySliceIndex[index].mod(palette.size)]
            val previousColor = lastOrNull()
            val firstColor = firstOrNull()
            val forbidden = buildSet {
                previousColor?.let(::add)
                if (index == slices.lastIndex) {
                    firstColor?.let(::add)
                }
            }
            add(resolveNonAdjacentPieColor(baseColor, forbidden, palette))
        }
    }
}

private fun resolveNonAdjacentPieColor(
    baseColor: Color,
    forbidden: Set<Color>,
    palette: List<Color>
): Color {
    if (baseColor !in forbidden) {
        return baseColor
    }
    palette.firstOrNull { it !in forbidden }?.let { return it }

    val hsv = FloatArray(3)
    AndroidColor.colorToHSV(baseColor.toArgb(), hsv)
    hsv[0] = (hsv[0] + 137f) % 360f
    hsv[1] = (hsv[1] * 0.9f).coerceIn(0.45f, 1f)
    return Color(AndroidColor.HSVToColor(hsv))
}

internal fun resolveInsightsBreakdownPaletteColors(
    palettePreset: InsightsPiePalettePreset
): List<Color> = insightsPiePaletteHexColors(palettePreset)
    .mapNotNull(::parsePiePaletteHexColor)
    .ifEmpty { listOf(Color(0xFF4F46E5), Color(0xFF0F766E), Color(0xFFB45309)) }

internal fun resolvePiePalettePreviewColors(
    palettePreset: InsightsPiePalettePreset
): List<Color> = resolveInsightsBreakdownPaletteColors(palettePreset)

private fun parsePiePaletteHexColor(raw: String): Color? = runCatching {
    val normalized = raw.trim().removePrefix("#")
    when (normalized.length) {
        6 -> Color(0xFF000000L or normalized.toLong(16))
        8 -> Color(normalized.toLong(16))
        else -> error("Unsupported color format")
    }
}.getOrNull()
