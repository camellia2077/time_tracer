package com.example.tracer

import androidx.activity.ComponentActivity
import androidx.compose.material3.MaterialTheme
import androidx.compose.ui.test.assertIsDisplayed
import androidx.compose.ui.test.junit4.createAndroidComposeRule
import androidx.compose.ui.test.onNodeWithTag
import androidx.compose.ui.test.onNodeWithText
import androidx.compose.ui.test.performClick
import androidx.test.ext.junit.runners.AndroidJUnit4
import org.junit.Assert.assertEquals
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class RecordTxtPreviewTest {
    @get:Rule
    val composeRule = createAndroidComposeRule<ComponentActivity>()

    @Test
    fun recordInputCard_previewButtonInvokesCallback() {
        var clickCount = 0

        composeRule.setContent {
            MaterialTheme {
                RecordInputCard(
                    recordContent = "",
                    onRecordContentChange = {},
                    recordRemark = "",
                    onRecordRemarkChange = {},
                    suggestionsVisible = false,
                    onToggleSuggestions = {},
                    onOpenTxtPreview = { clickCount += 1 },
                    onRecordNow = {}
                )
            }
        }

        composeRule.onNodeWithTag(recordTxtPreviewButtonTestTag()).performClick()
        composeRule.waitForIdle()

        assertEquals(1, clickCount)
    }

    @Test
    fun recordTxtPreviewSheet_showsResolvedDayBlockBody() {
        composeRule.setContent {
            MaterialTheme {
                RecordTxtPreviewSheet(
                    txtStorageGateway = PreviewTestTxtStorageGateway(),
                    selectedMonth = "2026-04",
                    selectedHistoryFile = "2026/2026-04.txt",
                    editableHistoryContent = "y2026\nm04\n0416\n  coding\n",
                    logicalDayTarget = RecordLogicalDayTarget.TODAY,
                    isLoading = false,
                    previewStatusText = "",
                    onDismissRequest = {}
                )
            }
        }

        composeRule.onNodeWithTag(recordTxtPreviewSheetTestTag()).assertIsDisplayed()
        composeRule.onNodeWithTag(recordTxtPreviewContentTestTag()).assertIsDisplayed()
        composeRule.onNodeWithText("coding").assertIsDisplayed()
    }
}

private class PreviewTestTxtStorageGateway : TxtStorageGateway {
    override suspend fun inspectTxtFiles(): TxtInspectionResult =
        TxtInspectionResult(ok = true, entries = emptyList(), message = "ok")

    override suspend fun listTxtFiles(): TxtHistoryListResult =
        TxtHistoryListResult(ok = true, files = emptyList(), message = "ok")

    override suspend fun readTxtFile(relativePath: String): TxtFileContentResult =
        TxtFileContentResult(ok = true, filePath = relativePath, content = "", message = "ok")

    override suspend fun saveTxtFileAndSync(relativePath: String, content: String): RecordActionResult =
        RecordActionResult(ok = true, message = "ok")

    override suspend fun defaultTxtDayMarker(
        selectedMonth: String,
        targetDateIso: String
    ): TxtDayMarkerResult = TxtDayMarkerResult(
        ok = true,
        normalizedDayMarker = "0416",
        message = "ok"
    )

    override suspend fun resolveTxtDayBlock(
        content: String,
        dayMarker: String,
        selectedMonth: String
    ): TxtDayBlockResolveResult = TxtDayBlockResolveResult(
        ok = true,
        normalizedDayMarker = "0416",
        found = true,
        isMarkerValid = true,
        canSave = false,
        dayBody = "coding",
        dayContentIsoDate = "2026-04-16",
        message = "ok"
    )
}
