@file:Suppress("LargeClass")

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
                        content = "y2026\nm03\nd0301\n",
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
    fun recordNow_showsUserFacingSuccessStatusAfterInsert() = runTest {
        val useCases = RecordUseCases(
            recordGateway = FakeRecordGateway(
                recordNowResult = RecordActionResult(
                    ok = true,
                    message = "record: ok\n" +
                        "sync: ok\n" +
                        "gap_from_previous: 00:25\n" +
                        "target_file: /data/user/0/app/files/2026-03.txt"
                )
            ),
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
            insightsGateway = FakeInsightsGateway(
                activityName = "coding",
                durationSeconds = 1500L
            )
        )

        val result = useCases.recordNow(
            RecordUiState(
                recordContent = "coding",
                selectedMonth = "2026-03"
            )
        )

        assertEquals("coding\n25m", result.statusText)
        assertEquals("coding", result.lastRecordedActivityHierarchyLeaf)
        assertEquals("00:25", result.lastRecordedDuration)
    }

    @Test
    fun recordNow_wakeUsesDatabaseSleepDuration() = runTest {
        val useCases = RecordUseCases(
            recordGateway = FakeRecordGateway(
                recordNowResult = RecordActionResult(
                    ok = true,
                    message = "record: ok\ngap_from_previous: 34:00"
                )
            ),
            txtStorageGateway = FakeTxtStorageGateway(
                inspectionResult = TxtInspectionResult(
                    ok = true,
                    entries = listOf(inspectionEntry("2026/2026-08.txt", "2026-08")),
                    message = "ok"
                ),
                readResults = mapOf(
                    "2026/2026-08.txt" to txtReadResult("2026/2026-08.txt")
                )
            ),
            queryGateway = FakeQueryGateway(
                wakeKeywordsResult = ActivityMappingNamesResult(
                    ok = true,
                    names = listOf("w"),
                    message = "ok"
                )
            ),
            insightsGateway = FakeInsightsGateway(
                activityName = "sleep_night",
                durationSeconds = 9 * 60 * 60L
            )
        )

        val result = useCases.recordNow(
            RecordUiState(
                recordContent = "w",
                selectedMonth = "2026-08"
            )
        )

        assertEquals("w\n9h", result.statusText)
        assertEquals("09:00", result.lastRecordedDuration)
    }

    @Test
    fun recordNow_keepsActivityAndRemarkAfterFailedInsert() = runTest {
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
                        content = "y2026\nm03\nd0301\n",
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

        assertEquals("coding", result.recordContent)
        assertEquals("ship atomic record", result.recordRemark)
        assertEquals("record: failed", result.statusText)
    }

    @Test
    fun recordInterval_clearsActivityRemarkAndTimesAfterInsert() = runTest {
        val inspectionEntries = listOf(
            inspectionEntry("2026/2026-03.txt", "2026-03")
        )
        val gateway = FakeRecordGateway(
            recordIntervalResult = RecordActionResult(
                ok = true,
                message = "record interval: ok"
            )
        )
        val useCases = RecordUseCases(
            recordGateway = gateway,
            txtStorageGateway = FakeTxtStorageGateway(
                inspectionResult = TxtInspectionResult(
                    ok = true,
                    entries = inspectionEntries,
                    message = "ok"
                ),
                readResults = mapOf(
                    "2026/2026-03.txt" to txtReadResult("2026/2026-03.txt")
                )
            ),
            queryGateway = FakeQueryGateway()
        )

        val result = useCases.recordInterval(
            RecordUiState(
                authoringMode = RecordAuthoringMode.INTERVAL,
                recordContent = "study",
                recordRemark = "focused block",
                intervalStart = "09:00:00",
                intervalEnd = "10:30:00",
                attributionDateIso = "2026-03-28",
                logicalDayTarget = RecordLogicalDayTarget.TODAY,
                selectedMonth = "2026-03"
            )
        )

        assertEquals("09:00:00", gateway.lastIntervalStart)
        assertEquals("10:30:00", gateway.lastIntervalEnd)
        assertEquals("2026-03-28", gateway.lastTargetDateIso)
        assertEquals("", result.recordContent)
        assertEquals("", result.recordRemark)
        assertEquals("", result.intervalStart)
        assertEquals("", result.intervalEnd)
    }

    @Test
    fun recordInterval_showsUserFacingSuccessStatusAfterInsert() = runTest {
        val useCases = RecordUseCases(
            recordGateway = FakeRecordGateway(
                recordIntervalResult = RecordActionResult(
                    ok = true,
                    message = "record: ok\nsync: ok\ntarget_file: /data/user/0/app/files/2026-03.txt"
                )
            ),
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
            queryGateway = FakeQueryGateway(
                aliasMappingsResult = ActivityHierarchyLeafMappingListResult(
                    ok = true,
                    entries = listOf(ActivityHierarchyLeafMappingEntry("学习", "study")),
                    message = "ok"
                )
            ),
            insightsGateway = FakeInsightsGateway(
                activityName = "study",
                durationSeconds = 5400L
            )
        )

        val result = useCases.recordInterval(
            RecordUiState(
                authoringMode = RecordAuthoringMode.INTERVAL,
                recordContent = "学习",
                intervalStart = "09:00:00",
                intervalEnd = "10:30:00",
                selectedMonth = "2026-03"
            )
        )

        assertEquals("study\n1h 30m", result.statusText)
        assertEquals("学习", result.lastRecordedActivityHierarchyLeaf)
        assertEquals("01:30", result.lastRecordedDuration)
    }

    @Test
    fun recordInterval_successStatusKeepsMinutesAndSeconds() = runTest {
        val useCases = RecordUseCases(
            recordGateway = FakeRecordGateway(
                recordIntervalResult = RecordActionResult(
                    ok = true,
                    message = "record: ok\nsync: ok"
                )
            ),
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
            insightsGateway = FakeInsightsGateway(
                activityName = "study",
                durationSeconds = 125L
            )
        )

        val result = useCases.recordInterval(
            RecordUiState(
                authoringMode = RecordAuthoringMode.INTERVAL,
                recordContent = "study",
                intervalStart = "09:00:01",
                intervalEnd = "09:02:06"
            )
        )

        assertEquals("study\n2m 5s", result.statusText)
        assertEquals("00:02:05", result.lastRecordedDuration)
    }

    @Test
    fun recordInterval_invalidIsoTimeReturnsStableStatus() = runTest {
        val useCases = RecordUseCases(
            recordGateway = FakeRecordGateway(),
            txtStorageGateway = FakeTxtStorageGateway(
                inspectionResult = TxtInspectionResult(ok = true, entries = emptyList(), message = "ok"),
                readResults = emptyMap()
            ),
            queryGateway = FakeQueryGateway()
        )

        val result = useCases.recordInterval(
            RecordUiState(
                authoringMode = RecordAuthoringMode.INTERVAL,
                recordContent = "study",
                intervalStart = "09:00:00",
                intervalEnd = "24:60:00"
            )
        )

        assertEquals("Record blocked: start/end must use ISO HH:mm:ss.", result.statusText)
    }

    @Test
    fun recordInterval_keepsDraftAfterFailedInsert() = runTest {
        val useCases = RecordUseCases(
            recordGateway = FakeRecordGateway(
                recordIntervalResult = RecordActionResult(
                    ok = false,
                    message = "record interval: failed"
                )
            ),
            txtStorageGateway = FakeTxtStorageGateway(
                inspectionResult = TxtInspectionResult(ok = true, entries = emptyList(), message = "ok"),
                readResults = emptyMap()
            ),
            queryGateway = FakeQueryGateway()
        )

        val result = useCases.recordInterval(
            RecordUiState(
                authoringMode = RecordAuthoringMode.INTERVAL,
                recordContent = "study",
                recordRemark = "focused block",
                intervalStart = "09:00:00",
                intervalEnd = "10:30:00"
            )
        )

        assertEquals("study", result.recordContent)
        assertEquals("focused block", result.recordRemark)
        assertEquals("09:00:00", result.intervalStart)
        assertEquals("10:30:00", result.intervalEnd)
        assertEquals("record interval: failed", result.statusText)
    }

    @Test
    fun loadFrequentActivities_anchorsToLogicalTargetDate() = runTest {
        val queryGateway = FakeQueryGateway()
        val useCases = RecordUseCases(
            recordGateway = FakeRecordGateway(),
            txtStorageGateway = FakeTxtStorageGateway(
                inspectionResult = TxtInspectionResult(ok = true, entries = emptyList(), message = "ok"),
                readResults = emptyMap()
            ),
            queryGateway = queryGateway,
            clock = fixedClock("2026-03-31T16:30:00Z", "Asia/Shanghai")
        )

        useCases.loadFrequentActivities(
            state = RecordUiState(logicalDayTarget = RecordLogicalDayTarget.YESTERDAY),
            lookbackDays = 7,
            topN = 5
        )

        assertEquals("2026-03-31", queryGateway.lastAnchorDateIso)
    }

    @Test
    fun loadFrequentActivities_keepsCanonicalAndAliasDisplayTokens() = runTest {
        val useCases = RecordUseCases(
            recordGateway = FakeRecordGateway(),
            txtStorageGateway = FakeTxtStorageGateway(
                inspectionResult = TxtInspectionResult(ok = true, entries = emptyList(), message = "ok"),
                readResults = emptyMap()
            ),
            queryGateway = FakeQueryGateway(
                activityFrequentResult = ActivityFrequentResult(
                    ok = true,
                    frequentActivities = listOf("recreation_game_clash-royale", "meal"),
                    message = "ok"
                ),
                aliasMappingsResult = ActivityHierarchyLeafMappingListResult(
                    ok = true,
                    entries = listOf(
                        ActivityHierarchyLeafMappingEntry("皇室战争", "recreation_game_clash-royale")
                    ),
                    message = "ok"
                )
            )
        )

        val result = useCases.loadFrequentActivities(
            state = RecordUiState(),
            lookbackDays = 7,
            topN = 5
        )

        assertEquals(
            listOf(
                RecordFrequentActivity(
                    canonicalToken = "recreation_game_clash-royale",
                    aliasToken = "皇室战争"
                ),
                RecordFrequentActivity(canonicalToken = "meal")
            ),
            result.frequentActivities
        )
    }

    @Test
    fun loadCanonicalCatalog_keepsFailureSeparateFromFrequentState() = runTest {
        val useCases = RecordUseCases(
            recordGateway = FakeRecordGateway(),
            txtStorageGateway = FakeTxtStorageGateway(
                inspectionResult = TxtInspectionResult(ok = true, entries = emptyList(), message = "ok"),
                readResults = emptyMap()
            ),
            queryGateway = FakeQueryGateway(
                activityFrequentResult = ActivityFrequentResult(
                    ok = true,
                    frequentActivities = listOf("meal"),
                    message = "suggest ok"
                ),
                canonicalCatalogResult = CanonicalCatalogResult(
                    ok = false,
                    roots = emptyList(),
                    entries = emptyList(),
                    message = "catalog failed"
                )
            )
        )

        val result = useCases.loadCanonicalCatalog(
            state = RecordUiState(),
        )

        assertEquals("catalog failed", result.canonicalCatalogStatusText)
        assertTrue(result.canonicalCatalogRoots.isEmpty())
        assertEquals("", result.statusText)
    }

    @Test
    fun loadCanonicalCatalog_populatesRootsAndClearsLoadingState() = runTest {
        val useCases = RecordUseCases(
            recordGateway = FakeRecordGateway(),
            txtStorageGateway = FakeTxtStorageGateway(
                inspectionResult = TxtInspectionResult(ok = true, entries = emptyList(), message = "ok"),
                readResults = emptyMap()
            ),
            queryGateway = FakeQueryGateway()
        )

        val result = useCases.loadCanonicalCatalog(
            state = RecordUiState(isCanonicalCatalogLoading = true)
        )

        assertEquals(false, result.isCanonicalCatalogLoading)
        assertEquals(true, result.isCanonicalCatalogVisible)
        assertEquals(
            listOf("study", "study/math"),
            result.canonicalCatalogRoots.first().let { root ->
                listOf(root.path, root.children.single().path)
            }
        )
    }

    @Test
    fun applyFrequentActivity_defaultsToCanonicalInsertMode() = runTest {
        val useCases = RecordUseCases(
            recordGateway = FakeRecordGateway(),
            txtStorageGateway = FakeTxtStorageGateway(
                inspectionResult = TxtInspectionResult(ok = true, entries = emptyList(), message = "ok"),
                readResults = emptyMap()
            ),
            queryGateway = FakeQueryGateway()
        )

        val result = useCases.applyFrequentActivity(
            state = RecordUiState(frequentActivitiesVisible = true),
            frequentActivityToken = "recreation_game_clash-royale"
        )

        assertEquals("recreation_game_clash-royale", result.recordContent)
        assertEquals(false, result.frequentActivitiesVisible)
        assertEquals("", result.statusText)
    }

    @Test
    fun applyFrequentActivity_resolvesClickedAliasTokenToCachedCanonicalFrequent() = runTest {
        val useCases = RecordUseCases(
            recordGateway = FakeRecordGateway(),
            txtStorageGateway = FakeTxtStorageGateway(
                inspectionResult = TxtInspectionResult(ok = true, entries = emptyList(), message = "ok"),
                readResults = emptyMap()
            ),
            queryGateway = FakeQueryGateway()
        )

        val result = useCases.applyFrequentActivity(
            state = RecordUiState(
                frequentActivitiesVisible = true,
                frequentOutputMode = RecordFrequentOutputMode.ALIAS,
                frequentActivities = listOf(
                    RecordFrequentActivity(
                        canonicalToken = "recreation_game_clash-royale",
                        aliasToken = "皇室战争"
                    )
                )
            ),
            frequentActivityToken = "皇室战争"
        )

        assertEquals("皇室战争", result.recordContent)
        assertEquals(false, result.frequentActivitiesVisible)
        assertEquals("", result.statusText)
    }

    @Test
    fun applyFrequentActivity_resolvesCanonicalToFirstDeclaredAliasWhenAliasModeEnabled() = runTest {
        val useCases = RecordUseCases(
            recordGateway = FakeRecordGateway(),
            txtStorageGateway = FakeTxtStorageGateway(
                inspectionResult = TxtInspectionResult(ok = true, entries = emptyList(), message = "ok"),
                readResults = emptyMap()
            ),
            queryGateway = FakeQueryGateway(
                aliasMappingsResult = ActivityHierarchyLeafMappingListResult(
                    ok = true,
                    entries = listOf(
                        ActivityHierarchyLeafMappingEntry("皇室战争", "recreation_game_clash-royale"),
                        ActivityHierarchyLeafMappingEntry("cr", "recreation_game_clash-royale"),
                        ActivityHierarchyLeafMappingEntry("Clash Royale", "recreation_game_clash-royale")
                    ),
                    message = "ok"
                )
            )
        )

        val result = useCases.applyFrequentActivity(
            state = RecordUiState(
                frequentActivitiesVisible = true,
                frequentOutputMode = RecordFrequentOutputMode.ALIAS
            ),
            frequentActivityToken = "recreation_game_clash-royale"
        )

        assertEquals("皇室战争", result.recordContent)
        assertEquals(false, result.frequentActivitiesVisible)
        assertEquals("", result.statusText)
    }

    @Test
    fun applyFrequentActivity_keepsStateWhenCanonicalHasNoAliasMapping() = runTest {
        val useCases = RecordUseCases(
            recordGateway = FakeRecordGateway(),
            txtStorageGateway = FakeTxtStorageGateway(
                inspectionResult = TxtInspectionResult(ok = true, entries = emptyList(), message = "ok"),
                readResults = emptyMap()
            ),
            queryGateway = FakeQueryGateway(
                aliasMappingsResult = ActivityHierarchyLeafMappingListResult(
                    ok = true,
                    entries = listOf(ActivityHierarchyLeafMappingEntry("快递", "routine_express")),
                    message = "ok"
                )
            )
        )

        val result = useCases.applyFrequentActivity(
            state = RecordUiState(
                recordContent = "existing",
                frequentActivitiesVisible = true,
                frequentOutputMode = RecordFrequentOutputMode.ALIAS
            ),
            frequentActivityToken = "recreation_game_clash-royale"
        )

        assertEquals("existing", result.recordContent)
        assertEquals(false, result.frequentActivitiesVisible)
        assertEquals(
            "Frequent activity unavailable for authoring: no alias mapped for recreation_game_clash-royale.",
            result.statusText
        )
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
                        content = "y2026\nm03\nd0301\n",
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
    fun recordNow_whenMonthRollsOver_doesNotSendStalePreferredTxtPath() = runTest {
        val gateway = FakeRecordGateway()
        val useCases = RecordUseCases(
            recordGateway = gateway,
            txtStorageGateway = FakeTxtStorageGateway(
                inspectionResult = TxtInspectionResult(
                    ok = true,
                    entries = listOf(
                        inspectionEntry("2026/2026-05.txt", "2026-05"),
                        inspectionEntry("2026/2026-06.txt", "2026-06")
                    ),
                    message = "ok"
                ),
                readResults = mapOf(
                    "2026/2026-06.txt" to txtReadResult("2026/2026-06.txt")
                )
            ),
            queryGateway = FakeQueryGateway(),
            clock = fixedClock("2026-05-31T17:30:00Z", "Asia/Shanghai")
        )

        val result = useCases.recordNow(
            RecordUiState(
                recordContent = "coding",
                logicalDayTarget = RecordLogicalDayTarget.TODAY,
                selectedMonth = "2026-05",
                selectedHistoryFile = "2026/2026-05.txt"
            )
        )

        assertEquals("2026-06-01", gateway.lastTargetDateIso)
        assertEquals(null, gateway.lastPreferredTxtPath)
        assertEquals("2026-06", result.selectedMonth)
        assertEquals("2026/2026-06.txt", result.selectedHistoryFile)
    }

    @Test
    fun recordNow_whenLogicalYesterdayTargetsPreviousMonth_keepsMatchingPreferredTxtPath() = runTest {
        val gateway = FakeRecordGateway()
        val useCases = RecordUseCases(
            recordGateway = gateway,
            txtStorageGateway = FakeTxtStorageGateway(
                inspectionResult = TxtInspectionResult(
                    ok = true,
                    entries = listOf(
                        inspectionEntry("2026/2026-05.txt", "2026-05"),
                        inspectionEntry("2026/2026-06.txt", "2026-06")
                    ),
                    message = "ok"
                ),
                readResults = mapOf(
                    "2026/2026-05.txt" to txtReadResult("2026/2026-05.txt")
                )
            ),
            queryGateway = FakeQueryGateway(),
            clock = fixedClock("2026-05-31T17:30:00Z", "Asia/Shanghai")
        )

        val result = useCases.recordNow(
            RecordUiState(
                recordContent = "coding",
                logicalDayTarget = RecordLogicalDayTarget.YESTERDAY,
                selectedMonth = "2026-05",
                selectedHistoryFile = "2026/2026-05.txt"
            )
        )

        assertEquals("2026-05-31", gateway.lastTargetDateIso)
        assertEquals("2026/2026-05.txt", gateway.lastPreferredTxtPath)
        assertEquals("2026-05", result.selectedMonth)
        assertEquals("2026/2026-05.txt", result.selectedHistoryFile)
        assertEquals("coding\nn/a", result.statusText)
    }

    @Test
    fun recordNow_ignoresNonCanonicalPreferredTxtPathEvenWhenMonthMatches() = runTest {
        val gateway = FakeRecordGateway()
        val useCases = RecordUseCases(
            recordGateway = gateway,
            txtStorageGateway = FakeTxtStorageGateway(
                inspectionResult = TxtInspectionResult(
                    ok = true,
                    entries = listOf(inspectionEntry("2026/2026-05.txt", "2026-05")),
                    message = "ok"
                ),
                readResults = mapOf(
                    "2026/2026-05.txt" to txtReadResult("2026/2026-05.txt")
                )
            ),
            queryGateway = FakeQueryGateway(),
            clock = fixedClock("2026-05-10T01:00:00Z", "Asia/Shanghai")
        )

        useCases.recordNow(
            RecordUiState(
                recordContent = "coding",
                logicalDayTarget = RecordLogicalDayTarget.TODAY,
                selectedMonth = "2026-05",
                selectedHistoryFile = "custom/2026-05.txt"
            )
        )

        assertEquals("2026-05-10", gateway.lastTargetDateIso)
        assertEquals(null, gateway.lastPreferredTxtPath)
    }

    @Test
    fun recordInterval_whenMonthRollsOver_doesNotSendStalePreferredTxtPath() = runTest {
        val gateway = FakeRecordGateway()
        val useCases = RecordUseCases(
            recordGateway = gateway,
            txtStorageGateway = FakeTxtStorageGateway(
                inspectionResult = TxtInspectionResult(
                    ok = true,
                    entries = listOf(
                        inspectionEntry("2026/2026-05.txt", "2026-05"),
                        inspectionEntry("2026/2026-06.txt", "2026-06")
                    ),
                    message = "ok"
                ),
                readResults = mapOf(
                    "2026/2026-06.txt" to txtReadResult("2026/2026-06.txt")
                )
            ),
            queryGateway = FakeQueryGateway(),
            clock = fixedClock("2026-05-31T17:30:00Z", "Asia/Shanghai")
        )

        val result = useCases.recordInterval(
            RecordUiState(
                authoringMode = RecordAuthoringMode.INTERVAL,
                recordContent = "study",
                intervalStart = "09:00:00",
                intervalEnd = "10:30:00",
                logicalDayTarget = RecordLogicalDayTarget.TODAY,
                selectedMonth = "2026-05",
                selectedHistoryFile = "2026/2026-05.txt"
            )
        )

        assertEquals("2026-06-01", gateway.lastTargetDateIso)
        assertEquals(null, gateway.lastPreferredTxtPath)
        assertEquals("2026-06", result.selectedMonth)
        assertEquals("2026/2026-06.txt", result.selectedHistoryFile)
    }

    @Test
    fun recordNow_whenYearRollsOver_doesNotSendStalePreferredTxtPath() = runTest {
        val gateway = FakeRecordGateway()
        val useCases = RecordUseCases(
            recordGateway = gateway,
            txtStorageGateway = FakeTxtStorageGateway(
                inspectionResult = TxtInspectionResult(
                    ok = true,
                    entries = listOf(
                        inspectionEntry("2026/2026-12.txt", "2026-12"),
                        inspectionEntry("2027/2027-01.txt", "2027-01")
                    ),
                    message = "ok"
                ),
                readResults = mapOf(
                    "2027/2027-01.txt" to txtReadResult("2027/2027-01.txt")
                )
            ),
            queryGateway = FakeQueryGateway(),
            clock = fixedClock("2026-12-31T17:30:00Z", "Asia/Shanghai")
        )

        val result = useCases.recordNow(
            RecordUiState(
                recordContent = "coding",
                logicalDayTarget = RecordLogicalDayTarget.TODAY,
                selectedMonth = "2026-12",
                selectedHistoryFile = "2026/2026-12.txt"
            )
        )

        assertEquals("2027-01-01", gateway.lastTargetDateIso)
        assertEquals(null, gateway.lastPreferredTxtPath)
        assertEquals("2027-01", result.selectedMonth)
        assertEquals("2027/2027-01.txt", result.selectedHistoryFile)
        assertEquals("coding\nn/a", result.statusText)
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
                        content = "y2026\nm04\nd0401\n",
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
        assertEquals("y2026\nm03\nd0301\n", result.selectedHistoryContent)
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

    @Test
    fun refreshHistory_whenCurrentMonthIsMissing_createsAndOpensCurrentMonthTxt() = runTest {
        val gateway = FakeRecordGateway()
        val initialInspection = TxtInspectionResult(
            ok = true,
            entries = listOf(inspectionEntry("2026/2026-04.txt", "2026-04")),
            message = "ok"
        )
        val createdInspection = TxtInspectionResult(
            ok = true,
            entries = listOf(
                inspectionEntry("2026/2026-04.txt", "2026-04"),
                inspectionEntry("2026/2026-05.txt", "2026-05")
            ),
            message = "ok"
        )
        val txtStorageGateway = FakeTxtStorageGateway(
            inspectionResult = initialInspection,
            readResults = mapOf(
                "2026/2026-05.txt" to TxtFileContentResult(
                    ok = true,
                    filePath = "2026/2026-05.txt",
                        content = "y2026\nm05\nd0501\n",
                    message = "Read TXT success."
                )
            ),
            inspectionResults = listOf(initialInspection, createdInspection)
        )
        val useCases = RecordUseCases(
            recordGateway = gateway,
            txtStorageGateway = txtStorageGateway,
            queryGateway = FakeQueryGateway(),
            clock = fixedClock("2026-05-01T01:00:00Z", "Asia/Shanghai")
        )

        val result = useCases.refreshHistory(
            RecordUiState(
                availableMonths = listOf("2026-04"),
                selectedMonth = "2026-04",
                selectedHistoryFile = "2026/2026-04.txt"
            )
        )

        assertEquals("2026-05", gateway.lastCreatedMonth)
        assertEquals("2026-05", result.selectedMonth)
        assertEquals("2026/2026-05.txt", result.selectedHistoryFile)
    }

    @Test
    fun refreshHistory_whenYearRollsOver_createsAndOpensJanuaryTxt() = runTest {
        val gateway = FakeRecordGateway()
        val initialInspection = TxtInspectionResult(
            ok = true,
            entries = listOf(inspectionEntry("2026/2026-12.txt", "2026-12")),
            message = "ok"
        )
        val createdInspection = TxtInspectionResult(
            ok = true,
            entries = listOf(
                inspectionEntry("2026/2026-12.txt", "2026-12"),
                inspectionEntry("2027/2027-01.txt", "2027-01")
            ),
            message = "ok"
        )
        val txtStorageGateway = FakeTxtStorageGateway(
            inspectionResult = initialInspection,
            readResults = mapOf(
                "2027/2027-01.txt" to TxtFileContentResult(
                    ok = true,
                    filePath = "2027/2027-01.txt",
                        content = "y2027\nm01\nd0101\n",
                    message = "Read TXT success."
                )
            ),
            inspectionResults = listOf(initialInspection, createdInspection)
        )
        val useCases = RecordUseCases(
            recordGateway = gateway,
            txtStorageGateway = txtStorageGateway,
            queryGateway = FakeQueryGateway(),
            clock = fixedClock("2027-01-01T01:00:00Z", "Asia/Shanghai")
        )

        val result = useCases.refreshHistory(
            RecordUiState(
                availableMonths = listOf("2026-12"),
                selectedMonth = "2026-12",
                selectedHistoryFile = "2026/2026-12.txt"
            )
        )

        assertEquals("2027-01", gateway.lastCreatedMonth)
        assertEquals("2027-01", result.selectedMonth)
        assertEquals("2027/2027-01.txt", result.selectedHistoryFile)
    }

    @Test
    fun openMonth_whenMonthIsMissing_createsAndOpensTargetMonthTxt() = runTest {
        val gateway = FakeRecordGateway()
        val initialInspection = TxtInspectionResult(
            ok = true,
            entries = listOf(inspectionEntry("2026/2026-04.txt", "2026-04")),
            message = "ok"
        )
        val createdInspection = TxtInspectionResult(
            ok = true,
            entries = listOf(
                inspectionEntry("2026/2026-04.txt", "2026-04"),
                inspectionEntry("2026/2026-05.txt", "2026-05")
            ),
            message = "ok"
        )
        val txtStorageGateway = FakeTxtStorageGateway(
            inspectionResult = initialInspection,
            readResults = mapOf(
                "2026/2026-05.txt" to TxtFileContentResult(
                    ok = true,
                    filePath = "2026/2026-05.txt",
                        content = "y2026\nm05\nd0501\n",
                    message = "Read TXT success."
                )
            ),
            inspectionResults = listOf(initialInspection, createdInspection)
        )
        val useCases = RecordUseCases(
            recordGateway = gateway,
            txtStorageGateway = txtStorageGateway,
            queryGateway = FakeQueryGateway()
        )

        val result = useCases.openMonth(RecordUiState(), "2026-05")

        assertEquals("2026-05", gateway.lastCreatedMonth)
        assertEquals("2026-05", result.selectedMonth)
        assertEquals("2026/2026-05.txt", result.selectedHistoryFile)
    }
}

private class FakeTxtStorageGateway(
    private val inspectionResult: TxtInspectionResult,
    private val readResults: Map<String, TxtFileContentResult>,
    private val inspectionResults: List<TxtInspectionResult> = listOf(inspectionResult),
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
    private var inspectionCallCount = 0

    override suspend fun inspectTxtFiles(): TxtInspectionResult {
        val index = inspectionCallCount.coerceAtMost(inspectionResults.lastIndex)
        inspectionCallCount += 1
        return inspectionResults[index]
    }

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
    ),
    private val recordIntervalResult: RecordActionResult = RecordActionResult(
        ok = true,
        message = "ok"
    )
) : RecordGateway {
    var lastTargetDateIso: String? = null
    var lastTimeOrderMode: RecordTimeOrderMode? = null
    var lastIntervalStart: String? = null
    var lastIntervalEnd: String? = null
    var lastPreferredTxtPath: String? = null
    var lastCreatedMonth: String? = null

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
    ).also {
        lastCreatedMonth = month
    }

    override suspend fun recordNow(
        activityName: String,
        remark: String,
        targetDateIso: String?,
        preferredTxtPath: String?,
        timeOrderMode: RecordTimeOrderMode
    ): RecordActionResult {
        lastTargetDateIso = targetDateIso
        lastPreferredTxtPath = preferredTxtPath
        lastTimeOrderMode = timeOrderMode
        return recordNowResult
    }

    override suspend fun recordInterval(
        activityName: String,
        startTime: String,
        endTime: String,
        remark: String,
        targetDateIso: String?,
        preferredTxtPath: String?
    ): RecordActionResult {
        lastTargetDateIso = targetDateIso
        lastPreferredTxtPath = preferredTxtPath
        lastIntervalStart = startTime
        lastIntervalEnd = endTime
        return recordIntervalResult
    }

    override suspend fun syncLiveToDatabase(): NativeCallResult = NativeCallResult(
        initialized = true,
        operationOk = true,
        rawResponse = """{"ok":true}"""
    )
}

private class FakeQueryGateway(
    private val activityFrequentResult: ActivityFrequentResult = ActivityFrequentResult(
        ok = true,
        frequentActivities = emptyList(),
        message = "ok"
    ),
    private val aliasMappingsResult: ActivityHierarchyLeafMappingListResult = ActivityHierarchyLeafMappingListResult(
        ok = false,
        entries = emptyList(),
        message = "not implemented"
    ),
    private val canonicalCatalogResult: CanonicalCatalogResult = CanonicalCatalogResult(
        ok = true,
        roots = listOf(
            CanonicalPathNode(
                name = "study",
                path = "study",
                children = listOf(
                    CanonicalPathNode(
                        name = "math",
                        path = "study/math"
                    )
                )
            )
        ),
        entries = emptyList(),
        message = "ok"
    ),
    private val wakeKeywordsResult: ActivityMappingNamesResult = ActivityMappingNamesResult(
        ok = false,
        names = emptyList(),
        message = "not implemented"
    )
) : QueryGateway {
    var lastAnchorDateIso: String? = null

    override suspend fun queryFrequentActivities(
        lookbackDays: Int,
        topN: Int,
        anchorDateIso: String?
    ): ActivityFrequentResult = activityFrequentResult.also {
        lastAnchorDateIso = anchorDateIso
    }

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

    override suspend fun listWakeKeywords(): ActivityMappingNamesResult =
        wakeKeywordsResult

    override suspend fun listActivityHierarchyLeafMappings(): ActivityHierarchyLeafMappingListResult =
        aliasMappingsResult

    override suspend fun listCanonicalCatalog(): CanonicalCatalogResult =
        canonicalCatalogResult
}

private class FakeInsightsGateway(
    private val activityName: String,
    private val durationSeconds: Long
) : InsightsGateway {
    override suspend fun insightsMarkdown(request: TemporalInsightsQueryRequest): InsightsCallResult =
        InsightsCallResult(
            initialized = true,
            operationOk = true,
            outputText = "",
            rawResponse = ""
        )

    override suspend fun insightsStructured(
        request: TemporalInsightsQueryRequest
    ): StructuredInsightsCallResult = StructuredInsightsCallResult(
        initialized = true,
        operationOk = true,
        insights = StructuredDailyInsights(
            date = request.selection.date.orEmpty(),
            totalDurationSeconds = durationSeconds,
            activities = listOf(
                ActivityTimelineItem(
                    logicalId = 1L,
                    startTime = "09:00:00",
                    endTime = "10:30:00",
                    activityName = activityName,
                    durationSeconds = durationSeconds
                )
            )
        ),
        rawResponse = ""
    )
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
        content = "y2026\nm03\nd0301\n",
        message = "Read TXT success."
    )

private fun fixedClock(instantIso: String, zoneId: String): Clock =
    Clock.fixed(Instant.parse(instantIso), ZoneId.of(zoneId))
