package com.example.tracer

import androidx.activity.ComponentActivity
import androidx.compose.material3.MaterialTheme
import androidx.compose.ui.test.assertIsDisplayed
import androidx.compose.ui.test.junit4.createAndroidComposeRule
import androidx.compose.ui.test.onNodeWithText
import androidx.compose.ui.test.onNodeWithTag
import androidx.compose.ui.test.performTouchInput
import androidx.compose.ui.test.performClick
import androidx.test.ext.junit.runners.AndroidJUnit4
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class RecordCanonicalCatalogDragTest {
    @get:Rule
    val composeRule = createAndroidComposeRule<ComponentActivity>()

    @Test
    fun canonicalCatalogRootDrag_previewsDropBeforeRelease_andCommitsOnRelease() {
        val reorderCalls = mutableListOf<List<String>>()
        val roots = listOf(
            canonicalRoot("alpha"),
            canonicalRoot("beta"),
            canonicalRoot("gamma")
        )

        composeRule.setContent {
            MaterialTheme {
                RecordCanonicalCatalogScreen(
                    isLoading = false,
                    roots = roots,
                    statusText = "",
                    displayMode = RecordFrequentOutputMode.CANONICAL,
                    target = CanonicalBrowserTarget.QUICK_ACCESS,
                    collapsedRootPaths = emptySet(),
                    orderedRootPaths = emptyList(),
                    onDismissRequest = {},
                    onDisplayModeChange = {},
                    onCollapsedRootPathsChange = {},
                    onOrderedRootPathsChange = { reorderCalls += it },
                    onCanonicalEntryClick = {}
                )
            }
        }
        composeRule.waitForIdle()
        composeRule.onNodeWithText("Add Quick Access Activity").assertIsDisplayed()

        val draggedRoot = composeRule.onNodeWithTag(canonicalCatalogRootTestTag("alpha"))
        val targetCenter = composeRule.onNodeWithTag(canonicalCatalogRootTestTag("beta"))
            .fetchSemanticsNode()
            .boundsInRoot
            .center

        draggedRoot.performTouchInput {
            down(center)
            advanceEventTime(900L)
            moveTo(targetCenter)
        }
        composeRule.waitForIdle()

        assertTrue(reorderCalls.isEmpty())
        composeRule.onNodeWithTag(canonicalCatalogRootDropPreviewTestTag()).assertIsDisplayed()

        draggedRoot.performTouchInput {
            up()
        }
        composeRule.waitForIdle()

        assertEquals(listOf(listOf("beta", "alpha", "gamma")), reorderCalls)
    }

    @Test
    fun quickAccessCatalog_frequentTabLoadsAndDisplaysFrequentActivities() {
        var frequentRequested = 0

        composeRule.setContent {
            MaterialTheme {
                RecordCanonicalCatalogScreen(
                    isLoading = false,
                    roots = emptyList(),
                    statusText = "",
                    displayMode = RecordFrequentOutputMode.CANONICAL,
                    target = CanonicalBrowserTarget.RECORD_INPUT,
                    frequentActivities = listOf(RecordFrequentActivity(canonicalToken = "reading")),
                    collapsedRootPaths = emptySet(),
                    orderedRootPaths = emptyList(),
                    onDismissRequest = {},
                    onDisplayModeChange = {},
                    onCollapsedRootPathsChange = {},
                    onOrderedRootPathsChange = {},
                    onCanonicalEntryClick = {},
                    onFrequentActivitiesRequested = { frequentRequested += 1 }
                )
            }
        }

        composeRule.onNodeWithText("Activities").assertIsDisplayed()
        composeRule.onNodeWithText("Frequent").performClick()

        assertEquals(1, frequentRequested)
        composeRule.onNodeWithText("reading").assertIsDisplayed()
    }
}

private fun canonicalRoot(path: String): CanonicalPathNode =
    CanonicalPathNode(name = path, path = path)
