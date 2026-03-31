package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class RecordStateReducerTest {
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

}

private fun buildRecordViewModel(): RecordViewModel =
    RecordViewModel(
        recordUseCases = RecordUseCases(
            recordGateway = ReducerTestRecordGateway(),
            txtStorageGateway = ReducerTestTxtStorageGateway(),
            queryGateway = ReducerTestQueryGateway()
        )
    )

private fun timeMillis(instantIso: String): Long =
    java.time.Instant.parse(instantIso).toEpochMilli()

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
        topN: Int
    ): ActivitySuggestionResult = ActivitySuggestionResult(ok = true, suggestions = emptyList(), message = "ok")

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
