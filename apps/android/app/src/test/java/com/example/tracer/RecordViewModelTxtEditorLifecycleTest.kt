package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.delay
import kotlinx.coroutines.test.StandardTestDispatcher
import kotlinx.coroutines.test.advanceUntilIdle
import kotlinx.coroutines.test.resetMain
import kotlinx.coroutines.test.runTest
import kotlinx.coroutines.test.setMain
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
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
            content = "y2026\nm04\nd0417\nold-day\nd0418\nold-all\n"
        )

        viewModel.openHistoryFile("2026/2026-04.txt")
        advanceUntilIdle()

        viewModel.updateEditableHistoryContent("y2026\nm04\nd0417\nold-day\nd0418\nedited-all\n")
        viewModel.discardUnsavedHistoryDraft()

        assertEquals(
            "y2026\nm04\nd0417\nold-day\nd0418\nold-all\n",
            viewModel.uiState.editableHistoryContent
        )

        viewModel.openHistoryFile("2026/2026-04.txt")
        advanceUntilIdle()

        assertEquals(
            "y2026\nm04\nd0417\nold-day\nd0418\nold-all\n",
            viewModel.uiState.editableHistoryContent
        )
    }

    @Test
    fun applyFrequentActivity_hidesFrequentActivitiesAndDoesNotReopenAfterAsyncCanonicalInsert() =
        runTest(dispatcher) {
            val runtime = TxtEditorLifecycleFakeRuntime(
                inspectionEntries = emptyList(),
                fileContents = emptyMap(),
                aliasMappings = listOf(
                    ActivityHierarchyLeafMappingEntry("快递", "routine_express")
                )
            )
            val viewModel = RecordViewModel(
                RecordUseCases(
                    recordGateway = runtime,
                    txtStorageGateway = runtime,
                    queryGateway = runtime
                )
            )

            viewModel.toggleFrequentActivities()
            advanceUntilIdle()
            assertEquals(true, viewModel.uiState.frequentActivitiesVisible)

            viewModel.applyFrequentActivity("routine_express")

            assertFalse(viewModel.uiState.frequentActivitiesVisible)
            advanceUntilIdle()

            assertFalse(viewModel.uiState.frequentActivitiesVisible)
            assertEquals("routine_express", viewModel.uiState.recordContent)
        }

    @Test
    fun queuedMonthNavigation_usesTheLatestSelectedStateForEachRequest() =
        runTest(dispatcher) {
            val filePaths = listOf(
                "2026/2026-01.txt",
                "2026/2026-02.txt",
                "2026/2026-03.txt"
            )
            val runtime = TxtEditorLifecycleFakeRuntime(
                inspectionEntries = filePaths.mapIndexed { index, path ->
                    TxtInspectionEntry(
                        relativePath = path,
                        headerMonth = "2026-0${index + 1}",
                        expectedCanonicalRelativePath = path,
                        syncState = TxtSyncState.SYNCED,
                        canOpen = true,
                        message = "ok"
                    )
                },
                fileContents = filePaths.associateWith { path -> path },
                readDelayByPathMs = mapOf("2026/2026-02.txt" to 100L)
            )
            val viewModel = RecordViewModel(
                RecordUseCases(
                    recordGateway = runtime,
                    txtStorageGateway = runtime,
                    queryGateway = runtime
                )
            )

            viewModel.openHistoryFile("2026/2026-01.txt")
            advanceUntilIdle()

            viewModel.openNextMonth()
            viewModel.openNextMonth()
            advanceUntilIdle()

            assertEquals("2026-03", viewModel.uiState.selectedMonth)
            assertEquals("2026/2026-03.txt", viewModel.uiState.selectedHistoryFile)
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
    private val fileContents: Map<String, String>,
    private val aliasMappings: List<ActivityHierarchyLeafMappingEntry> = emptyList(),
    private val readDelayByPathMs: Map<String, Long> = emptyMap()
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

    override suspend fun readTxtFile(relativePath: String): TxtFileContentResult {
        readDelayByPathMs[relativePath]?.let { delay(it) }
        return TxtFileContentResult(
            ok = true,
            filePath = relativePath,
            content = fileContents.getValue(relativePath),
            message = "ok"
        )
    }

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

    override suspend fun recordInterval(
        activityName: String,
        startTime: String,
        endTime: String,
        remark: String,
        targetDateIso: String?,
        preferredTxtPath: String?
    ): RecordActionResult = RecordActionResult(ok = true, message = "ok")

    override suspend fun syncLiveToDatabase(): NativeCallResult = NativeCallResult(
        initialized = true,
        operationOk = true,
        rawResponse = """{"ok":true}"""
    )

    override suspend fun queryFrequentActivities(
        lookbackDays: Int,
        topN: Int,
        anchorDateIso: String?
    ): ActivityFrequentResult = ActivityFrequentResult(
        ok = true,
        frequentActivities = listOf("routine_express"),
        message = "ok"
    )

    override suspend fun queryDayDurations(params: DataDurationQueryParams): DataQueryTextResult =
        DataQueryTextResult(ok = true, outputText = "", message = "ok")

    override suspend fun queryDayDurationStats(params: DataDurationQueryParams): DataQueryTextResult =
        DataQueryTextResult(ok = true, outputText = "", message = "ok")

    override suspend fun queryProjectTree(params: DataTreeQueryParams): TreeQueryResult =
        TreeQueryResult(ok = true, found = false, message = "ok")

    override suspend fun queryInsightsChart(params: InsightsChartQueryParams): InsightsChartQueryResult =
        InsightsChartQueryResult(ok = true, data = null, message = "ok")

    override suspend fun listActivityMappingNames(): ActivityMappingNamesResult =
        ActivityMappingNamesResult(ok = true, names = emptyList(), message = "ok")

    override suspend fun listActivityHierarchyLeafMappings(): ActivityHierarchyLeafMappingListResult =
        ActivityHierarchyLeafMappingListResult(ok = true, entries = aliasMappings, message = "ok")

    override suspend fun listCanonicalCatalog(): CanonicalCatalogResult =
        CanonicalCatalogResult(ok = true, roots = emptyList(), entries = emptyList(), message = "ok")
}
