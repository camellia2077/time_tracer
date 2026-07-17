package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotEquals
import org.junit.Test

class ReportTreemapPaletteTest {
    @Test
    fun resolveTreemapColors_assignsDistinctColorsToEveryAdjacentTile() {
        val layout = computeTreemapRects(
            slices = listOf(
                ReportCompositionSlice("study", 4_000L, 40f),
                ReportCompositionSlice("sleep", 2_500L, 25f),
                ReportCompositionSlice("exercise", 1_500L, 15f),
                ReportCompositionSlice("reading", 1_200L, 12f),
                ReportCompositionSlice("meal", 800L, 8f)
            ),
            widthPx = 400f,
            heightPx = 260f
        )

        ReportPiePalettePreset.entries.forEach { palettePreset ->
            val colors = resolveTreemapColors(layout, palettePreset)
            assertEquals(
                resolveReportBreakdownPaletteColors(palettePreset).first(),
                colors[layout.first().index]
            )
            layout.indices.forEach { firstIndex ->
                layout.drop(firstIndex + 1).forEach { second ->
                    val first = layout[firstIndex]
                    if (areTreemapNodesAdjacent(first, second)) {
                        assertNotEquals(colors[first.index], colors[second.index])
                    }
                }
            }
        }
    }
}
