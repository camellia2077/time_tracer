package com.example.tracer

import androidx.compose.ui.test.assertCountEquals
import androidx.compose.ui.test.junit4.createComposeRule
import androidx.compose.ui.test.onAllNodesWithTag
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

    private fun comparisonColorOptions() =
        composeRule.onAllNodesWithTag(
            "config_insights_comparison_color_option",
            useUnmergedTree = true
        )

    private fun comparisonIndicatorOptions() =
        composeRule.onAllNodesWithTag(
            "config_insights_comparison_indicator_option",
            useUnmergedTree = true
        )

    private fun comparisonPresentationExample() =
        composeRule.onAllNodesWithTag(
            "config_insights_comparison_presentation_example",
            useUnmergedTree = true
        )

    private fun comparisonSummaryPreview() =
        composeRule.onAllNodesWithTag(
            "config_insights_comparison_summary_preview",
            useUnmergedTree = true
        )

    @Test
    fun insightsPaletteCollapsed_showsOnlySummarySwatches() {
        setInsightsSettingsCardContent(
            insightsChartStyleExpanded = false,
            insightsComparisonExpanded = false
        )

        summarySwatches().assertCountEquals(1)
        expandedContent().assertCountEquals(0)
        expandedBarPreview().assertCountEquals(0)
        presetSwatches().assertCountEquals(0)
        comparisonSummaryPreview().assertCountEquals(1)
        comparisonColorOptions().assertCountEquals(0)
        comparisonIndicatorOptions().assertCountEquals(0)
        comparisonPresentationExample().assertCountEquals(0)
    }

    @Test
    fun insightsPaletteExpanded_keepsBarPreviewOnlyInMainExpandedArea() {
        setInsightsSettingsCardContent(
            insightsChartStyleExpanded = true,
            insightsComparisonExpanded = true
        )

        summarySwatches().assertCountEquals(1)
        expandedContent().assertCountEquals(1)
        expandedBarPreview().assertCountEquals(1)
        presetSwatches().assertCountEquals(InsightsPiePalettePreset.entries.size)
        comparisonSummaryPreview().assertCountEquals(1)
        comparisonColorOptions().assertCountEquals(InsightsComparisonColorScheme.entries.size)
        comparisonIndicatorOptions().assertCountEquals(InsightsComparisonIndicatorStyle.entries.size)
        comparisonPresentationExample().assertCountEquals(1)
    }

    private fun setInsightsSettingsCardContent(
        insightsChartStyleExpanded: Boolean,
        insightsComparisonExpanded: Boolean
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
                    comparisonColorScheme = InsightsComparisonColorScheme.GREEN_RED,
                    onComparisonColorSchemeChange = {},
                    comparisonIndicatorStyle = InsightsComparisonIndicatorStyle.ARROWS,
                    onComparisonIndicatorStyleChange = {},
                    insightsChartStyleExpanded = insightsChartStyleExpanded,
                    onInsightsChartStyleExpandedChange = {},
                    insightsComparisonExpanded = insightsComparisonExpanded,
                    onInsightsComparisonExpandedChange = {},
                    selected = InsightsAverageDayBasis.ACTIVE_DAYS,
                    onSelected = {}
                )
            }
        }
    }
}
