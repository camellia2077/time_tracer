package com.example.tracer

import androidx.activity.ComponentActivity
import androidx.compose.material3.MaterialTheme
import androidx.compose.ui.test.assertTextEquals
import androidx.compose.ui.test.junit4.createAndroidComposeRule
import androidx.compose.ui.test.onNodeWithTag
import androidx.compose.ui.test.performTextClearance
import androidx.compose.ui.test.performTextInput
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
    fun targetDayDayField_appendsTypedDigitsAfterClearing() {
        var dayMarkerInput = "0401"

        composeRule.setContent {
            MaterialTheme {
                TxtEditorContentCard(
                    selectedHistoryFile = "2026/2026-04.txt",
                    selectedMonth = "2026-04",
                    currentDay = LocalDate.of(2026, 4, 1),
                    outputMode = TxtOutputMode.DAY,
                    onOutputModeChange = {},
                    dayBlockEditorState = TxtDayBlockResolveResult(
                        ok = true,
                        normalizedDayMarker = dayMarkerInput,
                        found = true,
                        isMarkerValid = true,
                        canSave = true,
                        dayBody = "",
                        dayContentIsoDate = null,
                        message = "ok"
                    ),
                    dayMarkerInput = dayMarkerInput,
                    onDayMarkerInputChange = { dayMarkerInput = it },
                    onOpenDay = {},
                    inlineStatusText = "",
                    isEditorContentVisible = false,
                    onToggleEditorContentVisibility = {},
                    editorText = "",
                    hasUnsavedChanges = false,
                    canEditDay = true,
                    canIngest = false,
                    onEditorTextChange = {},
                    onIngest = {}
                )
            }
        }

        composeRule
            .onNodeWithTag(targetDayDayFieldTestTag())
            .performTextClearance()
        composeRule
            .onNodeWithTag(targetDayDayFieldTestTag())
            .performTextInput("1")
        composeRule
            .onNodeWithTag(targetDayDayFieldTestTag())
            .performTextInput("5")
        composeRule.waitForIdle()

        composeRule
            .onNodeWithTag(targetDayDayFieldTestTag())
            .assertTextEquals("15")
    }
}
