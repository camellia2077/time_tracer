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
import java.time.Clock
import java.time.Instant
import java.time.ZoneId

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
                    authoringMode = RecordAuthoringMode.POINT,
                    onAuthoringModeChange = {},
                    recordContent = "",
                    onRecordContentChange = {},
                    recordRemark = "",
                    onRecordRemarkChange = {},
                    intervalStart = "",
                    onIntervalStartChange = {},
                    intervalEnd = "",
                    onIntervalEndChange = {},
                    lastRecordedActivityAlias = "",
                    lastRecordedDuration = "",
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
    fun recordInputCard_lastRecordedSummaryUsesSeparateLines() {
        composeRule.setContent {
            MaterialTheme {
                RecordInputCard(
                    authoringMode = RecordAuthoringMode.POINT,
                    onAuthoringModeChange = {},
                    recordContent = "",
                    onRecordContentChange = {},
                    recordRemark = "",
                    onRecordRemarkChange = {},
                    intervalStart = "",
                    onIntervalStartChange = {},
                    intervalEnd = "",
                    onIntervalEndChange = {},
                    lastRecordedActivityAlias = "coding",
                    lastRecordedDuration = "00:25",
                    suggestionsVisible = false,
                    onToggleSuggestions = {},
                    onOpenTxtPreview = {},
                    onRecordNow = {}
                )
            }
        }

        composeRule.onNodeWithText("Last:\ncoding\n00:25").assertIsDisplayed()
    }

    @Test
    fun recordTxtPreviewSheet_showsResolvedDayBlockBody() {
        composeRule.setContent {
            MaterialTheme {
                RecordTxtPreviewSheet(
                    txtStorageGateway = PreviewTestTxtStorageGateway(),
                    selectedMonth = "2026-04",
                    selectedHistoryFile = "2026/2026-04.txt",
                    editableHistoryContent = "y2026\nm04\nd0416\n  coding\n",
                    logicalDayTarget = RecordLogicalDayTarget.TODAY,
                    logicalDayClock = fixedClock("2026-04-16T12:00:00Z", "Asia/Shanghai"),
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

private fun fixedClock(instantIso: String, zoneId: String): Clock =
    Clock.fixed(Instant.parse(instantIso), ZoneId.of(zoneId))
