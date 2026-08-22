package com.example.tracer

import androidx.compose.ui.test.assertCountEquals
import androidx.compose.ui.test.junit4.createComposeRule
import androidx.compose.ui.test.onAllNodesWithTag
import androidx.compose.ui.test.onNodeWithText
import androidx.compose.ui.test.performClick
import com.example.tracer.data.AppLanguage
import com.example.tracer.data.DarkThemeStyle
import com.example.tracer.data.PageTransitionStyle
import com.example.tracer.data.ThemeConfig
import com.example.tracer.data.ThemeMode
import com.example.tracer.ui.theme.TracerTheme
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class ConfigApplicationPreferencesCardRobolectricTest {
    @get:Rule
    val composeRule = createComposeRule()

    @Test
    fun pageTransitionSelector_isVisibleWhenTransitionsAreEnabled() {
        setCardContent(pageTransitionsEnabled = true)
        expandPageTransitionSettings()

        pageTransitionOptions().assertCountEquals(1)
    }

    @Test
    fun pageTransitionSelector_isHiddenWhenTransitionsAreDisabled() {
        setCardContent(pageTransitionsEnabled = false)
        expandPageTransitionSettings()

        pageTransitionOptions().assertCountEquals(0)
    }

    private fun pageTransitionOptions() =
        composeRule.onAllNodesWithTag("config_page_transition_options", useUnmergedTree = true)

    private fun expandPageTransitionSettings() {
        composeRule.onNodeWithText("Page transition").performClick()
    }

    private fun setCardContent(pageTransitionsEnabled: Boolean) {
        composeRule.setContent {
            TracerTheme(
                themeConfig = ThemeConfig(
                    themeMode = ThemeMode.Light,
                    darkThemeStyle = DarkThemeStyle.Tinted
                )
            ) {
                ConfigApplicationPreferencesCard(
                    appLanguage = AppLanguage.English,
                    onSetAppLanguage = {},
                    promptBeforeUnconfiguredActivityRecord = false,
                    onPromptBeforeUnconfiguredActivityRecordChange = {},
                    pageTransitionsEnabled = pageTransitionsEnabled,
                    onPageTransitionsEnabledChange = {},
                    pageTransitionStyle = PageTransitionStyle.FADE,
                    onPageTransitionStyleChange = {}
                )
            }
        }
    }
}
