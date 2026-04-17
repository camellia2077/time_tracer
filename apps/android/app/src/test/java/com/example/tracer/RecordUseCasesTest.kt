package com.example.tracer

import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test
import java.time.Clock
import java.time.Instant
import java.time.ZoneId

class RecordUseCasesTest {
    @Test
    fun recordNow_clearsActivityAndRemarkAfterSuccessfulInsert() = runTest {
        val inspectionEntries = listOf(
            TxtInspectionEntry(
                relativePath = "2026/2026-03.txt",
                headerMonth = "2026-03",
                expectedCanonicalRelativePath = "2026/2026-03.txt",
                syncState = TxtSyncState.SYNCED,
                canOpen = true,
                message = "ok"
            )
        )
        val useCases = RecordUseCases(
            recordGateway = FakeRecordGateway(
                recordNowResult = RecordActionResult(
                    ok = true,
                    message = "record: ok"
                )
            ),
            txtStorageGateway = FakeTxtStorageGateway(
                inspectionResult = TxtInspectionResult(
                    ok = true,
                    entries = inspectionEntries,
                    message = "ok"
                ),
                readResults = mapOf(
                    "2026/2026-03.txt" to TxtFileContentResult(
                        ok = true,
                        filePath = "2026/2026-03.txt",
                        content = "y2026\nm03\n0101\n",
                        message = "Read TXT success."
                    )
                )
            ),
            queryGateway = FakeQueryGateway()
        )

        val result = useCases.recordNow(
            RecordUiState(
                recordContent = "coding",
                recordRemark = "ship atomic record",
                selectedMonth = "2026-03"
            )
        )

        assertEquals("", result.recordContent)
        assertEquals("", result.recordRemark)
    }

    @Test
    fun recordNow_clearsActivityAndRemarkAfterFailedInsert() = runTest {
        val inspectionEntries = listOf(
            TxtInspectionEntry(
                relativePath = "2026/2026-03.txt",
                headerMonth = "2026-03",
                expectedCanonicalRelativePath = "2026/2026-03.txt",
                syncState = TxtSyncState.SYNCED,
                canOpen = true,
                message = "ok"
            )
        )
        val useCases = RecordUseCases(
            recordGateway = FakeRecordGateway(
                recordNowResult = RecordActionResult(
                    ok = false,
                    message = "record: failed"
                )
            ),
            txtStorageGateway = FakeTxtStorageGateway(
                inspectionResult = TxtInspectionResult(
                    ok = true,
                    entries = inspectionEntries,
                    message = "ok"
                ),
                readResults = mapOf(
                    "2026/2026-03.txt" to TxtFileContentResult(
                        ok = true,
                        filePath = "2026/2026-03.txt",
                        content = "y2026\nm03\n0101\n",
                        message = "Read TXT success."
                    )
                )
            ),
            queryGateway = FakeQueryGateway()
        )

        val result = useCases.recordNow(
            RecordUiState(
                recordContent = "coding",
                recordRemark = "ship atomic record",
                selectedMonth = "2026-03"
            )
        )

        assertEquals("", result.recordContent)
        assertEquals("", result.recordRemark)
    }

    @Test
    fun openHistoryFile_allowsOpeningBlockedTxtForRepair() = runTest {
        val inspectionEntries = listOf(
            TxtInspectionEntry(
                relativePath = "broken.txt",
                headerMonth = null,
                expectedCanonicalRelativePath = null,
                syncState = TxtSyncState.HEADER_INVALID,
                canOpen = false,
                message = "TXT is missing valid yYYYY + mMM headers."
            )
        )
        val useCases = RecordUseCases(
            recordGateway = FakeRecordGateway(),
            txtStorageGateway = FakeTxtStorageGateway(
                inspectionResult = TxtInspectionResult(
                    ok = true,
                    entries = inspectionEntries,
                    message = "ok"
                ),
                readResults = mapOf(
                    "broken.txt" to TxtFileContentResult(
                        ok = true,
                        filePath = "broken.txt",
                        content = "bad content",
                        message = "Read TXT success."
                    )
                )
            ),
            queryGateway = FakeQueryGateway()
        )

        val result = useCases.openHistoryFile(RecordUiState(), "broken.txt")

        assertEquals(listOf("broken.txt"), result.historyFiles)
        assertEquals(inspectionEntries, result.txtInspectionEntries)
        assertEquals("broken.txt", result.selectedHistoryFile)
        assertEquals("bad content", result.selectedHistoryContent)
        assertEquals("bad content", result.editableHistoryContent)
        assertEquals("", result.selectedMonth)
        assertTrue(result.statusText.contains("repair txt -> broken.txt"))
    }

    @Test
    fun refreshHistory_opensBrokenMonthTxtForRepairWithoutFilePicker() = runTest {
        val inspectionEntries = listOf(
            TxtInspectionEntry(
                relativePath = "2026/2026-03.txt",
                headerMonth = "2026-03",
                expectedCanonicalRelativePath = "2026/2026-03.txt",
                syncState = TxtSyncState.DB_HASH_MISMATCH,
                canOpen = false,
                message = "TXT content differs from the version last ingested into DB."
            )
        )
        val useCases = RecordUseCases(
            recordGateway = FakeRecordGateway(),
            txtStorageGateway = FakeTxtStorageGateway(
                inspectionResult = TxtInspectionResult(
                    ok = true,
                    entries = inspectionEntries,
                    message = "ok"
                ),
                readResults = mapOf(
                    "2026/2026-03.txt" to TxtFileContentResult(
                        ok = true,
                        filePath = "2026/2026-03.txt",
                        content = "y2026\nm03\n0101\n",
                        message = "Read TXT success."
                    )
                )
            ),
            queryGateway = FakeQueryGateway()
        )

        val result = useCases.refreshHistory(RecordUiState())

        assertEquals(listOf("2026/2026-03.txt"), result.historyFiles)
        assertEquals(inspectionEntries, result.txtInspectionEntries)
        assertEquals(emptyList<String>(), result.availableMonths)
        assertEquals("2026/2026-03.txt", result.selectedHistoryFile)
        assertEquals("2026-03", result.selectedMonth)
        assertTrue(result.statusText.contains("Repair needed"))
    }

    @Test
    fun recordNow_beforeCutoff_resolvesYesterdayAndOpensPreviousMonth() = runTest {
        val gateway = FakeRecordGateway()
        val useCases = RecordUseCases(
            recordGateway = gateway,
            txtStorageGateway = FakeTxtStorageGateway(
                inspectionResult = TxtInspectionResult(
                    ok = true,
                    entries = listOf(
                        inspectionEntry("2026/2026-03.txt", "2026-03"),
                        inspectionEntry("2026/2026-04.txt", "2026-04")
                    ),
                    message = "ok"
                ),
                readResults = mapOf(
                    "2026/2026-03.txt" to txtReadResult("2026/2026-03.txt")
                )
            ),
            queryGateway = FakeQueryGateway(),
            clock = fixedClock("2026-03-31T16:30:00Z", "Asia/Shanghai")
        )

        val result = useCases.recordNow(
            RecordUiState(
                recordContent = "coding",
                logicalDayTarget = RecordLogicalDayTarget.YESTERDAY
            )
        )

        assertEquals("2026-03-31", gateway.lastTargetDateIso)
        assertEquals(RecordTimeOrderMode.LOGICAL_DAY_0600, gateway.lastTimeOrderMode)
        assertEquals("2026-03", result.selectedMonth)
        assertEquals("2026/2026-03.txt", result.selectedHistoryFile)
    }

    @Test
    fun recordNow_afterCutoff_resolvesToday() = runTest {
        val gateway = FakeRecordGateway()
        val useCases = RecordUseCases(
            recordGateway = gateway,
            txtStorageGateway = FakeTxtStorageGateway(
                inspectionResult = TxtInspectionResult(
                    ok = true,
                    entries = listOf(inspectionEntry("2026/2026-03.txt", "2026-03")),
                    message = "ok"
                ),
                readResults = mapOf(
                    "2026/2026-03.txt" to txtReadResult("2026/2026-03.txt")
                )
            ),
            queryGateway = FakeQueryGateway(),
            clock = fixedClock("2026-03-30T00:30:00Z", "Asia/Shanghai")
        )

        useCases.recordNow(
            RecordUiState(
                recordContent = "coding",
                logicalDayTarget = RecordLogicalDayTarget.TODAY
            )
        )

        assertEquals("2026-03-30", gateway.lastTargetDateIso)
        assertEquals(RecordTimeOrderMode.STRICT_CALENDAR, gateway.lastTimeOrderMode)
    }

    @Test
    fun saveHistoryFileAndSync_usesEditableHistoryContentAsTheOnlySaveSource() = runTest {
        val txtStorageGateway = FakeTxtStorageGateway(
            inspectionResult = TxtInspectionResult(
                ok = true,
                entries = listOf(inspectionEntry("2026/2026-03.txt", "2026-03")),
                message = "ok"
            ),
            readResults = mapOf(
                "2026/2026-03.txt" to txtReadResult("2026/2026-03.txt")
            ),
            saveResult = RecordActionResult(
                ok = true,
                message = "save ok"
            )
        )
        val useCases = RecordUseCases(
            recordGateway = FakeRecordGateway(),
            txtStorageGateway = txtStorageGateway,
            queryGateway = FakeQueryGateway()
        )

        val result = useCases.saveHistoryFileAndSync(
            RecordUiState(
                selectedHistoryFile = "2026/2026-03.txt",
                selectedHistoryContent = "old content",
                editableHistoryContent = "new content"
            )
        )

        assertEquals("2026/2026-03.txt", txtStorageGateway.lastSavedRelativePath)
        assertEquals("new content", txtStorageGateway.lastSavedContent)
        assertEquals("new content", result.selectedHistoryContent)
    }

    @Test
    fun openHistoryFile_restoresInSessionDraftForPreviouslyEditedTxt() = runTest {
        val useCases = RecordUseCases(
            recordGateway = FakeRecordGateway(),
            txtStorageGateway = FakeTxtStorageGateway(
                inspectionResult = TxtInspectionResult(
                    ok = true,
                    entries = listOf(
                        inspectionEntry("2026/2026-03.txt", "2026-03"),
                        inspectionEntry("2026/2026-04.txt", "2026-04")
                    ),
                    message = "ok"
                ),
                readResults = mapOf(
                    "2026/2026-03.txt" to txtReadResult("2026/2026-03.txt"),
                    "2026/2026-04.txt" to TxtFileContentResult(
                        ok = true,
                        filePath = "2026/2026-04.txt",
                        content = "y2026\nm04\n0101\n",
                        message = "Read TXT success."
                    )
                )
            ),
            queryGateway = FakeQueryGateway()
        )

        val result = useCases.openHistoryFile(
            state = RecordUiState(
                historyDraftsByFile = mapOf("2026/2026-03.txt" to "draft-content")
            ),
            path = "2026/2026-03.txt"
        )

        assertEquals("2026/2026-03.txt", result.selectedHistoryFile)
        assertEquals("y2026\nm03\n0101\n", result.selectedHistoryContent)
        assertEquals("draft-content", result.editableHistoryContent)
    }

    @Test
    fun openTxtPreview_refreshesLogicalDayMonthBeforeCutoff() = runTest {
        val useCases = RecordUseCases(
            recordGateway = FakeRecordGateway(),
            txtStorageGateway = FakeTxtStorageGateway(
                inspectionResult = TxtInspectionResult(
                    ok = true,
                    entries = listOf(
                        inspectionEntry("2026/2026-03.txt", "2026-03"),
                        inspectionEntry("2026/2026-04.txt", "2026-04")
                    ),
                    message = "ok"
                ),
                readResults = mapOf(
                    "2026/2026-03.txt" to txtReadResult("2026/2026-03.txt")
                )
            ),
            queryGateway = FakeQueryGateway(),
            clock = fixedClock("2026-03-31T16:30:00Z", "Asia/Shanghai")
        )

        val result = useCases.openTxtPreview(
            RecordUiState(
                logicalDayTarget = RecordLogicalDayTarget.YESTERDAY,
                selectedMonth = "2026-04",
                selectedHistoryFile = "2026/2026-04.txt",
                editableHistoryContent = "stale"
            )
        )

        assertEquals("2026-03", result.selectedMonth)
        assertEquals("2026/2026-03.txt", result.selectedHistoryFile)
        assertTrue(result.statusText.startsWith("TXT preview refreshed."))
    }
}

private class FakeTxtStorageGateway(
    private val inspectionResult: TxtInspectionResult,
    private val readResults: Map<String, TxtFileContentResult>,
    private val defaultDayMarkerResult: TxtDayMarkerResult = TxtDayMarkerResult(
        ok = true,
        normalizedDayMarker = "0101",
        message = "ok"
    ),
    private val dayBlockResolveResult: TxtDayBlockResolveResult = TxtDayBlockResolveResult(
        ok = true,
        normalizedDayMarker = "0101",
        found = true,
        isMarkerValid = true,
        canSave = true,
        dayBody = "",
        dayContentIsoDate = "2026-01-01",
        message = "ok"
    ),
    private val saveResult: RecordActionResult = RecordActionResult(
        ok = true,
        message = "ok"
    )
) : TxtStorageGateway {
    var lastSavedRelativePath: String? = null
    var lastSavedContent: String? = null

    override suspend fun inspectTxtFiles(): TxtInspectionResult = inspectionResult

    override suspend fun listTxtFiles(): TxtHistoryListResult = TxtHistoryListResult(
        ok = true,
        files = inspectionResult.entries.map { it.relativePath },
        message = "ok"
    )

    override suspend fun readTxtFile(relativePath: String): TxtFileContentResult {
        return readResults[relativePath] ?: TxtFileContentResult(
            ok = false,
            filePath = relativePath,
            content = "",
            message = "TXT file not found."
        )
    }

    override suspend fun saveTxtFileAndSync(
        relativePath: String,
        content: String
    ): RecordActionResult {
        lastSavedRelativePath = relativePath
        lastSavedContent = content
        return saveResult
    }

    override suspend fun defaultTxtDayMarker(
        selectedMonth: String,
        targetDateIso: String
    ): TxtDayMarkerResult = defaultDayMarkerResult

    override suspend fun resolveTxtDayBlock(
        content: String,
        dayMarker: String,
        selectedMonth: String
    ): TxtDayBlockResolveResult = dayBlockResolveResult
}

private class FakeRecordGateway(
    private val recordNowResult: RecordActionResult = RecordActionResult(
        ok = true,
        message = "ok"
    )
) : RecordGateway {
    var lastTargetDateIso: String? = null
    var lastTimeOrderMode: RecordTimeOrderMode? = null

    override suspend fun clearTxt(): ClearTxtResult = ClearTxtResult(
        ok = true,
        message = "ok"
    )

    override suspend fun createCurrentMonthTxt(): RecordActionResult = RecordActionResult(
        ok = true,
        message = "ok"
    )

    override suspend fun createMonthTxt(month: String): RecordActionResult = RecordActionResult(
        ok = true,
        message = "ok"
    )

    override suspend fun recordNow(
        activityName: String,
        remark: String,
        targetDateIso: String?,
        preferredTxtPath: String?,
        timeOrderMode: RecordTimeOrderMode
    ): RecordActionResult {
        lastTargetDateIso = targetDateIso
        lastTimeOrderMode = timeOrderMode
        return recordNowResult
    }

    override suspend fun syncLiveToDatabase(): NativeCallResult = NativeCallResult(
        initialized = true,
        operationOk = true,
        rawResponse = """{"ok":true}"""
    )
}

private class FakeQueryGateway : QueryGateway {
    override suspend fun queryActivitySuggestions(
        lookbackDays: Int,
        topN: Int
    ): ActivitySuggestionResult = ActivitySuggestionResult(
        ok = true,
        suggestions = emptyList(),
        message = "ok"
    )

    override suspend fun queryDayDurations(params: DataDurationQueryParams): DataQueryTextResult =
        DataQueryTextResult(ok = true, outputText = "", message = "ok")

    override suspend fun queryDayDurationStats(params: DataDurationQueryParams): DataQueryTextResult =
        DataQueryTextResult(ok = true, outputText = "", message = "ok")

    override suspend fun queryProjectTree(params: DataTreeQueryParams): TreeQueryResult =
        TreeQueryResult(ok = true, found = false, message = "ok")

    override suspend fun queryProjectTreeText(params: DataTreeQueryParams): DataQueryTextResult =
        DataQueryTextResult(ok = true, outputText = "", message = "ok")

    override suspend fun queryReportChart(params: ReportChartQueryParams): ReportChartQueryResult =
        ReportChartQueryResult(ok = true, data = null, message = "ok")

    override suspend fun listActivityMappingNames(): ActivityMappingNamesResult =
        ActivityMappingNamesResult(ok = true, names = emptyList(), message = "ok")
}

private fun inspectionEntry(relativePath: String, month: String): TxtInspectionEntry =
    TxtInspectionEntry(
        relativePath = relativePath,
        headerMonth = month,
        expectedCanonicalRelativePath = relativePath,
        syncState = TxtSyncState.SYNCED,
        canOpen = true,
        message = "ok"
    )

private fun txtReadResult(filePath: String): TxtFileContentResult =
    TxtFileContentResult(
        ok = true,
        filePath = filePath,
        content = "y2026\nm03\n0101\n",
        message = "Read TXT success."
    )

private fun fixedClock(instantIso: String, zoneId: String): Clock =
    Clock.fixed(Instant.parse(instantIso), ZoneId.of(zoneId))
