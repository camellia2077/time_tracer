package com.example.tracer

import androidx.compose.material3.SnackbarDuration
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.test.StandardTestDispatcher
import kotlinx.coroutines.test.advanceUntilIdle
import kotlinx.coroutines.test.resetMain
import kotlinx.coroutines.test.runTest
import kotlinx.coroutines.test.setMain
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test

@OptIn(ExperimentalCoroutinesApi::class)
class TracerTabRegistryTest {
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
    fun registry_contains_all_tabs_in_expected_order() {
        val ids = TracerTabRegistry.entries.map { it.meta.id }
        assertEquals(
            listOf(
                TracerTab.DATA,
                TracerTab.REPORT,
                TracerTab.RECORD,
                TracerTab.TXT,
                TracerTab.CONFIG
            ),
            ids
        )
    }

    @Test
    fun onEnter_record_refreshes_mapping_and_clears_unavailable_status() = runTest(dispatcher) {
        val runtime = FakeRuntimeGateway(
            mappingNamesResult = ActivityMappingNamesResult(
                ok = true,
                names = listOf("meal", "study"),
                message = "ok"
            )
        )
        val recordViewModel = RecordViewModel(
            RecordUseCases(
                recordGateway = runtime,
                txtStorageGateway = runtime,
                queryGateway = runtime
            )
        )
        val configViewModel = ConfigViewModel(runtime)
        advanceUntilIdle()

        recordViewModel.setStatusText("Activity mapping validation unavailable: stale")
        var capturedNames: Set<String> = emptySet()

        TracerTabRegistry.onEnter(
            tab = TracerTab.RECORD,
            args = TracerTabLifecycleArgs(
                controller = runtime,
                recordViewModel = recordViewModel,
                configViewModel = configViewModel,
                recordStatusText = { recordViewModel.uiState.statusText },
                onValidMappingNamesChanged = { capturedNames = it }
            )
        )

        assertEquals(setOf("meal", "study"), capturedNames)
        assertEquals("", recordViewModel.uiState.statusText)
    }

    @Test
    fun onLeave_txt_discards_unsaved_editor_draft() = runTest(dispatcher) {
        val runtime = FakeRuntimeGateway()
        val recordViewModel = RecordViewModel(
            RecordUseCases(
                recordGateway = runtime,
                txtStorageGateway = runtime,
                queryGateway = runtime
            )
        )
        val configViewModel = ConfigViewModel(runtime)
        advanceUntilIdle()

        recordViewModel.updateEditableHistoryContent("draft-content")
        assertEquals("draft-content", recordViewModel.uiState.editableHistoryContent)

        TracerTabRegistry.onLeave(
            tab = TracerTab.TXT,
            args = TracerTabLifecycleArgs(
                controller = runtime,
                recordViewModel = recordViewModel,
                configViewModel = configViewModel,
                recordStatusText = { recordViewModel.uiState.statusText },
                onValidMappingNamesChanged = {}
            )
        )

        assertEquals(recordViewModel.uiState.selectedHistoryContent, recordViewModel.uiState.editableHistoryContent)
    }

    @Test
    fun onLeave_config_discards_unsaved_toml_draft() = runTest(dispatcher) {
        val runtime = FakeRuntimeGateway()
        val recordViewModel = RecordViewModel(
            RecordUseCases(
                recordGateway = runtime,
                txtStorageGateway = runtime,
                queryGateway = runtime
            )
        )
        val configViewModel = ConfigViewModel(runtime)
        advanceUntilIdle()

        configViewModel.onEditableContentChange("unsaved")
        assertEquals("unsaved", configViewModel.uiState.editableContent)

        TracerTabRegistry.onLeave(
            tab = TracerTab.CONFIG,
            args = TracerTabLifecycleArgs(
                controller = runtime,
                recordViewModel = recordViewModel,
                configViewModel = configViewModel,
                recordStatusText = { recordViewModel.uiState.statusText },
                onValidMappingNamesChanged = {}
            )
        )

        assertEquals(configViewModel.uiState.selectedFileContent, configViewModel.uiState.editableContent)
    }

    @Test
    fun statusEvent_partialExportCompletion_emits_selectTxt_action() {
        val event = TracerTabRegistry.statusEvent(
            tab = TracerTab.DATA,
            args = TracerTabStatusEventArgs(
                selectedTab = TracerTab.DATA,
                statusText = "Export all completed -> 1/2 failed",
                lastObservedTab = TracerTab.DATA,
                lastObservedStatus = "previous"
            )
        )

        assertTrue(event is TracerTabUiEvent.ShowSnackbar)
        val snackbar = event as TracerTabUiEvent.ShowSnackbar
        assertEquals("Export all completed with issues.", snackbar.message)
        assertEquals("Details", snackbar.actionLabel)
        assertEquals(SnackbarDuration.Long, snackbar.duration)
        assertEquals(TracerCoordinatorEvent.SelectTab(TracerTab.TXT), snackbar.onActionEvent)
    }

    @Test
    fun statusEvent_txt_tab_is_suppressed() {
        val event = TracerTabRegistry.statusEvent(
            tab = TracerTab.TXT,
            args = TracerTabStatusEventArgs(
                selectedTab = TracerTab.TXT,
                statusText = "any status update",
                lastObservedTab = TracerTab.TXT,
                lastObservedStatus = "previous"
            )
        )

        assertNull(event)
    }
}

private class FakeRuntimeGateway(
    private val mappingNamesResult: ActivityMappingNamesResult = ActivityMappingNamesResult(
        ok = true,
        names = emptyList(),
        message = "ok"
    )
) : RuntimeGateway {
    override suspend fun initializeRuntime(): NativeCallResult = NativeCallResult(
        initialized = true,
        operationOk = true,
        rawResponse = """{"ok":true}"""
    )

    override suspend fun ingestFull(): NativeCallResult = initializeRuntime()

    override suspend fun ingestSingleTxtReplaceMonth(inputPath: String): NativeCallResult =
        initializeRuntime()

    override suspend fun clearAndReinitialize(): ClearAndInitResult = ClearAndInitResult(
        initialized = true,
        operationOk = true,
        clearMessage = "ok",
        initResponse = """{"ok":true}"""
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
        preferredTxtPath: String?
    ): RecordActionResult = RecordActionResult(
        ok = true,
        message = "ok"
    )

    override suspend fun syncLiveToDatabase(): NativeCallResult = initializeRuntime()

    override suspend fun clearTxt(): ClearTxtResult = ClearTxtResult(
        ok = true,
        message = "ok"
    )

    override suspend fun reportDayMarkdown(date: String): ReportCallResult = emptyReportResult()

    override suspend fun reportMonthMarkdown(month: String): ReportCallResult = emptyReportResult()

    override suspend fun reportYearMarkdown(year: String): ReportCallResult = emptyReportResult()

    override suspend fun reportWeekMarkdown(week: String): ReportCallResult = emptyReportResult()

    override suspend fun reportRecentMarkdown(days: String): ReportCallResult = emptyReportResult()

    override suspend fun reportRange(startDate: String, endDate: String): ReportCallResult = emptyReportResult()

    override suspend fun queryActivitySuggestions(
        lookbackDays: Int,
        topN: Int
    ): ActivitySuggestionResult = ActivitySuggestionResult(
        ok = true,
        suggestions = listOf("meal"),
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
        ReportChartQueryResult(
            ok = true,
            data = ReportChartData(
                roots = emptyList(),
                selectedRoot = "",
                lookbackDays = params.lookbackDays,
                points = emptyList()
            ),
            message = "ok"
        )

    override suspend fun listActivityMappingNames(): ActivityMappingNamesResult = mappingNamesResult

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

    override suspend fun saveTxtFileAndSync(relativePath: String, content: String): RecordActionResult =
        RecordActionResult(
            ok = true,
            message = "ok"
        )

    override suspend fun listConfigTomlFiles(): ConfigTomlListResult = ConfigTomlListResult(
        ok = true,
        converterFiles = emptyList(),
        reportFiles = emptyList(),
        message = "ok"
    )

    override suspend fun readConfigTomlFile(relativePath: String): TxtFileContentResult = TxtFileContentResult(
        ok = true,
        filePath = relativePath,
        content = "",
        message = "ok"
    )

    override suspend fun saveConfigTomlFile(
        relativePath: String,
        content: String
    ): TxtFileContentResult = TxtFileContentResult(
        ok = true,
        filePath = relativePath,
        content = content,
        message = "ok"
    )

    override suspend fun listRecentDiagnostics(limit: Int): RuntimeDiagnosticsListResult =
        RuntimeDiagnosticsListResult(
            ok = true,
            entries = emptyList(),
            message = "ok",
            diagnosticsLogPath = ""
        )

    override suspend fun buildDiagnosticsPayload(maxEntries: Int): RuntimeDiagnosticsPayloadResult =
        RuntimeDiagnosticsPayloadResult(
            ok = true,
            payload = "",
            message = "ok",
            entryCount = 0,
            diagnosticsLogPath = ""
        )

    private fun emptyReportResult(): ReportCallResult = ReportCallResult(
        initialized = true,
        operationOk = true,
        outputText = "",
        rawResponse = """{"ok":true}"""
    )
}
