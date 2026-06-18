package com.example.tracer

import android.content.Context
import androidx.compose.ui.test.assertCountEquals
import androidx.compose.ui.test.junit4.createComposeRule
import androidx.compose.ui.test.onAllNodesWithTag
import androidx.test.core.app.ApplicationProvider
import com.example.tracer.data.AppLanguage
import com.example.tracer.data.DarkThemeStyle
import com.example.tracer.data.ThemeColor
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
            "config_report_palette_summary_swatches",
            useUnmergedTree = true
        )

    private fun expandedContent() =
        composeRule.onAllNodesWithTag(
            "config_report_palette_expanded_content",
            useUnmergedTree = true
        )

    private fun expandedBarPreview() =
        composeRule.onAllNodesWithTag(
            "config_report_palette_expanded_bar_preview",
            useUnmergedTree = true
        )

    private fun presetSwatches() =
        composeRule.onAllNodesWithTag(
            "config_report_palette_preset_swatches",
            useUnmergedTree = true
        )

    @Test
    fun reportPaletteCollapsed_showsOnlySummarySwatches() {
        setAppearanceCardContent(initialReportPaletteExpanded = false)

        summarySwatches().assertCountEquals(1)
        expandedContent().assertCountEquals(0)
        expandedBarPreview().assertCountEquals(0)
        presetSwatches().assertCountEquals(0)
    }

    @Test
    fun reportPaletteExpanded_keepsBarPreviewOnlyInMainExpandedArea() {
        setAppearanceCardContent(initialReportPaletteExpanded = true)

        summarySwatches().assertCountEquals(1)
        expandedContent().assertCountEquals(1)
        expandedBarPreview().assertCountEquals(1)
        presetSwatches().assertCountEquals(ReportPiePalettePreset.entries.size)
    }

    private fun setAppearanceCardContent(
        initialReportPaletteExpanded: Boolean
    ) {
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
                    onSetAppLanguage = {},
                    initialReportPaletteExpanded = initialReportPaletteExpanded
                )
            }
        }
    }
}
