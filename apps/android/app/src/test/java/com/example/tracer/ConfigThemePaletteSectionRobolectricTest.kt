package com.example.tracer

import androidx.compose.ui.test.assertCountEquals
import androidx.compose.ui.test.junit4.createComposeRule
import androidx.compose.ui.test.onAllNodesWithTag
import com.example.tracer.data.DarkThemeStyle
import com.example.tracer.data.ThemeConfig
import com.example.tracer.data.ThemeMode
import com.example.tracer.data.ThemePalette
import com.example.tracer.ui.theme.TracerTheme
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class ConfigThemePaletteSectionRobolectricTest {
    @get:Rule
    val composeRule = createComposeRule()

    @Test
    fun collapsed_showsOnlyTheCurrentPalettePreview() {
        setThemePaletteSectionContent(expanded = false)

        summaryPreviews().assertCountEquals(1)
        paletteOptions().assertCountEquals(0)
    }

    @Test
    fun expanded_showsEveryPaletteOption() {
        setThemePaletteSectionContent(expanded = true)

        summaryPreviews().assertCountEquals(1)
        paletteOptions().assertCountEquals(ThemePalette.entries.size)
    }

    private fun summaryPreviews() = composeRule.onAllNodesWithTag(
        "config_theme_palette_summary_preview",
        useUnmergedTree = true
    )

    private fun paletteOptions() = composeRule.onAllNodesWithTag(
        "config_theme_palette_option",
        useUnmergedTree = true
    )

    private fun setThemePaletteSectionContent(expanded: Boolean) {
        val themeConfig = ThemeConfig(
            themeMode = ThemeMode.Light,
            palette = ThemePalette.Indigo,
            darkThemeStyle = DarkThemeStyle.Tinted
        )
        composeRule.setContent {
            TracerTheme(themeConfig = themeConfig) {
                ThemePaletteSection(
                    selectedThemePalette = ThemePalette.Indigo,
                    onSetThemePalette = {},
                    expanded = expanded,
                    onToggleExpanded = {}
                )
            }
        }
    }
}
