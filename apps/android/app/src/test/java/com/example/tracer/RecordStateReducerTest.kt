package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import java.time.Clock
import java.time.Instant
import java.time.ZoneId

class RecordStateReducerTest {
    @Test
    fun recordInput_defaultsToIntervalMode() {
        assertEquals(RecordAuthoringMode.INTERVAL, RecordUiState().authoringMode)
    }

    @Test
    fun persistedAllTxtOutputMode_overridesFirstOpenDayDefault() {
        val viewModel = buildRecordViewModel()

        viewModel.hydratePersistedRecordInput(
            PersistedRecordInputSnapshot(lastTxtOutputMode = TxtOutputMode.ALL)
        )

        assertEquals(TxtOutputMode.ALL, viewModel.uiState.txtOutputMode)
    }

    @Test
    fun updateQuickActivities_allowsEmptyList() {
        val viewModel = RecordViewModel(
            recordUseCases = RecordUseCases(
                recordGateway = ReducerTestRecordGateway(),
                txtStorageGateway = ReducerTestTxtStorageGateway(),
                queryGateway = ReducerTestQueryGateway()
            )
        )
        viewModel.updateQuickActivities(listOf("meal", "legacy-default"))

        viewModel.updateQuickActivities(emptyList())

        assertEquals(emptyList<String>(), viewModel.uiState.quickActivities)
    }

    @Test
    fun refreshLogicalDayDefault_beforeCutoff_selectsYesterday() {
        val viewModel = buildRecordViewModel()

        viewModel.refreshLogicalDayDefault(timeMillis("2026-03-29T21:59:00Z"))

        assertEquals(RecordLogicalDayTarget.YESTERDAY, viewModel.uiState.logicalDayTarget)
        assertFalse(viewModel.uiState.logicalDayIsUserOverride)
    }

    @Test
    fun refreshLogicalDayDefault_atCutoff_selectsToday() {
        val viewModel = buildRecordViewModel()

        viewModel.refreshLogicalDayDefault(timeMillis("2026-03-29T22:00:00Z"))

        assertEquals(RecordLogicalDayTarget.TODAY, viewModel.uiState.logicalDayTarget)
        assertFalse(viewModel.uiState.logicalDayIsUserOverride)
    }

    @Test
    fun refreshLogicalDayDefault_keepsUserOverrideAcrossCutoff() {
        val viewModel = buildRecordViewModel()
        viewModel.selectLogicalDayYesterday()

        viewModel.refreshLogicalDayDefault(timeMillis("2026-03-29T22:00:00Z"))

        assertEquals(RecordLogicalDayTarget.YESTERDAY, viewModel.uiState.logicalDayTarget)
        assertTrue(viewModel.uiState.logicalDayIsUserOverride)
    }

    @Test
    fun authoringModeAndIntervalFields_updateState() {
        val viewModel = buildRecordViewModel()

        viewModel.onAuthoringModeChange(RecordAuthoringMode.INTERVAL)
        viewModel.onIntervalStartChange("0900")
        viewModel.onIntervalEndChange("1030")

        assertEquals(RecordAuthoringMode.INTERVAL, viewModel.uiState.authoringMode)
        assertEquals("0900", viewModel.uiState.intervalStart)
        assertEquals("1030", viewModel.uiState.intervalEnd)
    }

    @Test
    fun suggestionOutputMode_updatesState() {
        val viewModel = buildRecordViewModel()

        viewModel.updateSuggestionOutputMode(RecordSuggestionOutputMode.ALIAS)

        assertEquals(RecordSuggestionOutputMode.ALIAS, viewModel.uiState.suggestionOutputMode)
    }

    @Test
    fun canonicalCatalogDisplayMode_updatesState() {
        val viewModel = buildRecordViewModel()

        viewModel.updateCanonicalCatalogDisplayMode(RecordSuggestionOutputMode.ALIAS)

        assertEquals(
            RecordSuggestionOutputMode.ALIAS,
            viewModel.uiState.canonicalCatalogDisplayMode
        )
    }

    @Test
    fun applyCanonicalCatalogEntry_insertsCanonicalPathAndDismissesOverlays() {
        val viewModel = buildRecordViewModel()
        viewModel.toggleSuggestions()
        viewModel.openCanonicalCatalog()

        viewModel.applyCanonicalCatalogEntry("study/math/calculus")

        assertEquals("study/math/calculus", viewModel.uiState.recordContent)
        assertFalse(viewModel.uiState.suggestionsVisible)
        assertFalse(viewModel.uiState.isCanonicalCatalogVisible)
    }

    @Test
    fun applyCanonicalCatalogEntry_insertsAliasTokenAndDismissesOverlays() {
        val viewModel = buildRecordViewModel()
        viewModel.toggleSuggestions()
        viewModel.openCanonicalCatalog()

        viewModel.applyCanonicalCatalogEntry("高数")

        assertEquals("高数", viewModel.uiState.recordContent)
        assertFalse(viewModel.uiState.suggestionsVisible)
        assertFalse(viewModel.uiState.isCanonicalCatalogVisible)
    }

    @Test
    fun refreshLogicalDayDefault_keepsDraftTargetAcrossCutoff() {
        val viewModel = buildRecordViewModel()
        viewModel.onRecordContentChange("study")
        viewModel.selectLogicalDayYesterday()

        viewModel.refreshLogicalDayDefault(timeMillis("2026-03-29T22:00:00Z"))

        assertEquals(RecordLogicalDayTarget.YESTERDAY, viewModel.uiState.logicalDayTarget)
        assertEquals("study", viewModel.uiState.recordContent)
    }

}

private fun buildRecordViewModel(): RecordViewModel =
    RecordViewModel(
        recordUseCases = RecordUseCases(
            recordGateway = ReducerTestRecordGateway(),
            txtStorageGateway = ReducerTestTxtStorageGateway(),
            queryGateway = ReducerTestQueryGateway(),
            // Pin the reducer/view-model logical-day tests to an explicit device-local zone so
            // host CI runners cannot change the before/after-cutoff expectations.
            clock = fixedClock("2026-03-30T00:00:00Z", "Asia/Shanghai")
        )
    )

private fun timeMillis(instantIso: String): Long =
    java.time.Instant.parse(instantIso).toEpochMilli()

private fun fixedClock(instantIso: String, zoneId: String): Clock =
    Clock.fixed(Instant.parse(instantIso), ZoneId.of(zoneId))

private class ReducerTestRecordGateway : RecordGateway {
    override suspend fun createCurrentMonthTxt(): RecordActionResult = RecordActionResult(true, "ok")

    override suspend fun createMonthTxt(month: String): RecordActionResult = RecordActionResult(true, "ok")

    override suspend fun recordNow(
        activityName: String,
        remark: String,
        targetDateIso: String?,
        preferredTxtPath: String?,
        timeOrderMode: RecordTimeOrderMode
    ): RecordActionResult = RecordActionResult(true, "ok")

    override suspend fun recordInterval(
        activityName: String,
        startTime: String,
        endTime: String,
        remark: String,
        targetDateIso: String?,
        preferredTxtPath: String?
    ): RecordActionResult = RecordActionResult(true, "ok")

    override suspend fun syncLiveToDatabase(): NativeCallResult =
        NativeCallResult(initialized = true, operationOk = true, rawResponse = """{"ok":true}""")

    override suspend fun clearTxt(): ClearTxtResult = ClearTxtResult(true, "ok")
}

private class ReducerTestTxtStorageGateway : TxtStorageGateway {
    override suspend fun inspectTxtFiles(): TxtInspectionResult =
        TxtInspectionResult(ok = true, entries = emptyList(), message = "ok")

    override suspend fun listTxtFiles(): TxtHistoryListResult =
        TxtHistoryListResult(ok = true, files = emptyList(), message = "ok")

    override suspend fun readTxtFile(relativePath: String): TxtFileContentResult =
        TxtFileContentResult(ok = false, filePath = relativePath, content = "", message = "not found")

    override suspend fun saveTxtFileAndSync(relativePath: String, content: String): RecordActionResult =
        RecordActionResult(ok = true, message = "ok")
}

private class ReducerTestQueryGateway : QueryGateway {
    override suspend fun queryActivitySuggestions(
        lookbackDays: Int,
        topN: Int,
        anchorDateIso: String?
    ): ActivitySuggestionResult = ActivitySuggestionResult(ok = true, suggestions = emptyList(), message = "ok")

    override suspend fun queryDayDurations(params: DataDurationQueryParams): DataQueryTextResult =
        DataQueryTextResult(ok = true, outputText = "", message = "ok")

    override suspend fun queryDayDurationStats(params: DataDurationQueryParams): DataQueryTextResult =
        DataQueryTextResult(ok = true, outputText = "", message = "ok")

    override suspend fun queryProjectTree(params: DataTreeQueryParams): TreeQueryResult =
        TreeQueryResult(ok = true, found = false, message = "ok")

    override suspend fun queryReportChart(params: ReportChartQueryParams): ReportChartQueryResult =
        ReportChartQueryResult(ok = true, data = null, message = "ok")

    override suspend fun listActivityMappingNames(): ActivityMappingNamesResult =
        ActivityMappingNamesResult(ok = true, names = emptyList(), message = "ok")

    override suspend fun listActivityAliasMappings(): ActivityAliasMappingListResult =
        ActivityAliasMappingListResult(ok = true, entries = emptyList(), message = "ok")

    override suspend fun listCanonicalCatalog(): CanonicalCatalogResult =
        CanonicalCatalogResult(ok = true, roots = emptyList(), entries = emptyList(), message = "ok")
}
