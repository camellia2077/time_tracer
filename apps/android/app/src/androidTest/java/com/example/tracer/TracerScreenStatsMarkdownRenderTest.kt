package com.example.tracer

import androidx.activity.ComponentActivity
import androidx.compose.ui.test.assertIsDisplayed
import androidx.compose.ui.test.assertCountEquals
import androidx.compose.ui.test.junit4.createAndroidComposeRule
import androidx.compose.ui.test.onAllNodesWithContentDescription
import androidx.compose.ui.test.onAllNodesWithText
import androidx.compose.ui.test.onNodeWithText
import androidx.compose.ui.test.performClick
import androidx.test.ext.junit.runners.AndroidJUnit4
import com.example.tracer.data.AppLanguage
import com.example.tracer.data.DarkThemeStyle
import com.example.tracer.data.ThemeColor
import com.example.tracer.data.ThemeConfig
import com.example.tracer.data.ThemeMode
import com.example.tracer.data.UserPreferencesRepository
import com.example.tracer.data.dataStore
import com.example.tracer.feature.report.R as ReportR
import com.example.tracer.ui.theme.TracerTheme
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class TracerScreenStatsMarkdownRenderTest {
    @get:Rule
    val composeRule = createAndroidComposeRule<ComponentActivity>()

    @Test
    fun reportTab_statsResult_rendersMarkdownContent() {
        val fakeRuntime = FakeTracerScreenServices(
            statsMarkdown = "## Day Duration Stats\n\nstats-md-marker"
        )
        val dayLabel =
            composeRule.activity.getString(ReportR.string.report_mode_day)
        val generateDayStatsLabel = composeRule.activity.getString(
            ReportR.string.report_action_generate_stats,
            dayLabel
        )
        val dayStatsResultLabel = composeRule.activity.getString(
            ReportR.string.report_result_title_stats,
            dayLabel
        )
        val expandLabel =
            composeRule.activity.getString(ReportR.string.report_cd_expand)
        val themeConfig = ThemeConfig(
            themeColor = ThemeColor.Slate,
            themeMode = ThemeMode.Light,
            useDynamicColor = false,
            darkThemeStyle = DarkThemeStyle.Tinted
        )
        val userPreferencesRepository =
            UserPreferencesRepository(composeRule.activity.dataStore)

        composeRule.setContent {
            TracerTheme(themeConfig = themeConfig) {
                TracerScreen(
                    runtimeInitializer = fakeRuntime,
                    recordGateway = fakeRuntime,
                    txtStorageGateway = fakeRuntime,
                    reportGateway = fakeRuntime,
                    queryGateway = fakeRuntime,
                    configGateway = fakeRuntime,
                    tracerExchangeGateway = fakeRuntime,
                    userPreferencesRepository = userPreferencesRepository,
                    themeConfig = themeConfig,
                    onSetThemeColor = {},
                    onSetThemeMode = {},
                    onSetUseDynamicColor = {},
                    onSetDarkThemeStyle = {},
                    appLanguage = AppLanguage.English,
                    onSetAppLanguage = {}
                )
            }
        }

        composeRule.onNodeWithText("Report").performClick()
        composeRule.onAllNodesWithContentDescription(expandLabel)[0].performClick()
        composeRule.onNodeWithText(generateDayStatsLabel).performClick()
        composeRule.waitForIdle()

        composeRule.onNodeWithText(dayStatsResultLabel).assertIsDisplayed()
        composeRule.onNodeWithText("stats-md-marker").assertIsDisplayed()
    }

    @Test
    fun reportTab_recentEmptyWindow_rendersSummaryAboveMarkdown() {
        val fakeRuntime = FakeTracerScreenServices(
            statsMarkdown = "## Day Duration Stats\n\nstats-md-marker",
            recentReportResult = ReportCallResult(
                initialized = true,
                operationOk = true,
                outputText = "## Recent Report\n\nrecent-md-marker",
                rawResponse = """{"ok":true}""",
                reportWindowMetadata = ReportWindowMetadata(
                    hasRecords = false,
                    matchedDayCount = 0,
                    matchedRecordCount = 0,
                    startDate = "2026-02-01",
                    endDate = "2026-02-07",
                    requestedDays = 7
                )
            )
        )
        val recentLabel = composeRule.activity.getString(ReportR.string.report_mode_recent)
        val emptyWindowTitle = composeRule.activity.getString(
            ReportR.string.report_result_title_report_window_summary,
            recentLabel
        )
        val emptyWindowBody = composeRule.activity.getString(
            ReportR.string.report_summary_window_empty_body,
            recentLabel
        )
        val generateReportLabel =
            composeRule.activity.getString(ReportR.string.report_action_generate_report_md)
        val expandLabel =
            composeRule.activity.getString(ReportR.string.report_cd_expand)

        val themeConfig = ThemeConfig(
            themeColor = ThemeColor.Slate,
            themeMode = ThemeMode.Light,
            useDynamicColor = false,
            darkThemeStyle = DarkThemeStyle.Tinted
        )
        val userPreferencesRepository =
            UserPreferencesRepository(composeRule.activity.dataStore)

        composeRule.setContent {
            TracerTheme(themeConfig = themeConfig) {
                TracerScreen(
                    runtimeInitializer = fakeRuntime,
                    recordGateway = fakeRuntime,
                    txtStorageGateway = fakeRuntime,
                    reportGateway = fakeRuntime,
                    queryGateway = fakeRuntime,
                    configGateway = fakeRuntime,
                    tracerExchangeGateway = fakeRuntime,
                    userPreferencesRepository = userPreferencesRepository,
                    themeConfig = themeConfig,
                    onSetThemeColor = {},
                    onSetThemeMode = {},
                    onSetUseDynamicColor = {},
                    onSetDarkThemeStyle = {},
                    appLanguage = AppLanguage.English,
                    onSetAppLanguage = {}
                )
            }
        }

        composeRule.onNodeWithText("Report").performClick()
        composeRule.onNodeWithText(recentLabel).performClick()
        composeRule.onAllNodesWithContentDescription(expandLabel)[0].performClick()
        composeRule.onNodeWithText(generateReportLabel).performClick()
        composeRule.waitForIdle()

        composeRule.onNodeWithText(emptyWindowTitle).assertIsDisplayed()
        composeRule.onNodeWithText(emptyWindowBody).assertIsDisplayed()
        composeRule.onNodeWithText("recent-md-marker").assertIsDisplayed()
    }

    @Test
    fun reportTab_dayMissingTarget_rendersSummaryWithoutMarkdown() {
        val fakeRuntime = FakeTracerScreenServices(
            statsMarkdown = "## Day Duration Stats\n\nstats-md-marker",
            dayReportResult = ReportCallResult(
                initialized = true,
                operationOk = false,
                outputText = "runtime report failed. [op=missing-day]",
                rawResponse = """{"ok":false}""",
                errorContract = ReportErrorContract(
                    errorCode = "reporting.target.not_found",
                    errorCategory = "reporting",
                    hints = listOf("Try another date.")
                )
            )
        )
        val dayLabel = composeRule.activity.getString(ReportR.string.report_mode_day)
        val missingTitle = composeRule.activity.getString(
            ReportR.string.report_result_title_report_missing_target,
            dayLabel
        )
        val missingBody = composeRule.activity.getString(
            ReportR.string.report_summary_missing_target_body,
            dayLabel
        )
        val generateReportLabel =
            composeRule.activity.getString(ReportR.string.report_action_generate_report_md)
        val expandLabel =
            composeRule.activity.getString(ReportR.string.report_cd_expand)

        val themeConfig = ThemeConfig(
            themeColor = ThemeColor.Slate,
            themeMode = ThemeMode.Light,
            useDynamicColor = false,
            darkThemeStyle = DarkThemeStyle.Tinted
        )
        val userPreferencesRepository =
            UserPreferencesRepository(composeRule.activity.dataStore)

        composeRule.setContent {
            TracerTheme(themeConfig = themeConfig) {
                TracerScreen(
                    runtimeInitializer = fakeRuntime,
                    recordGateway = fakeRuntime,
                    txtStorageGateway = fakeRuntime,
                    reportGateway = fakeRuntime,
                    queryGateway = fakeRuntime,
                    configGateway = fakeRuntime,
                    tracerExchangeGateway = fakeRuntime,
                    userPreferencesRepository = userPreferencesRepository,
                    themeConfig = themeConfig,
                    onSetThemeColor = {},
                    onSetThemeMode = {},
                    onSetUseDynamicColor = {},
                    onSetDarkThemeStyle = {},
                    appLanguage = AppLanguage.English,
                    onSetAppLanguage = {}
                )
            }
        }

        composeRule.onNodeWithText("Report").performClick()
        composeRule.onAllNodesWithContentDescription(expandLabel)[0].performClick()
        composeRule.onNodeWithText(generateReportLabel).performClick()
        composeRule.waitForIdle()

        composeRule.onNodeWithText(missingTitle).assertIsDisplayed()
        composeRule.onNodeWithText(missingBody).assertIsDisplayed()
        composeRule.onAllNodesWithText("runtime report failed. [op=missing-day]").assertCountEquals(0)
    }
}

private class FakeTracerScreenServices(
    private val statsMarkdown: String,
    private val dayReportResult: ReportCallResult = emptyReportResult(),
    private val monthReportResult: ReportCallResult = emptyReportResult(),
    private val yearReportResult: ReportCallResult = emptyReportResult(),
    private val weekReportResult: ReportCallResult = emptyReportResult(),
    private val recentReportResult: ReportCallResult = emptyReportResult(),
    private val rangeReportResult: ReportCallResult = emptyReportResult()
) : RuntimeInitializer,
    RecordGateway,
    TxtStorageGateway,
    ReportGateway,
    QueryGateway,
    ConfigGateway,
    TracerExchangeGateway {
    override suspend fun initializeRuntime(): NativeCallResult = NativeCallResult(
        initialized = true,
        operationOk = true,
        rawResponse = """{"ok":true,"content":"","error_message":""}"""
    )

    override suspend fun ingestSingleTxtReplaceMonth(inputPath: String): NativeCallResult =
        initializeRuntime()

    override suspend fun clearAndReinitialize(): ClearAndInitResult = ClearAndInitResult(
        initialized = true,
        operationOk = true,
        clearMessage = "ok",
        initResponse = """{"ok":true}"""
    )

    override suspend fun clearDatabase(): ClearDatabaseResult = ClearDatabaseResult(
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
    ): RecordActionResult = RecordActionResult(ok = true, message = "ok")

    override suspend fun syncLiveToDatabase(): NativeCallResult = initializeRuntime()

    override suspend fun clearTxt(): ClearTxtResult = ClearTxtResult(
        ok = true,
        message = "ok"
    )

    override suspend fun reportDayMarkdown(date: String): ReportCallResult =
        dayReportResult

    override suspend fun reportMonthMarkdown(month: String): ReportCallResult =
        monthReportResult

    override suspend fun reportYearMarkdown(year: String): ReportCallResult =
        yearReportResult

    override suspend fun reportWeekMarkdown(week: String): ReportCallResult =
        weekReportResult

    override suspend fun reportRecentMarkdown(days: String): ReportCallResult =
        recentReportResult

    override suspend fun reportRange(startDate: String, endDate: String): ReportCallResult =
        rangeReportResult

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
        DataQueryTextResult(ok = true, outputText = statsMarkdown, message = "ok")

    override suspend fun queryProjectTree(params: DataTreeQueryParams): TreeQueryResult =
        TreeQueryResult(ok = true, found = false, message = "ok")

    override suspend fun queryProjectTreeText(params: DataTreeQueryParams): DataQueryTextResult =
        DataQueryTextResult(ok = true, outputText = "Total: 0", message = "ok")

    override suspend fun queryReportChart(params: ReportChartQueryParams): ReportChartQueryResult =
        ReportChartQueryResult(
            ok = true,
            data = ReportChartData(
                roots = listOf("study"),
                selectedRoot = "study",
                lookbackDays = params.lookbackDays,
                points = listOf(
                    ReportChartPoint(date = "2026-02-13", durationSeconds = 3600L),
                    ReportChartPoint(date = "2026-02-14", durationSeconds = 5400L)
                )
            ),
            message = "ok"
        )

    override suspend fun listActivityMappingNames(): ActivityMappingNamesResult =
        ActivityMappingNamesResult(ok = true, names = listOf("meal"), message = "ok")

    override suspend fun listTxtFiles(): TxtHistoryListResult = TxtHistoryListResult(
        ok = true,
        files = emptyList(),
        message = "ok"
    )

    override suspend fun inspectTxtFiles(): TxtInspectionResult = TxtInspectionResult(
        ok = true,
        entries = emptyList(),
        message = "ok"
    )

    override suspend fun readTxtFile(relativePath: String): TxtFileContentResult =
        TxtFileContentResult(ok = true, filePath = relativePath, content = "", message = "ok")

    override suspend fun saveTxtFileAndSync(
        relativePath: String,
        content: String
    ): RecordActionResult = RecordActionResult(ok = true, message = "ok")

    override suspend fun listConfigTomlFiles(): ConfigTomlListResult = ConfigTomlListResult(
        ok = true,
        converterFiles = emptyList(),
        chartFiles = emptyList(),
        metaFiles = emptyList(),
        reportFiles = emptyList(),
        message = "ok"
    )

    override suspend fun readConfigTomlFile(relativePath: String): TxtFileContentResult =
        TxtFileContentResult(ok = true, filePath = relativePath, content = "", message = "ok")

    override suspend fun saveConfigTomlFile(
        relativePath: String,
        content: String
    ): TxtFileContentResult =
        TxtFileContentResult(ok = true, filePath = relativePath, content = content, message = "ok")

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

    companion object {
        private fun emptyReportResult(): ReportCallResult = ReportCallResult(
            initialized = true,
            operationOk = true,
            outputText = "",
            rawResponse = """{"ok":true,"content":"","error_message":""}"""
        )
    }
}
