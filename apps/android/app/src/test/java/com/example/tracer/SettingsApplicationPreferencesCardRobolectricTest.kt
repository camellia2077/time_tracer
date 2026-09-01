package com.example.tracer

import androidx.compose.ui.test.assertCountEquals
import androidx.compose.ui.test.junit4.createComposeRule
import androidx.compose.ui.test.onAllNodesWithTag
import androidx.compose.ui.test.onNodeWithTag
import androidx.compose.ui.test.onNodeWithText
import androidx.compose.ui.test.performClick
import androidx.compose.runtime.mutableStateOf
import com.example.tracer.data.AppLanguage
import com.example.tracer.data.DarkSurfaceStyle
import com.example.tracer.data.PageTransitionStyle
import com.example.tracer.data.ThemeConfig
import com.example.tracer.data.ThemeMode
import com.example.tracer.data.TimeDisplayMode
import com.example.tracer.ui.theme.TracerTheme
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class SettingsApplicationPreferencesCardRobolectricTest {
    @get:Rule
    val composeRule = createComposeRule()

    @Test
    fun pageTransitionSelector_isVisibleWithNoAnimationOption() {
        setCardContent()
        expandPageTransitionSettings()

        pageTransitionOptions().assertCountEquals(1)
        noAnimationOption().performClick()
    }

    @Test
    fun selectingNoAnimation_persistsNoneTransitionStyle_andKeepsSelectorExpanded() {
        var selectedStyle = PageTransitionStyle.FADE
        setCardContent(onPageTransitionStyleChange = { selectedStyle = it })
        expandPageTransitionSettings()

        noAnimationOption().performClick()

        org.junit.Assert.assertEquals(PageTransitionStyle.NONE, selectedStyle)
        pageTransitionOptions().assertCountEquals(1)
    }

    @Test
    fun selectingQuickFade_keepsSelectorExpanded() {
        setCardContent(initialPageTransitionStyle = PageTransitionStyle.NONE)
        expandPageTransitionSettings()

        quickFadeOption().performClick()

        pageTransitionOptions().assertCountEquals(1)
    }

    @Test
    fun selectingLightSlide_keepsSelectorExpanded() {
        setCardContent()
        expandPageTransitionSettings()

        lightSlideOption().performClick()

        pageTransitionOptions().assertCountEquals(1)
    }

    private fun pageTransitionOptions() =
        composeRule.onAllNodesWithTag("settings_page_transition_options", useUnmergedTree = true)

    private fun noAnimationOption() =
        composeRule.onNodeWithTag("settings_page_transition_option_none", useUnmergedTree = true)

    private fun quickFadeOption() =
        composeRule.onNodeWithTag("settings_page_transition_option_fade", useUnmergedTree = true)

    private fun lightSlideOption() =
        composeRule.onNodeWithTag("settings_page_transition_option_slide", useUnmergedTree = true)

    private fun expandPageTransitionSettings() {
        composeRule.onNodeWithText("Page transition").performClick()
    }

    private fun setCardContent(
        initialPageTransitionStyle: PageTransitionStyle = PageTransitionStyle.FADE,
        onPageTransitionStyleChange: (PageTransitionStyle) -> Unit = {}
    ) {
        val pageTransitionOptionsExpanded = mutableStateOf(false)
        val pageTransitionStyle = mutableStateOf(initialPageTransitionStyle)
        composeRule.setContent {
            TracerTheme(
                themeConfig = ThemeConfig(
                    themeMode = ThemeMode.Light,
                    darkSurfaceStyle = DarkSurfaceStyle.Neutral
                )
            ) {
                SettingsApplicationPreferencesCard(
                    appLanguage = AppLanguage.English,
                    onSetAppLanguage = {},
                    timeDisplayMode = TimeDisplayMode.TWENTY_FOUR_HOUR,
                    onTimeDisplayModeChange = {},
                    promptBeforeUnconfiguredActivityRecord = false,
                    onPromptBeforeUnconfiguredActivityRecordChange = {},
                    pageTransitionStyle = pageTransitionStyle.value,
                    onPageTransitionStyleChange = { style ->
                        pageTransitionStyle.value = style
                        onPageTransitionStyleChange(style)
                    },
                    pageTransitionOptionsExpanded = pageTransitionOptionsExpanded.value,
                    onPageTransitionOptionsExpandedChange = { value ->
                        pageTransitionOptionsExpanded.value = value
                    }
                )
            }
        }
    }
}
