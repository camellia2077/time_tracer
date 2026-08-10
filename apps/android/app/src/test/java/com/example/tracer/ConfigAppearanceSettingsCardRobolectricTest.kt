package com.example.tracer

import android.content.Context
import androidx.compose.ui.test.assertCountEquals
import androidx.compose.ui.test.junit4.createComposeRule
import androidx.compose.ui.test.onAllNodesWithTag
import androidx.test.core.app.ApplicationProvider
import com.example.tracer.data.DarkThemeStyle
import com.example.tracer.data.ThemeConfig
import com.example.tracer.data.ThemeMode
import com.example.tracer.ui.theme.TracerTheme
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class ConfigAppearanceSettingsCardRobolectricTest {
    @get:Rule
    val composeRule = createComposeRule()

    private val context: Context = ApplicationProvider.getApplicationContext()

    private fun summarySwatches() =
        composeRule.onAllNodesWithTag(
            "config_insights_palette_summary_swatches",
            useUnmergedTree = true
        )

    private fun expandedContent() =
        composeRule.onAllNodesWithTag(
            "config_insights_palette_expanded_content",
            useUnmergedTree = true
        )

    private fun expandedBarPreview() =
        composeRule.onAllNodesWithTag(
            "config_insights_palette_expanded_bar_preview",
            useUnmergedTree = true
        )

    private fun presetSwatches() =
        composeRule.onAllNodesWithTag(
            "config_insights_palette_preset_swatches",
            useUnmergedTree = true
        )

    @Test
    fun insightsPaletteCollapsed_showsOnlySummarySwatches() {
        setInsightsSettingsCardContent(initialInsightsPaletteExpanded = false)

        summarySwatches().assertCountEquals(1)
        expandedContent().assertCountEquals(0)
        expandedBarPreview().assertCountEquals(0)
        presetSwatches().assertCountEquals(0)
    }

    @Test
    fun insightsPaletteExpanded_keepsBarPreviewOnlyInMainExpandedArea() {
        setInsightsSettingsCardContent(initialInsightsPaletteExpanded = true)

        summarySwatches().assertCountEquals(1)
        expandedContent().assertCountEquals(1)
        expandedBarPreview().assertCountEquals(1)
        presetSwatches().assertCountEquals(InsightsPiePalettePreset.entries.size)
    }

    private fun setInsightsSettingsCardContent(
        initialInsightsPaletteExpanded: Boolean
    ) {
        val themeConfig = ThemeConfig(
            themeMode = ThemeMode.Light,
            darkThemeStyle = DarkThemeStyle.Tinted
        )

        composeRule.setContent {
            TracerTheme(themeConfig = themeConfig) {
                ConfigInsightsAverageDayBasisCard(
                    insightsPiePalettePreset = InsightsPiePalettePreset.SOFT,
                    onInsightsPiePalettePresetChange = {},
                    initialInsightsPaletteExpanded = initialInsightsPaletteExpanded,
                    selected = InsightsAverageDayBasis.ACTIVE_DAYS,
                    onSelected = {}
                )
            }
        }
    }
}
