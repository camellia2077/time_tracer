package com.example.tracer

import androidx.compose.material3.MaterialTheme
import androidx.compose.ui.test.assertIsDisplayed
import androidx.compose.ui.test.junit4.createComposeRule
import androidx.compose.ui.test.onNodeWithText
import com.example.tracer.ui.components.FullscreenTextEditor
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class FullscreenTextEditorRobolectricTest {
    @get:Rule
    val composeRule = createComposeRule()

    @Test
    fun rendersAsAnUnboundedEditorPage() {
        composeRule.setContent {
            MaterialTheme {
                FullscreenTextEditor(
                    title = "Edit day remark",
                    label = "Day remark",
                    text = "Existing note",
                    closeContentDescription = "Close",
                    saving = false,
                    error = "",
                    onTextChange = {},
                    onDismiss = {}
                )
            }
        }

        composeRule.onNodeWithText("Edit day remark").assertIsDisplayed()
        composeRule.onNodeWithText("Day remark").assertIsDisplayed()
        composeRule.onNodeWithText("Existing note").assertIsDisplayed()
    }
}
