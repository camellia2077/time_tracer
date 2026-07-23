package com.example.tracer

import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Test
import java.time.Clock
import java.time.Instant
import java.time.ZoneId

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
        val coordinator = TxtEditorRuntimeCoordinator(gateway, testClock())
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
        val coordinator = TxtEditorRuntimeCoordinator(gateway, testClock())
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
        val coordinator = TxtEditorRuntimeCoordinator(gateway, testClock())
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

    @Test
    fun convertActivityNames_usesCurrentMonthDraftAndReturnsConvertedText() = runBlocking {
        val gateway = FakeTxtEditorRuntimeGateway(
            conversionResult = TxtActivityNameConversionResult(
                ok = true,
                convertedContent = "canonical-month",
                message = "ok"
            )
        )
        val coordinator = TxtEditorRuntimeCoordinator(gateway, testClock())

        val result = coordinator.convertActivityNames(
            content = "alias-month",
            targetMode = TxtActivityNameTargetMode.CANONICAL
        )

        assertTrue(result.ok)
        assertEquals("alias-month", gateway.lastConversionContent)
        assertEquals(
            TxtActivityNameMappingDirection.ALIAS_TO_CANONICAL,
            gateway.lastConversionDirection
        )
        assertEquals("canonical-month", result.convertedContent)
    }

    @Test
    fun prepareEditableDayBlock_whenDayIsMissing_seedsEmptyDayBlock() = runBlocking {
        val gateway = FakeTxtEditorRuntimeGateway(
            resolveResult = TxtDayBlockResolveResult(
                ok = true,
                normalizedDayMarker = "0501",
                found = false,
                isMarkerValid = true,
                canSave = false,
                dayBody = "",
                dayContentIsoDate = "2026-05-01",
                message = "ok"
            )
        )
        val coordinator = TxtEditorRuntimeCoordinator(gateway, testClock())

        val result = coordinator.prepareEditableDayBlock(
            monthContent = "y2026\nm05\n",
            dayMarker = "0501",
            selectedMonth = "2026-05"
        )

        assertTrue(result.canEdit)
        assertTrue(result.resolveResult.found)
        assertTrue(result.resolveResult.canSave)
        assertEquals("y2026\nm05\n\nd0501\n", result.monthContent)
    }

    @Test
    fun prepareEditableDayBlock_whenMarkerMonthDoesNotMatchSelectedMonth_doesNotSeedDraft() = runBlocking {
        val gateway = FakeTxtEditorRuntimeGateway(
            resolveResult = TxtDayBlockResolveResult(
                ok = true,
                normalizedDayMarker = "0101",
                found = false,
                isMarkerValid = true,
                canSave = false,
                dayBody = "",
                dayContentIsoDate = null,
                message = "ok"
            )
        )
        val coordinator = TxtEditorRuntimeCoordinator(gateway, testClock())

        val result = coordinator.prepareEditableDayBlock(
            monthContent = "y2026\nm06\n",
            dayMarker = "0101",
            selectedMonth = "2026-06"
        )

        assertFalse(result.canEdit)
        assertFalse(result.resolveResult.found)
        assertEquals("y2026\nm06\n", result.monthContent)
    }

    @Test
    fun openDayEditor_whenColdStartResolveIsStillDefault_reloadsMarkerBeforeOpening() = runBlocking {
        val gateway = FakeTxtEditorRuntimeGateway(
            defaultMarkerResult = TxtDayMarkerResult(
                ok = true,
                normalizedDayMarker = "0417",
                message = "ok"
            ),
            resolveTxtDayBlockOverride = { _, dayMarker, selectedMonth ->
                if (dayMarker == "0417" && selectedMonth == "2026-04") {
                    TxtDayBlockResolveResult(
                        ok = true,
                        normalizedDayMarker = "0417",
                        found = true,
                        isMarkerValid = true,
                        canSave = true,
                        dayBody = "0900study\n1000break\n",
                        dayContentIsoDate = "2026-04-17",
                        message = "ok"
                    )
                } else {
                    TxtDayBlockResolveResult(
                        ok = true,
                        normalizedDayMarker = dayMarker,
                        found = false,
                        isMarkerValid = true,
                        canSave = false,
                        dayBody = "",
                        dayContentIsoDate = null,
                        message = "ok"
                    )
                }
            }
        )
        val coordinator = TxtEditorRuntimeCoordinator(gateway, testClock())
        val controller = TxtEditorSessionController()
        var reconciledMonthContent = ""

        controller.syncExternalMonthDraft(
            selectedHistoryContent = "y2026\nm04\n\nd0417\n0900study\n1000break\n",
            editableHistoryContent = "y2026\nm04\n\nd0417\n0900study\n1000break\n"
        )

        coordinator.openDayEditor(
            sessionController = controller,
            selectedHistoryFile = "2026/2026-04.txt",
            selectedMonth = "2026-04",
            logicalDayTarget = RecordLogicalDayTarget.TODAY,
            fallbackMonthContent = controller.state.allDraftState.draftText,
            persistedMonthContent = controller.state.allDraftState.draftText,
            currentResolveResult = TxtDayBlockResolveResult(
                ok = true,
                normalizedDayMarker = "0101",
                found = false,
                isMarkerValid = true,
                canSave = false,
                dayBody = "",
                dayContentIsoDate = null,
                message = "ok"
            ),
            onMonthContentReconciled = { reconciledMonthContent = it }
        )

        assertEquals("0417", controller.state.dayMarkerInput)
        assertTrue(controller.state.isEditorContentVisible)
        assertEquals("0900study\n1000break\n", controller.state.dayDraftState.draftText)
        assertEquals("", reconciledMonthContent)
    }

    @Test
    fun prepareEditableDayBlock_whenMonthHeadersAreMissing_doesNotSeedPlaceholderDay() = runBlocking {
        val gateway = FakeTxtEditorRuntimeGateway()
        val coordinator = TxtEditorRuntimeCoordinator(gateway, testClock())

        val result = coordinator.prepareEditableDayBlock(
            monthContent = "",
            dayMarker = "0624",
            selectedMonth = "2026-06"
        )

        assertFalse(result.canEdit)
        assertFalse(result.resolveResult.found)
        assertEquals("", result.monthContent)
    }

    @Test
    fun determineOpenDayPreparationStrategy_prefersCanonicalPersistedContent_forInvalidSessionDraft() {
        val strategy = determineOpenDayPreparationStrategy(
            sessionMonthContent = "d0624\n",
            editableMonthContent = "d0624\n",
            persistedMonthContent = "y2026\nm06\n\nd0624\n0650w\n0700study\n",
            currentResolveResult = TxtDayBlockResolveResult(
                ok = false,
                normalizedDayMarker = "0624",
                found = false,
                isMarkerValid = false,
                canSave = false,
                dayBody = "",
                dayContentIsoDate = null,
                message = ""
            )
        )

        assertEquals("y2026\nm06\n\nd0624\n0650w\n0700study\n", strategy.monthContent)
        assertNull(strategy.resolveResult)
        assertTrue(strategy.requiresMarkerReload)
    }

    @Test
    fun determineOpenDayPreparationStrategy_reusesCurrentResolve_whenIsoDateIsAlreadyValid() {
        val currentResolve = TxtDayBlockResolveResult(
            ok = true,
            normalizedDayMarker = "0624",
            found = true,
            isMarkerValid = true,
            canSave = true,
            dayBody = "0650w\n0700study\n",
            dayContentIsoDate = "2026-06-24",
            message = "ok"
        )

        val strategy = determineOpenDayPreparationStrategy(
            sessionMonthContent = "y2026\nm06\n\nd0624\n0650w\n0700study\n",
            editableMonthContent = "y2026\nm06\n\nd0624\n0650w\n0700study\n",
            persistedMonthContent = "y2026\nm06\n\nd0624\n0650w\n0700study\n",
            currentResolveResult = currentResolve
        )

        assertEquals(currentResolve, strategy.resolveResult)
        assertFalse(strategy.requiresMarkerReload)
    }

    @Test
    fun openDayEditor_prefersPersistedMonthContent_overInvalidSessionPlaceholder() = runBlocking {
        val gateway = FakeTxtEditorRuntimeGateway(
            defaultMarkerResult = TxtDayMarkerResult(
                ok = true,
                normalizedDayMarker = "0624",
                message = "ok"
            ),
            resolveTxtDayBlockOverride = { content, dayMarker, selectedMonth ->
                if (content.contains("d0624") && dayMarker == "0624" && selectedMonth == "2026-06") {
                    TxtDayBlockResolveResult(
                        ok = true,
                        normalizedDayMarker = "0624",
                        found = true,
                        isMarkerValid = true,
                        canSave = true,
                        dayBody = "0650w\n0700study\n",
                        dayContentIsoDate = "2026-06-24",
                        message = "ok"
                    )
                } else {
                    TxtDayBlockResolveResult(
                        ok = true,
                        normalizedDayMarker = dayMarker,
                        found = false,
                        isMarkerValid = true,
                        canSave = false,
                        dayBody = "",
                        dayContentIsoDate = null,
                        message = "ok"
                    )
                }
            }
        )
        val coordinator = TxtEditorRuntimeCoordinator(gateway, testClock())
        val controller = TxtEditorSessionController()

        controller.syncExternalMonthDraft(
            selectedHistoryContent = "d0624\n",
            editableHistoryContent = "d0624\n"
        )

        coordinator.openDayEditor(
            sessionController = controller,
            selectedHistoryFile = "2026/2026-06.txt",
            selectedMonth = "2026-06",
            logicalDayTarget = RecordLogicalDayTarget.TODAY,
            fallbackMonthContent = "d0624\n",
            persistedMonthContent = "y2026\nm06\n\nd0624\n0650w\n0700study\n",
            currentResolveResult = TxtDayBlockResolveResult(
                ok = false,
                normalizedDayMarker = "0624",
                found = false,
                isMarkerValid = false,
                canSave = false,
                dayBody = "",
                dayContentIsoDate = null,
                message = ""
            ),
            onMonthContentReconciled = {}
        )

        assertTrue(controller.state.isEditorContentVisible)
        assertEquals("0650w\n0700study\n", controller.state.dayDraftState.draftText)
    }
}

private fun testClock(): Clock =
    Clock.fixed(Instant.parse("2026-04-17T12:00:00Z"), ZoneId.of("Asia/Shanghai"))

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
    ),
    private val resolveTxtDayBlockOverride: (suspend (String, String, String) -> TxtDayBlockResolveResult)? = null,
    private val conversionResult: TxtActivityNameConversionResult = TxtActivityNameConversionResult(
        ok = true,
        convertedContent = "",
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
    var lastConversionContent: String = ""
        private set
    var lastConversionDirection: TxtActivityNameMappingDirection? = null
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
    ): TxtDayBlockResolveResult =
        resolveTxtDayBlockOverride?.invoke(content, dayMarker, selectedMonth) ?: resolveResult

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

    override suspend fun convertTxtActivityNames(
        content: String,
        direction: TxtActivityNameMappingDirection
    ): TxtActivityNameConversionResult {
        lastConversionContent = content
        lastConversionDirection = direction
        return conversionResult
    }
}
