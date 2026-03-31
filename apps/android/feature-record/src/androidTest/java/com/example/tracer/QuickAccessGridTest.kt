package com.example.tracer

import androidx.activity.ComponentActivity
import androidx.compose.foundation.layout.width
import androidx.compose.material3.MaterialTheme
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.test.assertIsDisplayed
import androidx.compose.ui.test.junit4.createAndroidComposeRule
import androidx.compose.ui.test.onNodeWithTag
import androidx.compose.ui.test.performClick
import androidx.compose.ui.test.performTouchInput
import androidx.compose.ui.unit.dp
import androidx.test.ext.junit.runners.AndroidJUnit4
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class QuickAccessGridTest {
    @get:Rule
    val composeRule = createAndroidComposeRule<ComponentActivity>()

    @Test
    fun quickAccessGrid_clickUpdatesRecordContent_withoutReordering() {
        var recordedContent = ""
        var reorderCalls = 0

        composeRule.setContent {
            MaterialTheme {
                RecordQuickAccessCard(
                    recordContent = recordedContent,
                    onRecordContentChange = { recordedContent = it },
                    quickActivities = listOf("meal", "wash", "sleep"),
                    availableActivityNames = emptyList(),
                    onQuickActivitiesUpdate = {
                        reorderCalls += 1
                        true
                    },
                    assistSettingsExpanded = false,
                    onToggleAssistSettings = {},
                    suggestionLookbackDays = 7,
                    suggestionTopN = 5,
                    onSuggestionLookbackDaysChange = {},
                    onSuggestionTopNChange = {},
                    quickActivitySearch = "",
                    onQuickActivitySearchChange = {},
                    maxQuickActivityCount = 12
                )
            }
        }

        composeRule.onNodeWithTag(quickAccessItemTestTag("wash")).performClick()
        composeRule.waitForIdle()

        assertEquals("wash", recordedContent)
        assertEquals(0, reorderCalls)
    }

    @Test
    fun quickAccessGrid_longPressDragReorders_andCommitsOnce() {
        val reorderCalls = mutableListOf<List<String>>()

        composeRule.setContent {
            var quickActivities by remember {
                mutableStateOf(listOf("bilibili", "zhihu", "weibo"))
            }
            MaterialTheme {
                RecordQuickAccessCard(
                    recordContent = "",
                    onRecordContentChange = {},
                    quickActivities = quickActivities,
                    availableActivityNames = emptyList(),
                    onQuickActivitiesUpdate = { updated ->
                        reorderCalls += updated
                        quickActivities = updated
                        true
                    },
                    assistSettingsExpanded = false,
                    onToggleAssistSettings = {},
                    suggestionLookbackDays = 7,
                    suggestionTopN = 5,
                    onSuggestionLookbackDaysChange = {},
                    onSuggestionTopNChange = {},
                    quickActivitySearch = "",
                    onQuickActivitySearchChange = {},
                    maxQuickActivityCount = 12
                )
            }
        }

        val draggedNode = composeRule.onNodeWithTag(quickAccessItemTestTag("bilibili"))
        val targetBounds = composeRule.onNodeWithTag(quickAccessItemTestTag("weibo"))
            .fetchSemanticsNode()
            .boundsInRoot

        draggedNode.performTouchInput {
            down(center)
            advanceEventTime(900L)
            moveTo(targetBounds.center)
            up()
        }
        composeRule.waitForIdle()

        assertEquals(1, reorderCalls.size)
        assertEquals(listOf("zhihu", "weibo", "bilibili"), reorderCalls.single())
    }

    @Test
    fun quickAccessGrid_deleteModeDoesNotStartDragSorting() {
        val reorderCalls = mutableListOf<List<String>>()

        composeRule.setContent {
            var quickActivities by remember {
                mutableStateOf(listOf("meal", "wash", "sleep"))
            }
            MaterialTheme {
                RecordQuickAccessCard(
                    recordContent = "",
                    onRecordContentChange = {},
                    quickActivities = quickActivities,
                    availableActivityNames = emptyList(),
                    onQuickActivitiesUpdate = { updated ->
                        reorderCalls += updated
                        quickActivities = updated
                        true
                    },
                    assistSettingsExpanded = true,
                    onToggleAssistSettings = {},
                    suggestionLookbackDays = 7,
                    suggestionTopN = 5,
                    onSuggestionLookbackDaysChange = {},
                    onSuggestionTopNChange = {},
                    quickActivitySearch = "",
                    onQuickActivitySearchChange = {},
                    maxQuickActivityCount = 12
                )
            }
        }

        val draggedNode = composeRule.onNodeWithTag(quickAccessItemTestTag("meal"))
        val targetBounds = composeRule.onNodeWithTag(quickAccessItemTestTag("sleep"))
            .fetchSemanticsNode()
            .boundsInRoot

        draggedNode.performTouchInput {
            down(center)
            advanceEventTime(900L)
            moveTo(targetBounds.center)
            up()
        }
        composeRule.waitForIdle()

        assertTrue(reorderCalls.isEmpty())
    }

    @Test
    fun quickAccessGrid_usesFlowContainerTag() {
        composeRule.setContent {
            MaterialTheme {
                QuickAccessActivityGrid(
                    modifier = Modifier.width(320.dp),
                    quickActivities = listOf("bilibili", "zhihu", "weibo"),
                    recordContent = "",
                    isDeleteMode = false,
                    onRecordContentChange = {},
                    onQuickActivitiesUpdate = { true }
                )
            }
        }

        composeRule.onNodeWithTag(quickAccessGridTestTag()).assertIsDisplayed()
    }

    @Test
    fun quickAccessGrid_usesVariableChipWidths() {
        composeRule.setContent {
            MaterialTheme {
                QuickAccessActivityGrid(
                    modifier = Modifier.width(320.dp),
                    quickActivities = listOf("bilibili", "zhihu", "weibo"),
                    recordContent = "",
                    isDeleteMode = false,
                    onRecordContentChange = {},
                    onQuickActivitiesUpdate = { true }
                )
            }
        }

        composeRule.waitForIdle()

        val longWidth = composeRule.onNodeWithTag(quickAccessItemTestTag("bilibili"))
            .fetchSemanticsNode()
            .boundsInRoot
            .width
        val shortWidth = composeRule.onNodeWithTag(quickAccessItemTestTag("zhihu"))
            .fetchSemanticsNode()
            .boundsInRoot
            .width

        assertTrue(longWidth > shortWidth)
    }

    @Test
    fun quickAccessGrid_clampsLongChipWidthToConfiguredMaximum() {
        val density = composeRule.activity.resources.displayMetrics.density

        composeRule.setContent {
            MaterialTheme {
                QuickAccessActivityGrid(
                    modifier = Modifier.width(400.dp),
                    quickActivities = listOf("this_is_a_very_long_activity_name_that_should_be_truncated"),
                    recordContent = "",
                    isDeleteMode = false,
                    onRecordContentChange = {},
                    onQuickActivitiesUpdate = { true }
                )
            }
        }

        composeRule.waitForIdle()

        val longWidth = composeRule.onNodeWithTag(
            quickAccessItemTestTag("this_is_a_very_long_activity_name_that_should_be_truncated")
        )
            .fetchSemanticsNode()
            .boundsInRoot
            .width

        assertTrue(longWidth <= (240f * density) + 1f)
    }
}
