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
    fun insightsTokensReuseThemeSemanticRoles() {
        ThemePalette.entries.forEach { palette ->
            val definition = palette.definition()

            assertEquals(definition.light.primary, definition.insightsLight.treeHierarchy)
            assertEquals(definition.light.secondary, definition.insightsLight.treeProgress)
            assertEquals(definition.light.tertiary, definition.insightsLight.timelineDuration)
            assertEquals(definition.light.outlineVariant, definition.insightsLight.track)
            assertEquals(definition.light.surfaceVariant, definition.insightsLight.gap)

            assertEquals(definition.dark.primary, definition.insightsDark.treeHierarchy)
            assertEquals(definition.dark.secondary, definition.insightsDark.treeProgress)
            assertEquals(definition.dark.tertiary, definition.insightsDark.timelineDuration)
            assertEquals(definition.dark.outlineVariant, definition.insightsDark.track)
            assertEquals(definition.dark.surfaceVariant, definition.insightsDark.gap)
        }
    }

    @Test
    fun insightsProgressColorsRemainStableForAllPalettes() {
        assertEquals(
            Color(0xFF2563EB),
            ThemePalette.Indigo.definition().insightsLight.treeProgress
        )
        assertEquals(
            Color(0xFFD97706),
            ThemePalette.GraphiteAmber.definition().insightsLight.treeProgress
        )
        assertEquals(
            Color(0xFF0D9488),
            ThemePalette.Teal.definition().insightsLight.treeProgress
        )
        assertEquals(
            Color(0xFF60A5FA),
            ThemePalette.Indigo.definition().insightsDark.treeProgress
        )
        assertEquals(
            Color(0xFFFBBF24),
            ThemePalette.GraphiteAmber.definition().insightsDark.treeProgress
        )
        assertEquals(
            Color(0xFF2DD4BF),
            ThemePalette.Teal.definition().insightsDark.treeProgress
        )
        assertEquals(
            Color(0xFFC78C25),
            ThemePalette.Parchment.definition().insightsLight.treeProgress
        )
        assertEquals(
            ThemePalette.Parchment.definition().insightsLight,
            ThemePalette.Parchment.definition().insightsDark
        )
        assertEquals(
            Color(0xFF2E3440),
            ThemePalette.Snowfield.definition().insightsLight.treeProgress
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
            ThemePalette.Snowfield.definition().insightsLight,
            ThemePalette.Snowfield.definition().insightsDark
        )
        assertEquals(
            Color(0xFFEA580C),
            ThemePalette.Orange.definition().insightsLight.treeProgress
        )
        assertEquals(
            Color(0xFFE11D48),
            ThemePalette.Rose.definition().insightsLight.treeProgress
        )
        assertEquals(
            Color(0xFFD97706),
            ThemePalette.Amber.definition().insightsLight.treeProgress
        )
        assertEquals(
            Color(0xFF4EA5D9),
            ThemePalette.Blueprint.definition().insightsLight.treeProgress
        )
        assertEquals(
            Color(0xFFA33F3F),
            ThemePalette.Newsprint.definition().insightsLight.treeProgress
        )
        assertEquals(
            Color(0xFFB23A2B),
            ThemePalette.InkWash.definition().insightsLight.treeProgress
        )
        assertEquals(
            Color(0xFF9A5B2F),
            ThemePalette.Kraft.definition().insightsLight.treeProgress
        )
        assertEquals(
            ThemePalette.Blueprint.definition().insightsLight,
            ThemePalette.Blueprint.definition().insightsDark
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
