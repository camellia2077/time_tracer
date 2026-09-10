package com.example.tracer

import java.time.Clock
import java.time.Instant
import java.time.ZoneId
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.test.StandardTestDispatcher
import kotlinx.coroutines.test.advanceUntilIdle
import kotlinx.coroutines.test.resetMain
import kotlinx.coroutines.test.runTest
import kotlinx.coroutines.test.setMain
import org.junit.Assert.assertEquals
import org.junit.Test

@OptIn(ExperimentalCoroutinesApi::class)
class RecordViewModelTest {
    @Test
    fun saveHistoryFileAndSync_refreshesLatestActivityAfterTxtEdit() = runTest {
        Dispatchers.setMain(StandardTestDispatcher(testScheduler))
        try {
            val queryGateway = FakeLatestActivityQueryGateway()
            val txtStorageGateway = FakeTxtStorageGateway(queryGateway)
            val viewModel = RecordViewModel(
                RecordUseCases(
                    recordGateway = UnusedRecordGateway,
                    txtStorageGateway = txtStorageGateway,
                    queryGateway = queryGateway,
                    clock = Clock.fixed(
                        Instant.parse("2026-09-10T12:00:00Z"),
                        ZoneId.of("Asia/Shanghai")
                    )
                )
            )
            advanceUntilIdle()
            assertEquals("newest", viewModel.uiState.latestActivityRecord?.activity)

            viewModel.openHistoryFile("2026/2026-09.txt")
            advanceUntilIdle()
            viewModel.updateEditableHistoryContent("y2026\nm09\nd0910\n090000older\n")
            viewModel.saveHistoryFileAndSync()
            advanceUntilIdle()

            assertEquals("older", viewModel.uiState.latestActivityRecord?.activity)
            assertEquals(1, txtStorageGateway.saveCount)
        } finally {
            Dispatchers.resetMain()
        }
    }
}

private class FakeLatestActivityQueryGateway : QueryGateway {
    var latestActivity: LatestActivityRecord? = LatestActivityRecord(
        dateIso = "2026-09-10",
        activity = "newest",
        recordKind = "point",
        startTime = "",
        endTime = "09:00:00",
        durationSeconds = 0
    )

    override suspend fun queryPreviousActivityTail(targetDateIso: String): PreviousActivityTailResult =
        PreviousActivityTailResult(ok = true, found = false, message = "none")

    override suspend fun queryLatestActivityRecord(targetDateIso: String): LatestActivityRecordResult =
        LatestActivityRecordResult(
            ok = true,
            found = latestActivity != null,
            record = latestActivity,
            message = "ok"
        )

    override suspend fun queryFrequentActivities(
        lookbackDays: Int,
        topN: Int,
        anchorDateIso: String?
    ): ActivityFrequentResult = error("not called")

    override suspend fun queryDayDurations(params: DataDurationQueryParams): DataQueryTextResult =
        error("not called")

    override suspend fun queryDayDurationStats(params: DataDurationQueryParams): DataQueryTextResult =
        error("not called")

    override suspend fun queryProjectTree(params: DataTreeQueryParams): TreeQueryResult =
        error("not called")

    override suspend fun queryInsightsChart(params: InsightsChartQueryParams): InsightsChartQueryResult =
        error("not called")

    override suspend fun listActivityMappingNames(): ActivityMappingNamesResult = error("not called")
}

private class FakeTxtStorageGateway(
    private val queryGateway: FakeLatestActivityQueryGateway
) : TxtStorageGateway {
    var saveCount: Int = 0
        private set

    override suspend fun inspectTxtFiles(): TxtInspectionResult = TxtInspectionResult(
        ok = true,
        entries = listOf(
            TxtInspectionEntry(
                relativePath = "2026/2026-09.txt",
                headerMonth = "2026-09",
                expectedCanonicalRelativePath = "2026/2026-09.txt",
                syncState = TxtSyncState.SYNCED,
                canOpen = true,
                message = "ok"
            )
        ),
        message = "ok"
    )

    override suspend fun listTxtFiles(): TxtHistoryListResult = TxtHistoryListResult(
        ok = true,
        files = listOf("2026/2026-09.txt"),
        message = "ok"
    )

    override suspend fun readTxtFile(relativePath: String): TxtFileContentResult = TxtFileContentResult(
        ok = true,
        filePath = relativePath,
        content = "y2026\nm09\nd0910\n090000newest\n",
        message = "ok"
    )

    override suspend fun saveTxtFileAndSync(
        relativePath: String,
        content: String
    ): RecordActionResult {
        saveCount += 1
        queryGateway.latestActivity = LatestActivityRecord(
            dateIso = "2026-09-10",
            activity = "older",
            recordKind = "point",
            startTime = "",
            endTime = "09:00:00",
            durationSeconds = 0
        )
        return RecordActionResult(ok = true, message = "saved")
    }
}

private object UnusedRecordGateway : RecordGateway {
    override suspend fun createCurrentMonthTxt(): RecordActionResult = error("not called")

    override suspend fun createMonthTxt(month: String): RecordActionResult = error("not called")

    override suspend fun recordNow(
        activityName: String,
        remark: String,
        targetDateIso: String?,
        preferredTxtPath: String?,
        timeOrderMode: RecordTimeOrderMode
    ): RecordActionResult = error("not called")

    override suspend fun recordInterval(
        activityName: String,
        startTime: String,
        endTime: String,
        remark: String,
        targetDateIso: String?,
        preferredTxtPath: String?
    ): RecordActionResult = error("not called")

    override suspend fun syncLiveToDatabase(): NativeCallResult = error("not called")

    override suspend fun clearTxt(): ClearTxtResult = error("not called")
}
