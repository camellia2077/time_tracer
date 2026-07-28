package com.example.tracer.ui.theme

import androidx.compose.ui.graphics.Color
import com.example.tracer.data.ThemePalette
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class ThemePaletteDefinitionTest {
    @Test
    fun everyPaletteProvidesLightDarkAndPreviewTokens() {
        ThemePalette.entries.forEach { palette ->
            val definition = palette.definition()

            assertTrue(definition.light.primary != Color.Unspecified)
            assertTrue(definition.dark.primary != Color.Unspecified)
            assertEquals(definition.light.primary, definition.preview.primary)
            assertTrue(definition.preview.accent != Color.Unspecified)
            assertEquals(definition.light.background, definition.preview.surface)
        }
    }

    @Test
    fun reportTokensReuseThemeSemanticRoles() {
        ThemePalette.entries.forEach { palette ->
            val definition = palette.definition()

            assertEquals(definition.light.primary, definition.reportLight.treeHierarchy)
            assertEquals(definition.light.secondary, definition.reportLight.treeProgress)
            assertEquals(definition.light.tertiary, definition.reportLight.timelineDuration)
            assertEquals(definition.light.outlineVariant, definition.reportLight.track)
            assertEquals(definition.light.surfaceVariant, definition.reportLight.gap)

            assertEquals(definition.dark.primary, definition.reportDark.treeHierarchy)
            assertEquals(definition.dark.secondary, definition.reportDark.treeProgress)
            assertEquals(definition.dark.tertiary, definition.reportDark.timelineDuration)
            assertEquals(definition.dark.outlineVariant, definition.reportDark.track)
            assertEquals(definition.dark.surfaceVariant, definition.reportDark.gap)
        }
    }

    @Test
    fun reportProgressColorsRemainStableForAllPalettes() {
        assertEquals(
            Color(0xFF2563EB),
            ThemePalette.Indigo.definition().reportLight.treeProgress
        )
        assertEquals(
            Color(0xFFD97706),
            ThemePalette.GraphiteAmber.definition().reportLight.treeProgress
        )
        assertEquals(
            Color(0xFF0D9488),
            ThemePalette.Teal.definition().reportLight.treeProgress
        )
        assertEquals(
            Color(0xFF60A5FA),
            ThemePalette.Indigo.definition().reportDark.treeProgress
        )
        assertEquals(
            Color(0xFFFBBF24),
            ThemePalette.GraphiteAmber.definition().reportDark.treeProgress
        )
        assertEquals(
            Color(0xFF2DD4BF),
            ThemePalette.Teal.definition().reportDark.treeProgress
        )
        assertEquals(
            Color(0xFFC78C25),
            ThemePalette.Parchment.definition().reportLight.treeProgress
        )
        assertEquals(
            ThemePalette.Parchment.definition().reportLight,
            ThemePalette.Parchment.definition().reportDark
        )
        assertEquals(
            Color(0xFF2E3440),
            ThemePalette.Snowfield.definition().reportLight.treeProgress
        )
        assertEquals(
            Color(0xFFE5E9F0),
            ThemePalette.Snowfield.definition().light.background
        )
        assertEquals(
            Color(0xFF4C566A),
            ThemePalette.Snowfield.definition().light.primary
        )
        assertEquals(
            ThemePalette.Snowfield.definition().reportLight,
            ThemePalette.Snowfield.definition().reportDark
        )
        assertEquals(
            Color(0xFFEA580C),
            ThemePalette.Orange.definition().reportLight.treeProgress
        )
        assertEquals(
            Color(0xFFE11D48),
            ThemePalette.Rose.definition().reportLight.treeProgress
        )
        assertEquals(
            Color(0xFFD97706),
            ThemePalette.Amber.definition().reportLight.treeProgress
        )
        assertEquals(
            Color(0xFF4EA5D9),
            ThemePalette.Blueprint.definition().reportLight.treeProgress
        )
        assertEquals(
            Color(0xFFA33F3F),
            ThemePalette.Newsprint.definition().reportLight.treeProgress
        )
        assertEquals(
            Color(0xFFB23A2B),
            ThemePalette.InkWash.definition().reportLight.treeProgress
        )
        assertEquals(
            Color(0xFF9A5B2F),
            ThemePalette.Kraft.definition().reportLight.treeProgress
        )
        assertEquals(
            ThemePalette.Blueprint.definition().reportLight,
            ThemePalette.Blueprint.definition().reportDark
        )
        assertEquals(
            Color(0xFF6D5B45),
            ThemePalette.Parchment.definition().preview.accent
        )
        assertEquals(
            Color(0xFF81A1C1),
            ThemePalette.Snowfield.definition().preview.accent
        )
        assertEquals(
            Color(0xFF6A7078),
            ThemePalette.Newsprint.definition().preview.accent
        )
    }

    @Test
    fun blueprintCardsAreDistinctFromThePageBackground() {
        val tokens = ThemePalette.Blueprint.definition().light

        assertTrue(tokens.surfaceContainerLow != tokens.background)
        assertTrue(tokens.surfaceContainer != tokens.surfaceContainerLow)
    }
}
