package com.example.tracer

import androidx.compose.material3.SnackbarHostState
import androidx.compose.ui.semantics.SemanticsActions
import androidx.compose.ui.text.TextLayoutResult
import androidx.compose.ui.test.junit4.createComposeRule
import androidx.compose.ui.test.onNodeWithTag
import com.example.tracer.data.DarkSurfaceStyle
import com.example.tracer.data.ThemeConfig
import com.example.tracer.data.ThemeMode
import com.example.tracer.ui.theme.TracerTheme
import org.junit.Assert.assertEquals
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class TracerBottomNavigationRobolectricTest {
    @get:Rule
    val composeRule = createComposeRule()

    @Test
    fun selectedInsightsLabel_remainsSingleLine() {
        composeRule.setContent {
            TracerTheme(
                themeConfig = ThemeConfig(
                    themeMode = ThemeMode.Light,
                    darkSurfaceStyle = DarkSurfaceStyle.Neutral
                )
            ) {
                TracerBottomNavShell(
                    selectedTab = TracerTab.INSIGHTS,
                    onTabSelected = {},
                    snackbarHostState = SnackbarHostState()
                ) {}
            }
        }

        val textLayoutResults = mutableListOf<TextLayoutResult>()
        composeRule
            .onNodeWithTag("tab_insights_label", useUnmergedTree = true)
            .fetchSemanticsNode()
            .config[SemanticsActions.GetTextLayoutResult]
            .action
            ?.invoke(textLayoutResults)

        assertEquals(1, textLayoutResults.single().lineCount)
    }
}
