package com.example.tracer

import androidx.activity.ComponentActivity
import androidx.compose.ui.test.assertIsDisplayed
import androidx.compose.ui.test.junit4.createAndroidComposeRule
import androidx.compose.ui.test.onNodeWithText
import androidx.compose.ui.test.performClick
import androidx.test.ext.junit.runners.AndroidJUnit4
import com.example.tracer.data.DarkThemeStyle
import com.example.tracer.data.ThemeColor
import com.example.tracer.data.ThemeConfig
import com.example.tracer.data.ThemeMode
import com.example.tracer.data.UserPreferencesRepository
import com.example.tracer.data.dataStore
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
        val fakeRuntime = FakeRuntimeGateway(
            statsMarkdown = "## Day Duration Stats\n\nstats-md-marker"
        )
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
                    controller = fakeRuntime,
                    userPreferencesRepository = userPreferencesRepository,
                    themeConfig = themeConfig,
                    onSetThemeColor = {},
                    onSetThemeMode = {},
                    onSetUseDynamicColor = {},
                    onSetDarkThemeStyle = {}
                )
            }
        }

        composeRule.onNodeWithText("Report").performClick()
        composeRule.onNodeWithText("Generate recent Stats").performClick()
        composeRule.waitForIdle()

        composeRule.onNodeWithText("Recent Stats Result").assertIsDisplayed()
        composeRule.onNodeWithText("stats-md-marker").assertIsDisplayed()
    }
}

private class FakeRuntimeGateway(
    private val statsMarkdown: String
) : RuntimeGateway {
    override suspend fun initializeRuntime(): NativeCallResult = NativeCallResult(
        initialized = true,
        operationOk = true,
        rawResponse = """{"ok":true,"content":"","error_message":""}"""
    )

    override suspend fun querySmoke(): NativeCallResult = initializeRuntime()

    override suspend fun ingestSmoke(): NativeCallResult = initializeRuntime()

    override suspend fun ingestFull(): NativeCallResult = initializeRuntime()

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
        targetDateIso: String?
    ): RecordActionResult = RecordActionResult(ok = true, message = "ok")

    override suspend fun syncLiveToDatabase(): NativeCallResult = initializeRuntime()

    override suspend fun clearLiveTxt(): ClearTxtResult = ClearTxtResult(
        ok = true,
        message = "ok"
    )

    override suspend fun reportDayMarkdown(date: String): ReportCallResult =
        emptyReportResult()

    override suspend fun reportMonthMarkdown(month: String): ReportCallResult =
        emptyReportResult()

    override suspend fun reportYearMarkdown(year: String): ReportCallResult =
        emptyReportResult()

    override suspend fun reportWeekMarkdown(week: String): ReportCallResult =
        emptyReportResult()

    override suspend fun reportRecentMarkdown(days: String): ReportCallResult =
        emptyReportResult()

    override suspend fun reportRange(startDate: String, endDate: String): ReportCallResult =
        emptyReportResult()

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

    override suspend fun queryProjectTree(params: DataTreeQueryParams): DataQueryTextResult =
        DataQueryTextResult(ok = true, outputText = "Total: 0", message = "ok")

    override suspend fun listActivityMappingNames(): ActivityMappingNamesResult =
        ActivityMappingNamesResult(ok = true, names = listOf("meal"), message = "ok")

    override suspend fun listLiveTxtFiles(): TxtHistoryListResult = TxtHistoryListResult(
        ok = true,
        files = emptyList(),
        message = "ok"
    )

    override suspend fun readLiveTxtFile(relativePath: String): TxtFileContentResult =
        TxtFileContentResult(ok = true, filePath = relativePath, content = "", message = "ok")

    override suspend fun saveLiveTxtFileAndSync(
        relativePath: String,
        content: String
    ): RecordActionResult = RecordActionResult(ok = true, message = "ok")

    override suspend fun listConfigTomlFiles(): ConfigTomlListResult = ConfigTomlListResult(
        ok = true,
        converterFiles = emptyList(),
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

    private fun emptyReportResult(): ReportCallResult = ReportCallResult(
        initialized = true,
        operationOk = true,
        outputText = "",
        rawResponse = """{"ok":true,"content":"","error_message":""}"""
    )
}
