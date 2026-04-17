package com.example.tracer

import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class TxtEditorRuntimeCoordinatorTest {
    @Test
    fun syncAutoDayMarkerIfNeeded_loadsAndAppliesMarker() = runBlocking {
        val gateway = FakeTxtEditorRuntimeGateway(
            defaultMarkerResult = TxtDayMarkerResult(
                ok = true,
                normalizedDayMarker = "0417",
                message = "ok"
            )
        )
        val coordinator = TxtEditorRuntimeCoordinator(gateway)
        val controller = TxtEditorSessionController()

        coordinator.syncAutoDayMarkerIfNeeded(
            sessionController = controller,
            selectedHistoryFile = "2026/2026-04.txt",
            selectedMonth = "2026-04",
            logicalDayTarget = RecordLogicalDayTarget.TODAY
        )

        assertEquals("0417", controller.state.dayMarkerInput)
        assertEquals("2026-04", gateway.lastDefaultMarkerMonth)
        assertEquals("2026-04-17", gateway.lastDefaultMarkerTargetDateIso)
    }

    @Test
    fun ingestCurrentEditor_dayMode_mergesAndResetsDirtyState() = runBlocking {
        val gateway = FakeTxtEditorRuntimeGateway(
            replaceResult = TxtDayBlockReplaceResult(
                ok = true,
                normalizedDayMarker = "0417",
                found = true,
                isMarkerValid = true,
                updatedContent = "merged-content",
                message = "ok"
            )
        )
        val coordinator = TxtEditorRuntimeCoordinator(gateway)
        val controller = TxtEditorSessionController()
        var mergedMonthContent = ""
        var saveCalled = false

        controller.updateOutputMode(TxtOutputMode.DAY)
        controller.syncExternalMonthDraft(
            selectedHistoryContent = "saved-month",
            editableHistoryContent = "month-content"
        )
        controller.syncResolvedDayBody("0900study\n")
        controller.onEditorTextChange("0900study\n1000break\n")

        val ingested = coordinator.ingestCurrentEditor(
            sessionController = controller,
            canEditDay = true,
            dayMarker = "0417",
            onMergedMonthContent = { mergedMonthContent = it },
            onSaveHistoryFile = { saveCalled = true }
        )

        assertTrue(ingested)
        assertEquals("month-content", gateway.lastReplaceContent)
        assertEquals("0417", gateway.lastReplaceDayMarker)
        assertEquals("0900study\n1000break\n", gateway.lastReplaceDayBody)
        assertEquals("merged-content", mergedMonthContent)
        assertTrue(saveCalled)
        assertEquals("merged-content", controller.state.allDraftState.baselineText)
        assertEquals("merged-content", controller.state.allDraftState.draftText)
        assertFalse(controller.state.dayDraftState.hasUnsavedChanges)
        assertFalse(controller.state.isEditorContentVisible)
    }

    @Test
    fun ingestCurrentEditor_allMode_savesWithoutDayMerge() = runBlocking {
        val gateway = FakeTxtEditorRuntimeGateway()
        val coordinator = TxtEditorRuntimeCoordinator(gateway)
        val controller = TxtEditorSessionController()
        var saveCalled = false

        controller.updateOutputMode(TxtOutputMode.ALL)
        controller.syncExternalMonthDraft(
            selectedHistoryContent = "saved",
            editableHistoryContent = "saved"
        )
        controller.openEditor(resolvedDayBody = "")
        controller.onEditorTextChange("draft")

        val ingested = coordinator.ingestCurrentEditor(
            sessionController = controller,
            canEditDay = false,
            dayMarker = "0417",
            onMergedMonthContent = {},
            onSaveHistoryFile = { saveCalled = true }
        )

        assertTrue(ingested)
        assertTrue(saveCalled)
        assertEquals("", gateway.lastReplaceContent)
        assertEquals("draft", controller.state.allDraftState.baselineText)
        assertEquals("draft", controller.state.allDraftState.draftText)
        assertFalse(controller.state.isEditorContentVisible)
    }
}

private class FakeTxtEditorRuntimeGateway(
    private val defaultMarkerResult: TxtDayMarkerResult = TxtDayMarkerResult(
        ok = true,
        normalizedDayMarker = "",
        message = "ok"
    ),
    private val replaceResult: TxtDayBlockReplaceResult = TxtDayBlockReplaceResult(
        ok = true,
        normalizedDayMarker = "",
        found = false,
        isMarkerValid = false,
        updatedContent = "",
        message = "ok"
    ),
    private val resolveResult: TxtDayBlockResolveResult = TxtDayBlockResolveResult(
        ok = true,
        normalizedDayMarker = "",
        found = false,
        isMarkerValid = false,
        canSave = false,
        dayBody = "",
        dayContentIsoDate = null,
        message = "ok"
    )
) : TxtStorageGateway {
    var lastDefaultMarkerMonth: String = ""
        private set
    var lastDefaultMarkerTargetDateIso: String = ""
        private set
    var lastReplaceContent: String = ""
        private set
    var lastReplaceDayMarker: String = ""
        private set
    var lastReplaceDayBody: String = ""
        private set

    override suspend fun inspectTxtFiles(): TxtInspectionResult = TxtInspectionResult(
        ok = true,
        entries = emptyList(),
        message = "ok"
    )

    override suspend fun listTxtFiles(): TxtHistoryListResult = TxtHistoryListResult(
        ok = true,
        files = emptyList(),
        message = "ok"
    )

    override suspend fun readTxtFile(relativePath: String): TxtFileContentResult = TxtFileContentResult(
        ok = true,
        filePath = relativePath,
        content = "",
        message = "ok"
    )

    override suspend fun saveTxtFileAndSync(
        relativePath: String,
        content: String
    ): RecordActionResult = RecordActionResult(ok = true, message = "ok")

    override suspend fun defaultTxtDayMarker(
        selectedMonth: String,
        targetDateIso: String
    ): TxtDayMarkerResult {
        lastDefaultMarkerMonth = selectedMonth
        lastDefaultMarkerTargetDateIso = targetDateIso
        return defaultMarkerResult
    }

    override suspend fun resolveTxtDayBlock(
        content: String,
        dayMarker: String,
        selectedMonth: String
    ): TxtDayBlockResolveResult = resolveResult

    override suspend fun replaceTxtDayBlock(
        content: String,
        dayMarker: String,
        editedDayBody: String
    ): TxtDayBlockReplaceResult {
        lastReplaceContent = content
        lastReplaceDayMarker = dayMarker
        lastReplaceDayBody = editedDayBody
        return replaceResult
    }
}
