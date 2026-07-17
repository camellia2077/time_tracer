package com.example.tracer

import androidx.compose.ui.graphics.Color
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotEquals
import org.junit.Test

class ReportPiePaletteTest {
    @Test
    fun resolvePieSliceColor_keepsRootColorStable() {
        val first = resolvePieSliceColor(
            ReportCompositionSlice(root = "study", durationSeconds = 3600L, percent = 50f),
            ReportPiePalettePreset.SOFT
        )
        val second = resolvePieSliceColor(
            ReportCompositionSlice(root = "study", durationSeconds = 1800L, percent = 25f),
            ReportPiePalettePreset.SOFT
        )

        assertEquals(first, second)
    }

    @Test
    fun resolvePieSliceColor_reservesNeutralGrayForOthers() {
        val color = resolvePieSliceColor(
            ReportCompositionSlice(root = "Others", durationSeconds = 900L, percent = 10f),
            ReportPiePalettePreset.SOFT
        )

        assertEquals(Color(0xFF94A3B8), color)
        assertNotEquals(
            color,
            resolvePieSliceColor(
                ReportCompositionSlice(root = "study", durationSeconds = 3600L, percent = 50f),
                ReportPiePalettePreset.SOFT
            )
        )
    }

    @Test
    fun resolveAdjacentPieSliceColors_keepsEveryAdjacentPairDistinct() {
        val colors = resolveAdjacentPieSliceColors(
            slices = List(4) {
                ReportCompositionSlice(
                    root = "same-name",
                    durationSeconds = 900L,
                    percent = 25f
                )
            },
            palettePreset = ReportPiePalettePreset.SOFT
        )

        assertNotEquals(colors[0], colors[1])
        assertNotEquals(colors[1], colors[2])
        assertNotEquals(colors[2], colors[3])
        assertNotEquals(colors[3], colors[0])
    }

    @Test
    fun resolveAdjacentPieSliceColors_assignsColorsByWeightRankInsteadOfName() {
        val first = resolveAdjacentPieSliceColors(
            slices = listOf(
                ReportCompositionSlice(root = "study", durationSeconds = 900L, percent = 15f),
                ReportCompositionSlice(root = "sleep", durationSeconds = 3_600L, percent = 60f),
                ReportCompositionSlice(root = "meal", durationSeconds = 1_500L, percent = 25f)
            ),
            palettePreset = ReportPiePalettePreset.SOFT
        )
        val second = resolveAdjacentPieSliceColors(
            slices = listOf(
                ReportCompositionSlice(root = "alpha", durationSeconds = 900L, percent = 15f),
                ReportCompositionSlice(root = "beta", durationSeconds = 3_600L, percent = 60f),
                ReportCompositionSlice(root = "gamma", durationSeconds = 1_500L, percent = 25f)
            ),
            palettePreset = ReportPiePalettePreset.SOFT
        )

        assertEquals(first, second)
        assertEquals(Color(0xFF4338CA), first[1])
        assertEquals(Color(0xFF0F766E), first[2])
        assertEquals(Color(0xFFBE123C), first[0])
    }

    @Test
    fun reportPiePaletteHexColors_providesTenColorsForEveryPreset() {
        ReportPiePalettePreset.entries.forEach { preset ->
            assertEquals(10, reportPiePaletteHexColors(preset).size)
        }
    }
}
