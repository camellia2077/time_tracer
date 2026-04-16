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
}
