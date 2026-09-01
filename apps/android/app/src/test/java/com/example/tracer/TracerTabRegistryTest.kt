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
import java.time.Clock
import java.time.Instant
import java.time.ZoneId

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
    fun registry_contains_primary_tabs_in_expected_order() {
        val ids = TracerTabRegistry.entries.map { it.meta.id }
        assertEquals(
            listOf(
                TracerTab.FILES,
                TracerTab.INSIGHTS,
                TracerTab.RECORD,
                TracerTab.SETTINGS
            ),
            ids
        )
    }

    @Test
    fun settings_status_uses_data_status() {
        val status = TracerTabRegistry.statusText(
            tab = TracerTab.SETTINGS,
            args = TracerTabStatusArgs(
                dataStatusText = "Database rebuilt from TXT.",
                queryStatusText = "",
                recordStatusText = ""
            )
        )

        assertEquals("Database rebuilt from TXT.", status)
    }

    @Test
    fun onEnter_record_refreshes_authorable_tokens_and_clears_unavailable_status() = runTest(dispatcher) {
        val runtime = FakeRuntimeServices(
            mappingNamesResult = ActivityMappingNamesResult(
                ok = true,
                names = listOf("meal", "study", "起床"),
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
        advanceUntilIdle()

        recordViewModel.setStatusText("Activity authorable token validation unavailable: stale")
        var capturedNames: Set<String> = emptySet()

        TracerTabRegistry.onEnter(
            tab = TracerTab.RECORD,
            args = TracerTabLifecycleArgs(
                queryGateway = runtime,
                queryInsightsViewModel = QueryInsightsViewModel(
                    insightsGateway = runtime,
                    queryGateway = runtime
                ),
                recordViewModel = recordViewModel,
                recordStatusText = { recordViewModel.uiState.statusText },
                onValidAuthorableEventTokensChanged = { capturedNames = it }
            )
        )

        assertEquals(setOf("meal", "study", "起床"), capturedNames)
        assertEquals("", recordViewModel.uiState.statusText)
    }

    @Test
    fun onLeave_config_discards_unsaved_txt_editor_draft() = runTest(dispatcher) {
        val runtime = FakeRuntimeServices(
            inspectionEntries = listOf(
                TxtInspectionEntry(
                    relativePath = "draft.txt",
                    headerMonth = "2026-04",
                    expectedCanonicalRelativePath = "draft.txt",
                    syncState = TxtSyncState.SYNCED,
                    canOpen = true,
                    message = "ok"
                )
            ),
            readTxtContents = mapOf("draft.txt" to "saved-content")
        )
        val recordViewModel = RecordViewModel(
            RecordUseCases(
                recordGateway = runtime,
                txtStorageGateway = runtime,
                queryGateway = runtime
            )
        )
        advanceUntilIdle()

        recordViewModel.openHistoryFile("draft.txt")
        advanceUntilIdle()
        recordViewModel.updateEditableHistoryContent("draft-content")
        assertEquals("draft-content", recordViewModel.uiState.editableHistoryContent)

        TracerTabRegistry.onLeave(
            tab = TracerTab.FILES,
            args = TracerTabLifecycleArgs(
                queryGateway = runtime,
                queryInsightsViewModel = QueryInsightsViewModel(
                    insightsGateway = runtime,
                    queryGateway = runtime
                ),
                recordViewModel = recordViewModel,
                recordStatusText = { recordViewModel.uiState.statusText },
                onValidAuthorableEventTokensChanged = {}
            )
        )

        assertEquals(recordViewModel.uiState.selectedHistoryContent, recordViewModel.uiState.editableHistoryContent)
    }

    @Test
    fun statusEvent_insights_query_data_status_is_suppressed() {
        val event = TracerTabRegistry.statusEvent(
            tab = TracerTab.INSIGHTS,
            args = TracerTabStatusEventArgs(
                selectedTab = TracerTab.INSIGHTS,
                statusText = "query data insights-chart -> OK=true",
                lastObservedTab = TracerTab.INSIGHTS,
                lastObservedStatus = "query data insights-chart running..."
            )
        )

        assertNull(event)
    }

    @Test
    fun statusEvent_insights_markdown_generation_status_is_suppressed() {
        val event = TracerTabRegistry.statusEvent(
            tab = TracerTab.INSIGHTS,
            args = TracerTabStatusEventArgs(
                selectedTab = TracerTab.INSIGHTS,
                statusText = "nativeInsightsJson(Day, md) -> OK=true",
                lastObservedTab = TracerTab.INSIGHTS,
                lastObservedStatus = "nativeInsightsJson(Day, md) running..."
            )
        )

        assertNull(event)
    }

    @Test
    fun statusEvent_record_success_uses_structured_snackbar_visuals() {
        val event = TracerTabRegistry.statusEvent(
            tab = TracerTab.RECORD,
            args = TracerTabStatusEventArgs(
                selectedTab = TracerTab.RECORD,
                statusText = "routine_toilet\n13h 57m",
                lastObservedTab = TracerTab.RECORD,
                lastObservedStatus = "previous"
            )
        )

        require(event is TracerTabUiEvent.ShowSnackbar)
        assertEquals("routine_toilet", event.visuals.message)
        assertEquals("13h 57m", event.visuals.supportingText)
        assertEquals(SnackbarDuration.Short, event.visuals.duration)
    }

    @Test
    fun onEnter_insights_refreshes_day_parameter_to_current_logical_day() = runTest(dispatcher) {
        val runtime = FakeRuntimeServices()
        val queryInsightsViewModel = QueryInsightsViewModel(
            insightsGateway = runtime,
            queryGateway = runtime,
            clock = fixedClock("2026-03-29T21:30:00Z", "Asia/Shanghai")
        )
        val recordViewModel = RecordViewModel(
            RecordUseCases(
                recordGateway = runtime,
                txtStorageGateway = runtime,
                queryGateway = runtime
            )
        )
        queryInsightsViewModel.onInsightsDateChange("20260330")

        TracerTabRegistry.onEnter(
            tab = TracerTab.INSIGHTS,
            args = TracerTabLifecycleArgs(
                queryGateway = runtime,
                queryInsightsViewModel = queryInsightsViewModel,
                recordViewModel = recordViewModel,
                recordStatusText = { recordViewModel.uiState.statusText },
                onValidAuthorableEventTokensChanged = {}
            )
        )

        assertEquals("20260329", queryInsightsViewModel.uiState.insightsDate)
    }

    @Test
    fun clearDataAndReinitialize_uses_user_facing_success_text() = runTest(dispatcher) {
        val runtime = FakeRuntimeServices(
            clearAndInitResult = ClearAndInitResult(
                initialized = true,
                operationOk = true,
                clearMessage = "clear -> removed /data/user/0/...",
                initResponse = """{"content":""}"""
            )
        )
        val viewModel = DataViewModel(runtime, runtime)
        advanceUntilIdle()

        viewModel.clearDataAndReinitialize(
            DestructiveActionStatusText(
                running = "Clearing activity data...",
                success = "Activity data cleared.",
                failure = "Could not clear activity data."
            )
        )
        advanceUntilIdle()

        assertTrue(viewModel.uiState.initialized)
        assertEquals("Activity data cleared.", viewModel.uiState.statusText)
    }

    @Test
    fun clearDataAndReinitialize_uses_user_facing_failure_text() = runTest(dispatcher) {
        val runtime = FakeRuntimeServices(
            clearAndInitResult = ClearAndInitResult(
                initialized = false,
                operationOk = false,
                clearMessage = "clear -> failed",
                initResponse = """{"error":"internal details"}"""
            )
        )
        val viewModel = DataViewModel(runtime, runtime)
        advanceUntilIdle()

        viewModel.clearDataAndReinitialize(
            DestructiveActionStatusText(
                running = "Clearing activity data...",
                success = "Activity data cleared.",
                failure = "Could not clear activity data."
            )
        )
        advanceUntilIdle()

        assertEquals("Could not clear activity data.", viewModel.uiState.statusText)
    }

    @Test
    fun clearDatabase_uses_user_facing_success_text() = runTest(dispatcher) {
        val runtime = FakeRuntimeServices(
            clearDatabaseResult = ClearDatabaseResult(
                ok = true,
                message = "clear database -> removed /data/user/0/..."
            )
        )
        val viewModel = DataViewModel(runtime, runtime)
        advanceUntilIdle()

        viewModel.clearDatabase(
            DestructiveActionStatusText(
                running = "Clearing database...",
                success = "Database cleared.",
                failure = "Could not clear database."
            )
        )
        advanceUntilIdle()

        assertEquals("Database cleared.", viewModel.uiState.statusText)
    }

    @Test
    fun clearTxt_uses_user_facing_failure_text() = runTest(dispatcher) {
        val runtime = FakeRuntimeServices(
            clearTxtResult = ClearTxtResult(
                ok = false,
                message = "clear txt -> failed with internal details"
            )
        )
        val viewModel = DataViewModel(runtime, runtime)
        advanceUntilIdle()

        viewModel.clearTxt(
            DestructiveActionStatusText(
                running = "Clearing TXT files...",
                success = "TXT files cleared.",
                failure = "Could not clear TXT files."
            )
        )
        advanceUntilIdle()

        assertEquals("Could not clear TXT files.", viewModel.uiState.statusText)
    }
}

private class FakeRuntimeServices(
    private val mappingNamesResult: ActivityMappingNamesResult = ActivityMappingNamesResult(
        ok = true,
        names = emptyList(),
        message = "ok"
    ),
    private val inspectionEntries: List<TxtInspectionEntry> = emptyList(),
    private val readTxtContents: Map<String, String> = emptyMap(),
    private val clearAndInitResult: ClearAndInitResult = ClearAndInitResult(
        initialized = true,
        operationOk = true,
        clearMessage = "ok",
        initResponse = """{"ok":true}"""
    ),
    private val clearDatabaseResult: ClearDatabaseResult = ClearDatabaseResult(
        ok = true,
        message = "ok"
    ),
    private val clearTxtResult: ClearTxtResult = ClearTxtResult(
        ok = true,
        message = "ok"
    )
) : RuntimeInitializer,
    RecordGateway,
    InsightsGateway,
    TxtStorageGateway,
    QueryGateway,
    ConfigGateway {
    override suspend fun initializeRuntime(): NativeCallResult = NativeCallResult(
        initialized = true,
        operationOk = true,
        rawResponse = """{"ok":true}"""
    )

    override suspend fun ingestSingleTxtReplaceMonth(inputPath: String): NativeCallResult =
        initializeRuntime()

    override suspend fun clearAndReinitialize(): ClearAndInitResult = clearAndInitResult

    override suspend fun clearDatabase(): ClearDatabaseResult = clearDatabaseResult

    override suspend fun rebuildDatabase(): NativeCallResult = initializeRuntime()

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
    ): RecordActionResult = RecordActionResult(
        ok = true,
        message = "ok"
    )

    override suspend fun recordInterval(
        activityName: String,
        startTime: String,
        endTime: String,
        remark: String,
        targetDateIso: String?,
        preferredTxtPath: String?
    ): RecordActionResult = RecordActionResult(
        ok = true,
        message = "ok"
    )

    override suspend fun syncLiveToDatabase(): NativeCallResult = initializeRuntime()

    override suspend fun clearTxt(): ClearTxtResult = clearTxtResult

    override suspend fun queryFrequentActivities(
        lookbackDays: Int,
        topN: Int,
        anchorDateIso: String?
    ): ActivityFrequentResult = ActivityFrequentResult(
        ok = true,
        frequentActivities = listOf("meal"),
        message = "ok"
    )

    override suspend fun queryDayDurations(params: DataDurationQueryParams): DataQueryTextResult =
        DataQueryTextResult(ok = true, outputText = "", message = "ok")

    override suspend fun queryDayDurationStats(params: DataDurationQueryParams): DataQueryTextResult =
        DataQueryTextResult(ok = true, outputText = "", message = "ok")

    override suspend fun queryProjectTree(params: DataTreeQueryParams): TreeQueryResult =
        TreeQueryResult(ok = true, found = false, message = "ok")

    override suspend fun queryInsightsChart(params: InsightsChartQueryParams): InsightsChartQueryResult =
        InsightsChartQueryResult(
            ok = true,
            data = InsightsChartData(
                roots = emptyList(),
                selectedRoot = "",
                lookbackDays = params.lookbackDays,
                points = emptyList()
            ),
            message = "ok"
        )

    override suspend fun insightsMarkdown(request: TemporalInsightsQueryRequest): InsightsCallResult =
        InsightsCallResult(
            initialized = true,
            operationOk = true,
            outputText = "",
            rawResponse = ""
        )

    override suspend fun listActivityMappingNames(): ActivityMappingNamesResult = mappingNamesResult

    override suspend fun listAuthorableEventTokens(): ActivityMappingNamesResult = mappingNamesResult

    override suspend fun listTxtFiles(): TxtHistoryListResult = TxtHistoryListResult(
        ok = true,
        files = emptyList(),
        message = "ok"
    )

    override suspend fun inspectTxtFiles(): TxtInspectionResult = TxtInspectionResult(
        ok = true,
        entries = inspectionEntries,
        message = "ok"
    )

    override suspend fun readTxtFile(relativePath: String): TxtFileContentResult = TxtFileContentResult(
        ok = true,
        filePath = relativePath,
        content = readTxtContents[relativePath].orEmpty(),
        message = "ok"
    )

    override suspend fun saveTxtFileAndSync(relativePath: String, content: String): RecordActionResult =
        RecordActionResult(
            ok = true,
            message = "ok"
        )

    override suspend fun listConfigTomlFiles(): ConfigTomlListResult = ConfigTomlListResult(
        ok = true,
        aliasFiles = emptyList(),
        chartFiles = emptyList(),
        metaFiles = emptyList(),
        insightsFiles = emptyList(),
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

    override suspend fun deleteConfigTomlFile(relativePath: String): TxtFileContentResult =
        TxtFileContentResult(
            ok = true,
            filePath = relativePath,
            content = "",
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
}

private fun fixedClock(instantIso: String, zoneId: String): Clock =
    Clock.fixed(Instant.parse(instantIso), ZoneId.of(zoneId))
