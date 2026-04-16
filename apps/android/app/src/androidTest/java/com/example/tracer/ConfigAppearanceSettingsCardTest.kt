package com.example.tracer

import androidx.activity.ComponentActivity
import androidx.compose.ui.test.assertCountEquals
import androidx.compose.ui.test.assertIsDisplayed
import androidx.compose.ui.test.junit4.createAndroidComposeRule
import androidx.compose.ui.test.onAllNodesWithTag
import androidx.compose.ui.test.onNodeWithText
import androidx.compose.ui.test.performClick
import androidx.test.ext.junit.runners.AndroidJUnit4
import com.example.tracer.data.AppLanguage
import com.example.tracer.data.DarkThemeStyle
import com.example.tracer.data.ThemeColor
import com.example.tracer.data.ThemeConfig
import com.example.tracer.data.ThemeMode
import com.example.tracer.ui.theme.TracerTheme
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class ConfigAppearanceSettingsCardTest {
    @get:Rule
    val composeRule = createAndroidComposeRule<ComponentActivity>()

    @Test
    fun reportPaletteCollapsed_showsOnlySummarySwatches() {
        setAppearanceCardContent()

        composeRule
            .onAllNodesWithTag("config_report_palette_summary_swatches")
            .assertCountEquals(1)
        composeRule
            .onAllNodesWithTag("config_report_palette_expanded_content")
            .assertCountEquals(0)
        composeRule
            .onAllNodesWithTag("config_report_palette_expanded_bar_preview")
            .assertCountEquals(0)
        composeRule
            .onAllNodesWithTag("config_report_palette_preset_swatches")
            .assertCountEquals(0)
    }

    @Test
    fun reportPaletteExpanded_keepsBarPreviewOnlyInMainExpandedArea() {
        setAppearanceCardContent()

        composeRule
            .onNodeWithText(
                composeRule.activity.getString(
                    R.string.config_report_breakdown_palette_tap_expand
                )
            )
            .performClick()

        composeRule
            .onAllNodesWithTag("config_report_palette_summary_swatches")
            .assertCountEquals(1)
        composeRule
            .onAllNodesWithTag("config_report_palette_expanded_content")
            .assertCountEquals(1)
        composeRule
            .onAllNodesWithTag("config_report_palette_expanded_bar_preview")
            .assertCountEquals(1)
        composeRule
            .onAllNodesWithTag("config_report_palette_preset_swatches")
            .assertCountEquals(ReportPiePalettePreset.entries.size)
        composeRule
            .onNodeWithText(
                composeRule.activity.getString(
                    R.string.config_report_breakdown_palette_bar_preview
                )
            )
            .assertIsDisplayed()
    }

    private fun setAppearanceCardContent() {
        val themeConfig = ThemeConfig(
            themeColor = ThemeColor.Slate,
            themeMode = ThemeMode.Light,
            useDynamicColor = false,
            darkThemeStyle = DarkThemeStyle.Tinted
        )

        composeRule.setContent {
            TracerTheme(themeConfig = themeConfig) {
                AppearanceSettingsCard(
                    themeConfig = themeConfig,
                    onSetThemeColor = {},
                    onSetThemeMode = {},
                    onSetUseDynamicColor = {},
                    onSetDarkThemeStyle = {},
                    reportPiePalettePreset = ReportPiePalettePreset.SOFT,
                    onReportPiePalettePresetChange = {},
                    appLanguage = AppLanguage.English,
                    onSetAppLanguage = {}
                )
            }
        }
    }
}
