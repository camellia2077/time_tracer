package com.example.tracer

import android.view.View
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.navigationBarsPadding
import androidx.compose.foundation.ScrollState
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalView
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.test.junit4.createComposeRule
import androidx.compose.ui.test.onNodeWithTag
import androidx.compose.ui.test.performScrollTo
import androidx.compose.ui.unit.dp
import androidx.core.graphics.Insets
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import org.junit.Assert.assertEquals
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner
import org.robolectric.annotation.Config
import kotlinx.coroutines.runBlocking

@RunWith(RobolectricTestRunner::class)
@Config(sdk = [35])
class FullscreenPageInsetsRobolectricTest {
    @get:Rule
    val composeRule = createComposeRule()

    private lateinit var view: View
    private lateinit var scrollState: ScrollState

    @Test
    fun scrollingPageReachesWindowBottomWithSafeSpaceOnlyAtEndOfList() {
        renderPage(hosted = true, scrollInsets = true)
        dispatchInsets(navigationBottom = 24, gestureBottom = 48)
        assertContentBottomInset(0)
        assertLastItemBottomInset(48)
        dispatchInsets(navigationBottom = 72, gestureBottom = 24)
        assertContentBottomInset(0)
        assertLastItemBottomInset(72)
    }

    @Test
    fun nestedScrollingPageKeepsFullHeightAndItsOwnTrailingInset() {
        renderPage(hosted = true, nested = true, scrollInsets = true)
        dispatchInsets(navigationBottom = 24, gestureBottom = 48)
        assertContentBottomInset(0)
        assertLastItemBottomInset(48)
    }

    @Test
    fun scrollingPageStillKeepsItsViewportAboveTheKeyboard() {
        renderPage(hosted = true, scrollInsets = true)
        dispatchInsets(navigationBottom = 24, gestureBottom = 48, imeBottom = 300)
        assertContentBottomInset(300)
        assertLastItemBottomInset(300)
    }

    @Test
    fun hostedPageReservesLargerGestureRegionAndCanScrollToLastItem() {
        renderPage(hosted = true)
        dispatchInsets(navigationBottom = 24, gestureBottom = 48)
        assertContentBottomInset(48)
        composeRule.onNodeWithTag("last").performScrollTo()
        val viewport = composeRule.onNodeWithTag("content").fetchSemanticsNode().boundsInRoot
        val last = composeRule.onNodeWithTag("last").fetchSemanticsNode().boundsInRoot
        assertEquals(viewport.bottom, last.bottom, 1f)
    }

    @Test
    fun fallbackPageReservesNavigationBarAndUpdatesWhenInsetsChange() {
        renderPage(hosted = false)
        dispatchInsets(navigationBottom = 72, gestureBottom = 24)
        assertContentBottomInset(72)
        dispatchInsets(navigationBottom = 24, gestureBottom = 48)
        assertContentBottomInset(48)
        dispatchInsets(navigationBottom = 0, gestureBottom = 0)
        assertContentBottomInset(0)
    }

    @Test
    fun nestedPageDoesNotLoseOrDoubleConsumeBottomInset() {
        renderPage(hosted = true, nested = true)
        dispatchInsets(navigationBottom = 24, gestureBottom = 48)
        assertContentBottomInset(48)
    }

    private fun renderPage(hosted: Boolean, nested: Boolean = false, scrollInsets: Boolean = false) {
        composeRule.setContent {
            view = LocalView.current
            MaterialTheme {
                Box(Modifier.fillMaxSize().testTag("root")) {
                    val page: @Composable () -> Unit = {
                        FullscreenPage(onDismissRequest = {}, scrollContentHandlesBottomInset = scrollInsets) {
                            if (nested) {
                                FullscreenPage(onDismissRequest = {}, scrollContentHandlesBottomInset = scrollInsets) {
                                    PageContent(scrollInsets)
                                }
                            } else {
                                PageContent(scrollInsets)
                            }
                        }
                    }
                    if (hosted) FullscreenPageHost { page() } else page()
                }
            }
        }
    }

    @Composable
    private fun PageContent(scrollInsets: Boolean) {
        scrollState = rememberScrollState()
        // Child inset handling must not add the already reserved navigation bar again.
        Column(
            Modifier.fillMaxSize()
                .then(if (scrollInsets) Modifier else Modifier.navigationBarsPadding())
                .testTag("content")
                .verticalScroll(scrollState)
                .then(if (scrollInsets) Modifier.fullscreenScrollContentPadding() else Modifier)
        ) {
            repeat(30) { Text("Item $it", Modifier.height(48.dp)) }
            Text("Last activity", Modifier.height(48.dp).testTag("last"))
        }
    }

    private fun dispatchInsets(navigationBottom: Int, gestureBottom: Int, imeBottom: Int = 0) {
        composeRule.runOnIdle {
            ViewCompat.dispatchApplyWindowInsets(
                view,
                WindowInsetsCompat.Builder()
                    .setInsets(WindowInsetsCompat.Type.ime(), Insets.of(0, 0, 0, imeBottom))
                    .setInsets(WindowInsetsCompat.Type.statusBars(), Insets.of(0, 32, 0, 0))
                    .setInsets(WindowInsetsCompat.Type.navigationBars(), Insets.of(0, 0, 0, navigationBottom))
                    .setInsets(WindowInsetsCompat.Type.mandatorySystemGestures(), Insets.of(0, 0, 0, gestureBottom))
                    .build()
            )
        }
        composeRule.waitForIdle()
    }

    private fun assertContentBottomInset(expected: Int) {
        val root = composeRule.onNodeWithTag("root").fetchSemanticsNode().boundsInRoot
        val content = composeRule.onNodeWithTag("content").fetchSemanticsNode().boundsInRoot
        assertEquals(expected.toFloat(), root.bottom - content.bottom, 1f)
        assertEquals(32f, content.top - root.top, 1f)
    }

    private fun assertLastItemBottomInset(expected: Int) {
        composeRule.runOnIdle { runBlocking { scrollState.scrollTo(scrollState.maxValue) } }
        val root = composeRule.onNodeWithTag("root").fetchSemanticsNode().boundsInRoot
        val last = composeRule.onNodeWithTag("last").fetchSemanticsNode().boundsInRoot
        assertEquals(expected.toFloat(), root.bottom - last.bottom, 1f)
    }
}
