package com.example.tracer.data

import com.example.tracer.InsightsPiePalettePreset
import org.junit.Assert.assertEquals
import org.junit.Test

class InsightsPiePaletteTomlLoaderTest {
    @Test
    fun parse_readsAllPaletteColorsAndOthersColor() {
        val config = InsightsPiePaletteTomlLoader.parse(
            """
            schema_version = 1
            others = "#010203"

            [palettes]
            SOFT = ["#000001", "#000002"]
            EDITORIAL = ["#000001", "#000002"]
            VIVID = ["#000001", "#000002"]
            MONO_ACCENT = ["#000001", "#000002"]
            """.trimIndent()
        )

        // Invalid palette lengths fall back to the complete built-in palette.
        assertEquals(10, config.palettes.getValue(InsightsPiePalettePreset.SOFT).size)
        assertEquals("#010203", config.othersHexColor)
    }

    @Test
    fun parse_usesFallbackForInvalidColors() {
        val config = InsightsPiePaletteTomlLoader.parse(
            """
            [palettes]
            SOFT = ["not-a-color"]
            """.trimIndent()
        )

        assertEquals(10, config.palettes.getValue(InsightsPiePalettePreset.SOFT).size)
        assertEquals("#94A3B8", config.othersHexColor)
    }
}
