package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.test.StandardTestDispatcher
import kotlinx.coroutines.test.advanceUntilIdle
import kotlinx.coroutines.test.resetMain
import kotlinx.coroutines.test.runTest
import kotlinx.coroutines.test.setMain
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Before
import org.junit.Test

@OptIn(ExperimentalCoroutinesApi::class)
class RecordViewModelTxtEditorLifecycleTest {
    private val dispatcher = StandardTestDispatcher()

    @Before
    fun setUp() {
        Dispatchers.setMain(dispatcher)
    }

    @After
    fun tearDown() {
        Dispatchers.resetMain()
    }

    @Test
    fun allDraft_withoutIngest_closeAndReopen_restoresSavedContent() = runTest(dispatcher) {
        val viewModel = buildViewModelWithTxt(
            filePath = "2026/2026-04.txt",
            month = "2026-04",
            content = "y2026\nm04\n0417\nold-day\n0418\nold-all\n"
        )

        viewModel.openHistoryFile("2026/2026-04.txt")
        advanceUntilIdle()

        viewModel.updateEditableHistoryContent("y2026\nm04\n0417\nold-day\n0418\nedited-all\n")
        viewModel.discardUnsavedHistoryDraft()

        assertEquals(
            "y2026\nm04\n0417\nold-day\n0418\nold-all\n",
            viewModel.uiState.editableHistoryContent
        )

        viewModel.openHistoryFile("2026/2026-04.txt")
        advanceUntilIdle()

        assertEquals(
            "y2026\nm04\n0417\nold-day\n0418\nold-all\n",
            viewModel.uiState.editableHistoryContent
        )
    }
}

private fun buildViewModelWithTxt(
    filePath: String,
    month: String,
    content: String
): RecordViewModel {
    val gateway = TxtEditorLifecycleFakeRuntime(
        inspectionEntries = listOf(
            TxtInspectionEntry(
                relativePath = filePath,
                headerMonth = month,
                expectedCanonicalRelativePath = filePath,
                syncState = TxtSyncState.SYNCED,
                canOpen = true,
                message = "ok"
            )
        ),
        fileContents = mapOf(filePath to content)
    )
    return RecordViewModel(
        RecordUseCases(
            recordGateway = gateway,
            txtStorageGateway = gateway,
            queryGateway = gateway
        )
    )
}

private class TxtEditorLifecycleFakeRuntime(
    private val inspectionEntries: List<TxtInspectionEntry>,
    private val fileContents: Map<String, String>
) : RecordGateway, TxtStorageGateway, QueryGateway {
    override suspend fun inspectTxtFiles(): TxtInspectionResult = TxtInspectionResult(
        ok = true,
        entries = inspectionEntries,
        message = "ok"
    )

    override suspend fun listTxtFiles(): TxtHistoryListResult = TxtHistoryListResult(
        ok = true,
        files = inspectionEntries.map { it.relativePath },
        message = "ok"
    )

    override suspend fun readTxtFile(relativePath: String): TxtFileContentResult = TxtFileContentResult(
        ok = true,
        filePath = relativePath,
        content = fileContents.getValue(relativePath),
        message = "ok"
    )

    override suspend fun saveTxtFileAndSync(
        relativePath: String,
        content: String
    ): RecordActionResult = RecordActionResult(ok = true, message = "ok")

    override suspend fun clearTxt(): ClearTxtResult = ClearTxtResult(ok = true, message = "ok")

    override suspend fun createCurrentMonthTxt(): RecordActionResult =
        RecordActionResult(ok = true, message = "ok")

    override suspend fun createMonthTxt(month: String): RecordActionResult =
        RecordActionResult(ok = true, message = "ok")

    override suspend fun recordNow(
        activityName: String,
        remark: String,
        targetDateIso: String?,
        preferredTxtPath: String?,
        timeOrderMode: RecordTimeOrderMode
    ): RecordActionResult = RecordActionResult(ok = true, message = "ok")

    override suspend fun syncLiveToDatabase(): NativeCallResult = NativeCallResult(
        initialized = true,
        operationOk = true,
        rawResponse = """{"ok":true}"""
    )

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
