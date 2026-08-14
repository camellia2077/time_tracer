package com.example.tracer

import android.content.Context
import androidx.compose.material3.MaterialTheme
import androidx.compose.ui.test.assertIsDisplayed
import androidx.compose.ui.test.assertCountEquals
import androidx.compose.ui.test.onAllNodesWithText
import androidx.compose.ui.test.onNodeWithContentDescription
import androidx.compose.ui.test.junit4.createComposeRule
import androidx.compose.ui.test.onNodeWithText
import androidx.compose.ui.test.performClick
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.test.onAllNodesWithTag
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
        composeRule.onNodeWithContentDescription(
            context.getString(R.string.insights_cd_edit_statuses, recentLabel)
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
                    remark = "整理错题",
                    parentColor = "#22C55E"
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
            periodActivityProjectTree = listOf(
                StructuredInsightsProjectNode(
                    name = "study",
                    durationSeconds = 3_600,
                    children = listOf(
                        StructuredInsightsProjectNode(
                            name = "math",
                            durationSeconds = 3_600
                        )
                    )
                )
            ),
            parameterSection = InsightsParameterSection.ACTIVITIES,
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
        composeRule.onAllNodesWithTag("insights-parent-color-indicator")
            .assertCountEquals(1)
    }

    @Test
    fun insightsResult_dayActivities_switchesToReusableOverview() {
        renderInsightsResultDisplay(
            activeResult = QueryResult.Insights(text = "# Day Insights"),
            dayTimeline = StructuredDailyInsights(
                date = "2026-02-14",
                totalDurationSeconds = 3_600,
                activities = listOf(
                    ActivityTimelineItem(
                        startTime = "09:00",
                        endTime = "10:00",
                        activityName = "study_math",
                        durationSeconds = 3_600
                    )
                )
            ),
            periodActivityProjectTree = listOf(
                StructuredInsightsProjectNode(
                    name = "study",
                    durationSeconds = 3_600
                )
            ),
            parameterSection = InsightsParameterSection.ACTIVITIES,
            insightsMode = InsightsMode.DAY
        )

        composeRule.onNodeWithText(
            context.getString(R.string.insights_period_activities_records)
        ).assertIsDisplayed()
        composeRule.onNodeWithText(
            context.getString(R.string.insights_period_activities_overview)
        ).performClick()

        composeRule.onNodeWithText(
            context.getString(R.string.insights_period_activities_total)
        ).assertIsDisplayed()
        composeRule.onNodeWithText("study").assertIsDisplayed()
    }

    @Test
    fun insightsResult_markdownBreakdown_collapsesNestedActivities() {
        renderInsightsResultDisplay(
            activeResult = QueryResult.Insights(
                text = """
                    ## Project Breakdown

                    - **study**: 3h 0m
                      - math: 2h 0m
                        - algebra: 1h 0m
                    - sleep: 1h 0m
                """.trimIndent()
            ),
            insightsMode = InsightsMode.MONTH
        )

        composeRule.onNodeWithText("math: 2h 0m").assertIsDisplayed()
        composeRule.onNodeWithText("algebra: 1h 0m").assertIsDisplayed()

        composeRule.onNodeWithText("study: 3h 0m").performClick()

        composeRule.onAllNodesWithText("math: 2h 0m").assertCountEquals(0)
        composeRule.onAllNodesWithText("algebra: 1h 0m").assertCountEquals(0)
        composeRule.onNodeWithText("sleep: 1h 0m").assertIsDisplayed()
    }

    @Test
    fun insightsResult_periodActivities_startsWithOverviewAndDisclosesDayRecords() {
        renderInsightsResultDisplay(
            activeResult = QueryResult.Insights(text = "# Month Insights"),
            periodActivityDays = listOf(
                StructuredDailyInsights(
                    date = "2026-05-18",
                    totalDurationSeconds = 5_400,
                    activities = listOf(
                        ActivityTimelineItem(
                            startTime = "09:00",
                            endTime = "10:30",
                            activityName = "study_math_calculus",
                            durationSeconds = 5_400
                        )
                    )
                )
            ),
            periodActivityProjectTree = listOf(
                StructuredInsightsProjectNode(
                    name = "study",
                    durationSeconds = 5_400,
                    children = listOf(
                        StructuredInsightsProjectNode(
                            name = "math",
                            durationSeconds = 5_400,
                            children = listOf(
                                StructuredInsightsProjectNode(
                                    name = "calculus",
                                    durationSeconds = 5_400
                                )
                            )
                        )
                    )
                )
            ),
            parameterSection = InsightsParameterSection.ACTIVITIES,
            insightsMode = InsightsMode.MONTH
        )

        val overviewLabel = context.getString(R.string.insights_period_activities_overview)
        val recordsLabel = context.getString(R.string.insights_period_activities_records)
        composeRule.onNodeWithText(overviewLabel).assertIsDisplayed()
        composeRule.onNodeWithText(
            context.getString(R.string.insights_period_activities_total)
        ).assertIsDisplayed()
        composeRule.onNodeWithText(
            context.getString(R.string.insights_period_activities_parent_breakdown)
        ).assertIsDisplayed()
        composeRule.onNodeWithText("study").assertIsDisplayed()
        composeRule.onNodeWithText("100%").assertIsDisplayed()
        composeRule.onAllNodesWithText("math").assertCountEquals(0)
        composeRule.onNodeWithText("study").performClick()
        composeRule.onNodeWithText("math").assertExists()

        composeRule.onAllNodesWithText(recordsLabel)[0].performClick()
        composeRule.onNodeWithText("2026-05-18").assertIsDisplayed()
        composeRule.onAllNodesWithText("math > calculus").assertCountEquals(0)
        composeRule.onNodeWithText("2026-05-18").performClick()
        composeRule.onNodeWithText("study").assertIsDisplayed()
        composeRule.onNodeWithText("math > calculus").assertIsDisplayed()
        composeRule.onAllNodesWithText(
            context.getString(R.string.insights_edit_activity_remark)
        ).assertCountEquals(0)
    }

    @Test
    fun insightsResult_periodActivities_rendersPreviousPeriodComparison() {
        renderInsightsResultDisplay(
            activeResult = QueryResult.Insights(text = "# Month Insights"),
            periodActivityDays = listOf(
                StructuredDailyInsights(
                    date = "2026-05-18",
                    totalDurationSeconds = 3_600,
                    activities = listOf(
                        ActivityTimelineItem(
                            startTime = "09:00",
                            endTime = "10:00",
                            activityName = "study",
                            durationSeconds = 3_600
                        )
                    )
                )
            ),
            periodActivityProjectTree = listOf(StructuredInsightsProjectNode("study", 3_600)),
            periodComparison = InsightsPeriodComparisonState.Ready(
                label = "2026-04-01 – 2026-04-30",
                selection = InsightsPeriodSelection(
                    date = "20260401",
                    month = "202604",
                    year = "2026",
                    week = "202614"
                ),
                activityDays = emptyList(),
                projectTree = emptyList()
            ),
            parameterSection = InsightsParameterSection.ACTIVITIES,
            insightsMode = InsightsMode.MONTH
        )

        composeRule.onAllNodesWithText(
            context.getString(R.string.insights_period_activities_compare_previous)
        ).assertCountEquals(1)
        composeRule.onAllNodesWithText(
            context.getString(
                R.string.insights_period_activities_comparison_period,
                "2026-04-01 – 2026-04-30"
            )
        ).assertCountEquals(1)
        composeRule.onAllNodesWithText(
            context.getString(
                R.string.insights_period_activities_comparison_new_value,
                "+1h 0m"
            )
        ).assertCountEquals(3)
    }

    @Test
    fun insightsResult_periodActivities_groupsMultiMonthRecordsBeforeDays() {
        renderInsightsResultDisplay(
            activeResult = QueryResult.Insights(text = "# Year Insights"),
            periodActivityDays = listOf(
                StructuredDailyInsights(
                    date = "2026-05-18",
                    totalDurationSeconds = 3_600,
                    activities = listOf(
                        ActivityTimelineItem(
                            startTime = "09:00",
                            endTime = "10:00",
                            activityName = "study_math",
                            durationSeconds = 3_600
                        )
                    )
                ),
                StructuredDailyInsights(
                    date = "2026-04-30",
                    totalDurationSeconds = 1_800,
                    activities = listOf(
                        ActivityTimelineItem(
                            startTime = "18:00",
                            endTime = "18:30",
                            activityName = "exercise_run",
                            durationSeconds = 1_800
                        )
                    )
                )
            ),
            parameterSection = InsightsParameterSection.ACTIVITIES,
            insightsMode = InsightsMode.YEAR
        )

        composeRule.onAllNodesWithText(
            context.getString(R.string.insights_period_activities_records)
        )[0].performClick()
        composeRule.onNodeWithText("2026-05").assertIsDisplayed()
        composeRule.onNodeWithText("2026-04").assertIsDisplayed()
        composeRule.onAllNodesWithText("2026-05-18").assertCountEquals(0)

        composeRule.onNodeWithText("2026-05").performClick()
        composeRule.onNodeWithText("2026-05-18").assertIsDisplayed()
        composeRule.onAllNodesWithText("math").assertCountEquals(0)

        composeRule.onNodeWithText("2026-05-18").performClick()
        // The production page owns scrolling; this hostless renderer deliberately has no
        // viewport scroll container. Verify that expanding a day exposes the record semantics,
        // rather than treating off-screen placement as a product failure.
        composeRule.onAllNodesWithText("study").assertCountEquals(1)
        composeRule.onAllNodesWithText("math").assertCountEquals(1)
    }

    @Test
    fun insightsResult_periodActivities_parentDurationUsesDays() {
        renderInsightsResultDisplay(
            activeResult = QueryResult.Insights(text = "# Month Insights"),
            periodActivityDays = listOf(
                StructuredDailyInsights(
                    date = "2026-05-18",
                    totalDurationSeconds = 93_600,
                    activities = listOf(
                        ActivityTimelineItem(
                            startTime = "00:00",
                            endTime = "26:00",
                            activityName = "study_math",
                            durationSeconds = 93_600
                        )
                    )
                )
            ),
            periodActivityProjectTree = listOf(
                StructuredInsightsProjectNode(
                    name = "study",
                    durationSeconds = 93_600
                )
            ),
            parameterSection = InsightsParameterSection.ACTIVITIES,
            insightsMode = InsightsMode.MONTH
        )

        // The duration is repeated by the total, average, and parent activity. Its presence in
        // the semantics tree is the stable contract; a hostless test has no scroll viewport.
        composeRule.onAllNodesWithText("1d 2h", substring = true).assertCountEquals(3)
    }

    private fun renderInsightsResultDisplay(
        activeResult: QueryResult?,
        insightsSummary: InsightsSummary? = null,
        dayTimeline: StructuredDailyInsights? = null,
        periodActivityDays: List<StructuredDailyInsights> = emptyList(),
        periodActivityProjectTree: List<StructuredInsightsProjectNode> = emptyList(),
        periodComparison: InsightsPeriodComparisonState = InsightsPeriodComparisonState.Hidden,
        parameterSection: InsightsParameterSection = InsightsParameterSection.DAY,
        insightsMode: InsightsMode
    ) {
        composeRule.setContent {
            MaterialTheme {
                var dayActivitiesView by remember { mutableStateOf(InsightsActivityView.RECORDS) }
                var periodActivitiesView by remember { mutableStateOf(InsightsActivityView.OVERVIEW) }
                QueryInsightsResultDisplay(
                    resultDisplayMode = InsightsResultDisplayMode.TEXT,
                    activeResult = activeResult,
                    insightsSummary = insightsSummary,
                    dayTimeline = dayTimeline,
                    periodActivityDays = periodActivityDays,
                    periodActivityProjectTree = periodActivityProjectTree,
                    periodComparison = periodComparison,
                    dayActivitiesView = dayActivitiesView,
                    periodActivitiesView = periodActivitiesView,
                    onDayActivitiesViewChange = { dayActivitiesView = it },
                    onPeriodActivitiesViewChange = { periodActivitiesView = it },
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
