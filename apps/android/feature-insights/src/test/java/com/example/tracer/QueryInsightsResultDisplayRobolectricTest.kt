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
import com.example.tracer.feature.insights.R
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class QueryInsightsResultDisplayRobolectricTest {
    @get:Rule
    val composeRule = createComposeRule()

    private val context: Context = ApplicationProvider.getApplicationContext()

    @Test
    fun insightsResult_recentWindow_rendersSummaryAndMarkdown() {
        val recentLabel = context.getString(R.string.insights_mode_recent)
        val emptyWindowTitle = context.getString(
            R.string.insights_result_title_insights_window_summary,
            recentLabel
        )
        val emptyWindowBody = context.getString(
            R.string.insights_summary_window_empty_body,
            recentLabel
        )
        val windowSummary = InsightsSummary.WindowMetadata(
            period = DataTreePeriod.RECENT,
            metadata = InsightsWindowMetadata(
                hasRecords = false,
                matchedDayCount = 0,
                matchedRecordCount = 0,
                startDate = "2026-02-01",
                endDate = "2026-02-07",
                requestedDays = 7
            )
        )

        renderInsightsResultDisplay(
            activeResult = QueryResult.Insights(
                text = "## Recent Insights\n\nrecent-md-marker",
                summary = windowSummary
            ),
            insightsSummary = windowSummary,
            insightsMode = InsightsMode.RECENT
        )

        composeRule.onNodeWithText(emptyWindowTitle).assertIsDisplayed()
        composeRule.onNodeWithText(emptyWindowBody).assertIsDisplayed()
        composeRule.onNodeWithText("recent-md-marker").assertIsDisplayed()
        composeRule.onNodeWithContentDescription(
            context.getString(R.string.insights_cd_copy_markdown)
        ).assertIsDisplayed()
    }

    @Test
    fun insightsResult_dayWithoutRecords_rendersNormalNoDataSummary() {
        val noDataSummary = InsightsSummary.NoData(period = DataTreePeriod.DAY)
        val dayLabel = context.getString(R.string.insights_mode_day)
        val noDataTitle = context.getString(
            R.string.insights_result_title_insights_no_data,
            dayLabel
        )
        val noDataBody = context.getString(
            R.string.insights_summary_no_data_body,
            dayLabel
        )

        renderInsightsResultDisplay(
            activeResult = null,
            insightsSummary = noDataSummary,
            insightsMode = InsightsMode.DAY
        )

        composeRule.onNodeWithText(noDataTitle).assertIsDisplayed()
        composeRule.onNodeWithText(noDataBody).assertIsDisplayed()
        composeRule.onAllNodesWithText("insights.target.not_found")
            .assertCountEquals(0)
        composeRule.onAllNodesWithText("Try another date.")
            .assertCountEquals(0)
    }

    @Test
    fun insightsResult_day_rendersComposeActivityTimeline() {
        renderInsightsResultDisplay(
            activeResult = QueryResult.Insights(
                text = "## Day Insights\n\nmd-marker"
            ),
            dayTimeline = StructuredDailyInsights(
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
            parameterSection = InsightsParameterSection.TIMELINE,
            insightsMode = InsightsMode.DAY
        )

        composeRule.onNodeWithText(
            context.getString(R.string.insights_result_title_activity_timeline)
        ).assertIsDisplayed()
        composeRule.onNodeWithText(
            context.getString(R.string.insights_day_remark_label)
        ).assertIsDisplayed()
        composeRule.onNodeWithText("今天状态不错\n完成了主要计划").assertIsDisplayed()
        composeRule.onAllNodesWithText("study").assertCountEquals(2)
        composeRule.onNodeWithText("math > is > this").assertIsDisplayed()
        composeRule.onNodeWithText("1h 0m").assertIsDisplayed()
        composeRule.onNodeWithText(
            context.getString(
                R.string.insights_activity_timeline_end_only_time,
                "17:40:30"
            )
        ).assertExists()
        composeRule.onAllNodesWithText("0s").assertCountEquals(0)
        composeRule.onAllNodesWithText("整理错题").assertCountEquals(1)
    }

    private fun renderInsightsResultDisplay(
        activeResult: QueryResult?,
        insightsSummary: InsightsSummary? = null,
        dayTimeline: StructuredDailyInsights? = null,
        parameterSection: InsightsParameterSection = InsightsParameterSection.DAY,
        insightsMode: InsightsMode
    ) {
        composeRule.setContent {
            MaterialTheme {
                QueryInsightsResultDisplay(
                    resultDisplayMode = InsightsResultDisplayMode.TEXT,
                    activeResult = activeResult,
                    insightsSummary = insightsSummary,
                    dayTimeline = dayTimeline,
                    parameterSection = parameterSection,
                    insightsError = "",
                    analysisError = "",
                    chartSemanticMode = InsightsChartSemanticMode.COMPOSITION,
                    chartVisualMode = InsightsChartVisualMode.LINE,
                    compositionVisualMode = InsightsCompositionVisualMode.HORIZONTAL_BAR,
                    trendChartRoots = emptyList(),
                    trendChartSelectedRoot = "",
                    insightsMode = insightsMode,
                    trendChartLoading = false,
                    trendChartError = "",
                    trendChartRenderModel = null,
                    trendChartLastTrace = null,
                    compositionChartLoading = false,
                    compositionChartError = "",
                    compositionChartRenderModel = null,
                    compositionChartLastTrace = null,
                    chartShowAverageLine = false,
                    piePalettePreset = InsightsPiePalettePreset.SOFT,
                    heatmapTomlConfig = defaultInsightsHeatmapTomlConfig(),
                    heatmapStylePreference = InsightsHeatmapStylePreference(),
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
