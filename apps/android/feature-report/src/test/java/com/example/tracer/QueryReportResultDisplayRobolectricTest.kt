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
    fun reportResult_stats_rendersMarkdownContent() {
        val dayLabel = context.getString(R.string.report_mode_day)
        val dayStatsResultLabel = context.getString(
            R.string.report_result_title_stats,
            dayLabel
        )

        renderReportResultDisplay(
            activeResult = QueryResult.Stats(
                text = "## Day Duration Stats\n\nstats-md-marker",
                period = DataTreePeriod.DAY
            ),
            reportMode = ReportMode.DAY
        )

        composeRule.onNodeWithText(dayStatsResultLabel).assertIsDisplayed()
        composeRule.onNodeWithText("stats-md-marker").assertIsDisplayed()
        composeRule.onNodeWithContentDescription(
            context.getString(R.string.report_cd_copy_markdown)
        ).assertIsDisplayed()
    }

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
    fun reportResult_dayMissingTarget_rendersSummaryWithoutMarkdown() {
        val missingSummary = ReportSummary.MissingTarget(
            period = DataTreePeriod.DAY,
            errorCode = "reporting.target.not_found",
            errorCategory = "reporting",
            hints = listOf("Try another date.")
        )
        val dayLabel = context.getString(R.string.report_mode_day)
        val missingTitle = context.getString(
            R.string.report_result_title_report_missing_target,
            dayLabel
        )
        val missingBody = context.getString(
            R.string.report_summary_missing_target_body,
            dayLabel
        )

        renderReportResultDisplay(
            activeResult = null,
            reportSummary = missingSummary,
            reportMode = ReportMode.DAY
        )

        composeRule.onNodeWithText(missingTitle).assertIsDisplayed()
        composeRule.onNodeWithText(missingBody).assertIsDisplayed()
        composeRule.onAllNodesWithText("runtime report failed. [op=missing-day]")
            .assertCountEquals(0)
    }

    private fun renderReportResultDisplay(
        activeResult: QueryResult?,
        reportSummary: ReportSummary? = null,
        reportMode: ReportMode
    ) {
        composeRule.setContent {
            MaterialTheme {
                QueryReportResultDisplay(
                    resultDisplayMode = ReportResultDisplayMode.TEXT,
                    activeResult = activeResult,
                    reportSummary = reportSummary,
                    reportError = "",
                    analysisError = "",
                    chartSemanticMode = ReportChartSemanticMode.COMPOSITION,
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
                    onLoadChart = {}
                )
            }
        }
    }
}
