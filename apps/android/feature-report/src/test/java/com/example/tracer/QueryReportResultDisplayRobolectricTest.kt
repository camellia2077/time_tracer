package com.example.tracer

import android.content.Context
import androidx.compose.material3.MaterialTheme
import androidx.compose.ui.test.assertIsDisplayed
import androidx.compose.ui.test.assertCountEquals
import androidx.compose.ui.test.onAllNodesWithText
import androidx.compose.ui.test.onNodeWithContentDescription
import androidx.compose.ui.test.junit4.createComposeRule
import androidx.compose.ui.test.onNodeWithText
import androidx.test.core.app.ApplicationProvider
import com.example.tracer.feature.report.R
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class QueryReportResultDisplayRobolectricTest {
    @get:Rule
    val composeRule = createComposeRule()

    private val context: Context = ApplicationProvider.getApplicationContext()

    @Test
    fun reportResult_recentWindow_rendersSummaryAndMarkdown() {
        val recentLabel = context.getString(R.string.report_mode_recent)
        val emptyWindowTitle = context.getString(
            R.string.report_result_title_report_window_summary,
            recentLabel
        )
        val emptyWindowBody = context.getString(
            R.string.report_summary_window_empty_body,
            recentLabel
        )
        val windowSummary = ReportSummary.WindowMetadata(
            period = DataTreePeriod.RECENT,
            metadata = ReportWindowMetadata(
                hasRecords = false,
                matchedDayCount = 0,
                matchedRecordCount = 0,
                startDate = "2026-02-01",
                endDate = "2026-02-07",
                requestedDays = 7
            )
        )

        renderReportResultDisplay(
            activeResult = QueryResult.Report(
                text = "## Recent Report\n\nrecent-md-marker",
                summary = windowSummary
            ),
            reportSummary = windowSummary,
            reportMode = ReportMode.RECENT
        )

        composeRule.onNodeWithText(emptyWindowTitle).assertIsDisplayed()
        composeRule.onNodeWithText(emptyWindowBody).assertIsDisplayed()
        composeRule.onNodeWithText("recent-md-marker").assertIsDisplayed()
        composeRule.onNodeWithContentDescription(
            context.getString(R.string.report_cd_copy_markdown)
        ).assertIsDisplayed()
    }

    @Test
    fun reportResult_dayWithoutRecords_rendersNormalNoDataSummary() {
        val noDataSummary = ReportSummary.NoData(period = DataTreePeriod.DAY)
        val dayLabel = context.getString(R.string.report_mode_day)
        val noDataTitle = context.getString(
            R.string.report_result_title_report_no_data,
            dayLabel
        )
        val noDataBody = context.getString(
            R.string.report_summary_no_data_body,
            dayLabel
        )

        renderReportResultDisplay(
            activeResult = null,
            reportSummary = noDataSummary,
            reportMode = ReportMode.DAY
        )

        composeRule.onNodeWithText(noDataTitle).assertIsDisplayed()
        composeRule.onNodeWithText(noDataBody).assertIsDisplayed()
        composeRule.onAllNodesWithText("reporting.target.not_found")
            .assertCountEquals(0)
        composeRule.onAllNodesWithText("Try another date.")
            .assertCountEquals(0)
    }

    @Test
    fun reportResult_day_rendersComposeActivityTimeline() {
        renderReportResultDisplay(
            activeResult = QueryResult.Report(
                text = "## Day Report\n\nmd-marker"
            ),
            dayTimeline = StructuredDailyReport(
                date = "2026-02-14",
                totalDurationSeconds = 3_600,
                dayRemark = "今天状态不错\n完成了主要计划",
                activities = listOf(
                    ActivityTimelineItem(
                        startTime = "09:00",
                        endTime = "10:00",
                        activityName = "study_math_is_this",
                        durationSeconds = 3_600,
                        remark = "整理错题"
                    ),
                    ActivityTimelineItem(
                        startTime = "",
                        endTime = "17:40:30",
                        activityName = "study_math_checkpoint",
                        durationSeconds = 0,
                        kind = ActivityTimelineRecordKind.END_ONLY
                    )
                )
            ),
            parameterSection = ReportParameterSection.TIMELINE,
            reportMode = ReportMode.DAY
        )

        composeRule.onNodeWithText(
            context.getString(R.string.report_result_title_activity_timeline)
        ).assertIsDisplayed()
        composeRule.onNodeWithText(
            context.getString(R.string.report_day_remark_label)
        ).assertIsDisplayed()
        composeRule.onNodeWithText("今天状态不错\n完成了主要计划").assertIsDisplayed()
        composeRule.onAllNodesWithText("study").assertCountEquals(2)
        composeRule.onNodeWithText("math > is > this").assertIsDisplayed()
        composeRule.onNodeWithText("1h 0m").assertIsDisplayed()
        composeRule.onNodeWithText(
            context.getString(
                R.string.report_activity_timeline_end_only_time,
                "17:40:30"
            )
        ).assertExists()
        composeRule.onAllNodesWithText("0s").assertCountEquals(0)
        composeRule.onAllNodesWithText("整理错题").assertCountEquals(1)
    }

    private fun renderReportResultDisplay(
        activeResult: QueryResult?,
        reportSummary: ReportSummary? = null,
        dayTimeline: StructuredDailyReport? = null,
        parameterSection: ReportParameterSection = ReportParameterSection.DAY,
        reportMode: ReportMode
    ) {
        composeRule.setContent {
            MaterialTheme {
                QueryReportResultDisplay(
                    resultDisplayMode = ReportResultDisplayMode.TEXT,
                    activeResult = activeResult,
                    reportSummary = reportSummary,
                    dayTimeline = dayTimeline,
                    parameterSection = parameterSection,
                    reportError = "",
                    analysisError = "",
                    chartSemanticMode = ReportChartSemanticMode.COMPOSITION,
                    chartVisualMode = ReportChartVisualMode.LINE,
                    compositionVisualMode = ReportCompositionVisualMode.HORIZONTAL_BAR,
                    trendChartRoots = emptyList(),
                    trendChartSelectedRoot = "",
                    reportMode = reportMode,
                    trendChartLoading = false,
                    trendChartError = "",
                    trendChartRenderModel = null,
                    trendChartLastTrace = null,
                    compositionChartLoading = false,
                    compositionChartError = "",
                    compositionChartRenderModel = null,
                    compositionChartLastTrace = null,
                    chartShowAverageLine = false,
                    piePalettePreset = ReportPiePalettePreset.SOFT,
                    heatmapTomlConfig = defaultReportHeatmapTomlConfig(),
                    heatmapStylePreference = ReportHeatmapStylePreference(),
                    onHeatmapThemePolicyChange = {},
                    onHeatmapPaletteNameChange = {},
                    heatmapApplyMessage = "",
                    isAppDarkThemeActive = false,
                    onCompositionVisualModeChange = {},
                    onChartRootChange = {},
                    onChartShowAverageLineChange = {},
                    onChartVisualModeChange = {}
                )
            }
        }
    }
}
