package com.example.tracer

import androidx.activity.ComponentActivity
import androidx.compose.material3.MaterialTheme
import androidx.compose.ui.test.assertCountEquals
import androidx.compose.ui.test.assertIsDisplayed
import androidx.compose.ui.test.junit4.createAndroidComposeRule
import androidx.compose.ui.test.onAllNodesWithTag
import androidx.compose.ui.test.onAllNodesWithText
import androidx.compose.ui.test.onNodeWithText
import androidx.test.ext.junit.runners.AndroidJUnit4
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import java.time.LocalDate

@RunWith(AndroidJUnit4::class)
class TxtEditorContentCardTest {
    @get:Rule
    val composeRule = createAndroidComposeRule<ComponentActivity>()

    @Test
    fun targetDayInput_isNotShownInEditor() {
        composeRule.setContent {
            MaterialTheme {
                TxtEditorContentCard(
                    selectedHistoryFile = "2026/2026-04.txt",
                    currentDay = LocalDate.of(2026, 4, 1),
                    outputMode = TxtOutputMode.DAY,
                    onOutputModeChange = {},
                    activityNameTargetMode = TxtActivityNameTargetMode.CANONICAL,
                    onActivityNameTargetModeChange = {},
                    dayBlockEditorState = TxtDayBlockResolveResult(
                        ok = true,
                        normalizedDayMarker = "0401",
                        found = true,
                        isMarkerValid = true,
                        canSave = true,
                        dayBody = "",
                        dayContentIsoDate = null,
                        message = "ok"
                    ),
                    dayMarkerInput = "0401",
                    inlineStatusText = "",
                    editorText = "",
                    hasUnsavedChanges = false,
                    canEditDay = true,
                    canIngest = false,
                    onEditorTextChange = {},
                    onIngest = {}
                )
            }
        }

        composeRule.onAllNodesWithTag("txt_target_day_dd").assertCountEquals(0)
    }

    @Test
    fun structuredDay_exposesDayAndActivityRemarkEditing() {
        composeRule.setContent {
            MaterialTheme {
                TxtEditorContentCard(
                    selectedHistoryFile = "2026/2026-04.txt",
                    currentDay = LocalDate.of(2026, 4, 1),
                    outputMode = TxtOutputMode.DAY,
                    onOutputModeChange = {},
                    activityNameTargetMode = TxtActivityNameTargetMode.CANONICAL,
                    onActivityNameTargetModeChange = {},
                    dayBlockEditorState = TxtDayBlockResolveResult(
                        ok = true,
                        normalizedDayMarker = "0401",
                        found = true,
                        isMarkerValid = true,
                        canSave = true,
                        dayBody = "0900study // activity note\n",
                        dayContentIsoDate = "2026-04-01",
                        message = "ok"
                    ),
                    dayMarkerInput = "0401",
                    inlineStatusText = "",
                    editorText = "",
                    hasUnsavedChanges = false,
                    canEditDay = true,
                    canIngest = false,
                    onEditorTextChange = {},
                    onIngest = {},
                    structuredDayEdit = TxtDayEditResolveResult(
                        ok = true,
                        normalizedDayMarker = "0401",
                        found = true,
                        isMarkerValid = true,
                        canSave = true,
                        dayRemark = "day note",
                        events = listOf(
                            TxtDayEditEvent(
                                isInterval = false,
                                startTime = "",
                                endTime = "09:00:00",
                                activityToken = "study",
                                remark = "activity note"
                            )
                        ),
                        dayContentIsoDate = "2026-04-01",
                        message = "ok"
                    )
                )
            }
        }

        composeRule.onNodeWithText("Day remark").assertIsDisplayed()
        composeRule.onAllNodesWithText("Edit remark").assertCountEquals(2)
    }
}
