package com.example.tracer

import androidx.compose.ui.graphics.Color
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotEquals
import org.junit.Test

class InsightsPiePaletteTest {
    @Test
    fun resolvePieSliceColor_keepsRootColorStable() {
        val first = resolvePieSliceColor(
            InsightsCompositionSlice(root = "study", measureValue = 3600L, percent = 50f),
            InsightsPiePalettePreset.SOFT
        )
        val second = resolvePieSliceColor(
            InsightsCompositionSlice(root = "study", measureValue = 1800L, percent = 25f),
            InsightsPiePalettePreset.SOFT
        )

        assertEquals(first, second)
    }

    @Test
    fun resolvePieSliceColor_reservesNeutralGrayForOthers() {
        val color = resolvePieSliceColor(
            InsightsCompositionSlice(root = "Others", measureValue = 900L, percent = 10f),
            InsightsPiePalettePreset.SOFT
        )

        assertEquals(Color(0xFF94A3B8), color)
        assertNotEquals(
            color,
            resolvePieSliceColor(
                InsightsCompositionSlice(root = "study", measureValue = 3600L, percent = 50f),
                InsightsPiePalettePreset.SOFT
            )
        )
    }

    @Test
    fun resolveAdjacentPieSliceColors_keepsEveryAdjacentPairDistinct() {
        val colors = resolveAdjacentPieSliceColors(
            slices = List(4) {
                InsightsCompositionSlice(
                    root = "same-name",
                    measureValue = 900L,
                    percent = 25f
                )
            },
            palettePreset = InsightsPiePalettePreset.SOFT
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
                InsightsCompositionSlice(root = "study", measureValue = 900L, percent = 15f),
                InsightsCompositionSlice(root = "sleep", measureValue = 3_600L, percent = 60f),
                InsightsCompositionSlice(root = "meal", measureValue = 1_500L, percent = 25f)
            ),
            palettePreset = InsightsPiePalettePreset.SOFT
        )
        val second = resolveAdjacentPieSliceColors(
            slices = listOf(
                InsightsCompositionSlice(root = "alpha", measureValue = 900L, percent = 15f),
                InsightsCompositionSlice(root = "beta", measureValue = 3_600L, percent = 60f),
                InsightsCompositionSlice(root = "gamma", measureValue = 1_500L, percent = 25f)
            ),
            palettePreset = InsightsPiePalettePreset.SOFT
        )

        assertEquals(first, second)
        assertEquals(Color(0xFF4338CA), first[1])
        assertEquals(Color(0xFF0F766E), first[2])
        assertEquals(Color(0xFFBE123C), first[0])
    }

    @Test
    fun insightsPiePaletteHexColors_providesTenColorsForEveryPreset() {
        InsightsPiePalettePreset.entries.forEach { preset ->
            assertEquals(10, insightsPiePaletteHexColors(preset).size)
        }
    }

    @Test
    fun insightsPiePalettePresets_listsVividFirst() {
        assertEquals(InsightsPiePalettePreset.VIVID, InsightsPiePalettePreset.entries.first())
    }
}
