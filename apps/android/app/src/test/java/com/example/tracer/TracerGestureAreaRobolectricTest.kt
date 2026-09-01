package com.example.tracer

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.SnackbarHostState
import androidx.compose.runtime.mutableStateOf
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.semantics.SemanticsActions
import androidx.compose.ui.test.hasScrollAction
import androidx.compose.ui.test.junit4.createComposeRule
import androidx.compose.ui.test.onNodeWithTag
import androidx.compose.ui.test.performSemanticsAction
import androidx.compose.ui.unit.dp
import com.example.tracer.data.ThemeConfig
import com.example.tracer.data.ThemeMode
import com.example.tracer.data.ThemePalette
import com.example.tracer.ui.theme.TracerTheme
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class TracerGestureAreaRobolectricTest {
    @get:Rule
    val composeRule = createComposeRule()

    private val theme = mutableStateOf(ThemeConfig(ThemeMode.Light))
    private val tab = mutableStateOf(TracerTab.SETTINGS)

    @Test
    fun contentReachesWindowBottom_whenSwitchingEveryPaletteBetweenLightAndDark() {
        setContent()
        ThemePalette.entries.forEach { palette ->
            listOf(ThemeMode.Light, ThemeMode.Dark, ThemeMode.Light).forEach { mode ->
                composeRule.runOnIdle { theme.value = ThemeConfig(mode, palette = palette) }
                assertContentReachesWindowBottom("$palette / $mode")
            }
        }
    }

    @Test
    fun everyTab_keepsTheScrollViewportOpenToTheWindowBottom() {
        setContent()
        TracerTabRegistry.entries.forEach { entry ->
            composeRule.runOnIdle { tab.value = entry.meta.id }
            assertContentReachesWindowBottom(entry.meta.id.toString())
        }
    }

    @Test
    fun settingsFinalContent_canStillScrollAboveFloatingNavigation() {
        setContent()
        composeRule.onNode(hasScrollAction()).performSemanticsAction(SemanticsActions.ScrollBy) {
            it(0f, 100000f)
        }
        val root = composeRule.onNodeWithTag("window").fetchSemanticsNode().boundsInRoot
        val content = composeRule.onNodeWithTag("content").fetchSemanticsNode().boundsInRoot
        val minimumClearance = with(composeRule.density) { 80.dp.toPx() }
        assertTrue(root.bottom - content.bottom >= minimumClearance - 1f)
    }

    private fun setContent() {
        composeRule.setContent {
            TracerTheme(themeConfig = theme.value) {
                Box(
                    Modifier.size(400.dp)
                        .background(MaterialTheme.colorScheme.background)
                        .testTag("window")
                ) {
                    TracerBottomNavShell(tab.value, {}, SnackbarHostState()) { innerPadding ->
                        val routeModifier = Modifier.tracerTabContentModifier(tab.value, innerPadding)
                        // Insights owns its scrolling; the remaining tabs use the route's scroll.
                        val contentModifier = if (
                            TracerTabRegistry.entry(tab.value).scrollBehavior == TracerTabScrollBehavior.NONE
                        ) {
                            routeModifier.verticalScroll(rememberScrollState())
                        } else {
                            routeModifier
                        }
                        Column(contentModifier) {
                            Box(
                                Modifier.fillMaxWidth().height(1200.dp)
                                    .background(Color.Magenta).testTag("content")
                            )
                        }
                    }
                }
            }
        }
    }

    private fun assertContentReachesWindowBottom(message: String) {
        val root = composeRule.onNodeWithTag("window").fetchSemanticsNode().boundsInRoot
        val content = composeRule.onNodeWithTag("content").fetchSemanticsNode().boundsInRoot
        assertEquals(message, root.bottom, content.bottom, 1f)
    }
}
