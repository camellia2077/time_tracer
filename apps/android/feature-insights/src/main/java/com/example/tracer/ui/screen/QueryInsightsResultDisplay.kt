package com.example.tracer

import android.content.ClipData
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ContentCopy
import androidx.compose.material.icons.filled.Edit
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.ClipEntry
import androidx.compose.ui.platform.LocalClipboard
import androidx.compose.ui.unit.dp
import androidx.compose.ui.res.stringResource
import com.example.tracer.feature.insights.R
import com.example.tracer.ui.components.CalendarAvailability
import kotlinx.coroutines.launch


@Composable
internal fun QueryInsightsResultDisplay(
    resultDisplayMode: InsightsResultDisplayMode,
    activeResult: QueryResult?,
    insightsSummary: InsightsSummary?,
    dayTimeline: StructuredDailyInsights?,
    periodActivityDays: List<StructuredDailyInsights>,
    periodActivityAggregate: ActivityAggregate,
    periodActivityProjectTree: List<StructuredInsightsProjectNode>,
    periodComparison: InsightsPeriodComparisonState = InsightsPeriodComparisonState.Hidden,
    canComparePreviousPeriod: Boolean = false,
    trendChartComparison: InsightsPeriodComparisonState,
    canCompareChartPreviousPeriod: Boolean,
    calendarAvailability: CalendarAvailability = CalendarAvailability(emptyMap()),
    dayActivitiesView: InsightsActivityView,
    periodActivitiesView: InsightsActivityView,
    onDayActivitiesViewChange: (InsightsActivityView) -> Unit,
    onPeriodActivitiesViewChange: (InsightsActivityView) -> Unit,
    onPeriodComparisonToggle: () -> Unit = {},
    onComparisonPeriodSelected: (InsightsPeriodSelection) -> Unit = {},
    onChartPeriodComparisonToggle: () -> Unit,
    onChartComparisonPeriodSelected: (InsightsPeriodSelection) -> Unit,
    parameterSection: InsightsParameterSection,
    insightsError: String,
    analysisError: String,
    chartSemanticMode: InsightsChartSemanticMode,
    chartVisualMode: InsightsChartVisualMode,
    compositionVisualMode: InsightsCompositionVisualMode,
    trendChartRoots: List<String>,
    trendChartSelectedRoot: String,
    insightsMode: InsightsMode,
    trendChartError: String,
    trendChartRenderModel: ChartRenderModel?,
    compositionChartError: String,
    compositionChartRenderModel: CompositionChartRenderModel?,
    chartShowAverageLine: Boolean,
    piePalettePreset: InsightsPiePalettePreset,
    comparisonColorScheme: InsightsComparisonColorScheme,
    comparisonIndicatorStyle: InsightsComparisonIndicatorStyle,
    heatmapTomlConfig: InsightsHeatmapTomlConfig,
    heatmapStylePreference: InsightsHeatmapStylePreference,
    onHeatmapThemePolicyChange: (InsightsHeatmapThemePolicy) -> Unit,
    onHeatmapPaletteNameChange: (String) -> Unit,
    heatmapApplyMessage: String,
    isAppDarkThemeActive: Boolean,
    onCompositionVisualModeChange: (InsightsCompositionVisualMode) -> Unit,
    onChartRootChange: (String) -> Unit,
    onChartShowAverageLineChange: (Boolean) -> Unit,
    onChartVisualModeChange: (InsightsChartVisualMode) -> Unit,
    is12HourTime: Boolean,
    onUpdateActivityRemark: suspend (ActivityTimelineItem, String) -> RecordActionResult = { _, _ ->
        RecordActionResult(ok = false, message = "Activity remark editing is unavailable.")
    },
    onUpdateDayRemark: suspend (String) -> RecordActionResult = {
        RecordActionResult(ok = false, message = "Day remark editing is unavailable.")
    },
    onEditDailyStatuses: () -> Unit = {}
) {
    val clipboard = LocalClipboard.current
    val clipboardScope = rememberCoroutineScope()


    if (resultDisplayMode == InsightsResultDisplayMode.CHART) {
        ElevatedCard(
            modifier = Modifier.fillMaxWidth()
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                Text(
                    text = stringResource(R.string.insights_result_title_chart),
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.primary
                )
                Spacer(modifier = Modifier.height(8.dp))
                if (chartSemanticMode == InsightsChartSemanticMode.HIERARCHY &&
                    activeResult is QueryResult.Tree
                ) {
                    QueryInsightsTreeResultContent(
                        result = activeResult,
                        modifier = Modifier.fillMaxWidth()
                    )
                } else InsightsChartResultContent(
                    chartSemanticMode = chartSemanticMode,
                    chartVisualMode = chartVisualMode,
                    compositionVisualMode = compositionVisualMode,
                    trendChartRoots = trendChartRoots,
                    trendChartSelectedRoot = trendChartSelectedRoot,
                    insightsMode = insightsMode,
                    calendarAvailability = calendarAvailability,
                    trendChartError = trendChartError,
                    trendChartRenderModel = trendChartRenderModel,
                    trendChartComparison = trendChartComparison,
                    canCompareChartPreviousPeriod = canCompareChartPreviousPeriod,
                    onChartPeriodComparisonToggle = onChartPeriodComparisonToggle,
                    onChartComparisonPeriodSelected = onChartComparisonPeriodSelected,
                    compositionChartError = compositionChartError,
                    compositionChartRenderModel = compositionChartRenderModel,
                    chartShowAverageLine = chartShowAverageLine,
                    piePalettePreset = piePalettePreset,
                    heatmapTomlConfig = heatmapTomlConfig,
                    heatmapStylePreference = heatmapStylePreference,
                    onHeatmapThemePolicyChange = onHeatmapThemePolicyChange,
                    onHeatmapPaletteNameChange = onHeatmapPaletteNameChange,
                    heatmapApplyMessage = heatmapApplyMessage,
                    isAppDarkThemeActive = isAppDarkThemeActive,
                    onCompositionVisualModeChange = onCompositionVisualModeChange,
                    onChartRootChange = onChartRootChange,
                    onChartShowAverageLineChange = onChartShowAverageLineChange,
                    onChartVisualModeChange = onChartVisualModeChange
                )
            }
        }
        return
    }

    if (insightsSummary != null) {
        QueryInsightsSummaryCard(
            summary = insightsSummary,
            modifier = Modifier.fillMaxWidth()
        )
    }

    if (activeResult is QueryResult.Tree) {
        val periodLabel = stringResource(activeResult.period.insightsModeResId())
        Text(
            text = stringResource(
                R.string.insights_result_title_tree,
                periodLabel
            ),
            style = MaterialTheme.typography.titleMedium,
            color = MaterialTheme.colorScheme.primary
        )
        Spacer(modifier = Modifier.height(8.dp))
        QueryInsightsTreeResultContent(
            result = activeResult,
            modifier = Modifier.fillMaxWidth()
        )
    } else if (activeResult != null) {
        ElevatedCard(
            modifier = Modifier.fillMaxWidth()
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                val insights = activeResult as QueryResult.Insights
                if (parameterSection == InsightsParameterSection.ACTIVITIES) {
                    if (insightsMode == InsightsMode.DAY) {
                        InsightsActivityTimeline(
                            insights = dayTimeline ?: StructuredDailyInsights(
                                date = "",
                                totalDurationSeconds = 0L
                            ),
                            activityAggregate = periodActivityAggregate,
                            projectTree = periodActivityProjectTree,
                            calendarAvailability = calendarAvailability,
                            periodComparison = periodComparison,
                            canComparePreviousPeriod = canComparePreviousPeriod,
                            comparisonColorScheme = comparisonColorScheme,
                            comparisonIndicatorStyle = comparisonIndicatorStyle,
                            selectedView = dayActivitiesView,
                            onSelectedViewChange = onDayActivitiesViewChange,
                            onPeriodComparisonToggle = onPeriodComparisonToggle,
                            onComparisonPeriodSelected = onComparisonPeriodSelected,
                            onUpdateActivityRemark = onUpdateActivityRemark,
                            onUpdateDayRemark = onUpdateDayRemark,
                            is12HourTime = is12HourTime
                        )
                    } else {
                        InsightsPeriodActivityBrowser(
                            activityDays = periodActivityDays,
                            activityAggregate = periodActivityAggregate,
                            projectTree = periodActivityProjectTree,
                            insightsMode = insightsMode,
                            periodComparison = periodComparison,
                            canComparePreviousPeriod = canComparePreviousPeriod,
                            comparisonColorScheme = comparisonColorScheme,
                            comparisonIndicatorStyle = comparisonIndicatorStyle,
                            calendarAvailability = calendarAvailability,
                            selectedView = periodActivitiesView,
                            onSelectedViewChange = onPeriodActivitiesViewChange,
                            onPeriodComparisonToggle = onPeriodComparisonToggle,
                            onComparisonPeriodSelected = onComparisonPeriodSelected,
                            modifier = Modifier.fillMaxWidth(),
                            is12HourTime = is12HourTime
                        )
                    }
                } else {
                    MarkdownResultHeader(
                        title = stringResource(R.string.insights_result_title_insights),
                        markdown = insights.text,
                        showDailyStatusEditor = true,
                        statusEditorContentDescription = stringResource(
                            R.string.insights_cd_edit_statuses,
                            stringResource(insightsMode.insightsModeResId())
                        ),
                        onCopyMarkdown = {
                            clipboardScope.launch {
                                clipboard.setClipEntry(
                                    ClipEntry(
                                        ClipData.newPlainText(
                                            "Time Tracer insights",
                                            insights.text
                                        )
                                    )
                                )
                            }
                        },
                        onEditDailyStatuses = onEditDailyStatuses
                    )
                    Spacer(modifier = Modifier.height(8.dp))
                    InsightsMarkdownText(
                        markdown = insights.text,
                        modifier = Modifier.fillMaxWidth()
                    )
                }
            }
        }
    }

    if (insightsError.isNotBlank()) {
        ElevatedCard(
            modifier = Modifier.fillMaxWidth()
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                Text(
                    text = stringResource(R.string.insights_result_title_insights_error),
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.error
                )
                Spacer(modifier = Modifier.height(8.dp))
                Text(
                    text = insightsError,
                    style = MaterialTheme.typography.bodyMedium,
                    modifier = Modifier.fillMaxWidth()
                )
            }
        }
    }

    if (analysisError.isNotBlank()) {
        ElevatedCard(
            modifier = Modifier.fillMaxWidth()
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                Text(
                    text = stringResource(R.string.insights_result_title_analysis_error),
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.error
                )
                Spacer(modifier = Modifier.height(8.dp))
                Text(
                    text = analysisError,
                    style = MaterialTheme.typography.bodyMedium,
                    modifier = Modifier.fillMaxWidth()
                )
            }
        }
    }
}
@Composable
private fun MarkdownResultHeader(
    title: String,
    markdown: String,
    showDailyStatusEditor: Boolean,
    statusEditorContentDescription: String,
    onCopyMarkdown: () -> Unit,
    onEditDailyStatuses: () -> Unit = {}
) {
    Row(modifier = Modifier.fillMaxWidth()) {
        Text(
            text = title,
            modifier = Modifier.weight(1f),
            style = MaterialTheme.typography.titleMedium,
            color = MaterialTheme.colorScheme.primary
        )
        IconButton(
            onClick = onCopyMarkdown,
            enabled = markdown.isNotBlank()
        ) {
            Icon(
                imageVector = Icons.Default.ContentCopy,
                contentDescription = stringResource(R.string.insights_cd_copy_markdown)
            )
        }
        if (showDailyStatusEditor) {
            IconButton(onClick = onEditDailyStatuses) {
                Icon(
                    imageVector = Icons.Default.Edit,
                    contentDescription = statusEditorContentDescription
                )
            }
        }
    }
}

private fun InsightsMode.insightsModeResId(): Int = when (this) {
    InsightsMode.DAY -> R.string.insights_mode_day
    InsightsMode.WEEK -> R.string.insights_mode_week
    InsightsMode.MONTH -> R.string.insights_mode_month
    InsightsMode.YEAR -> R.string.insights_mode_year
    InsightsMode.RANGE -> R.string.insights_mode_range
    InsightsMode.RECENT -> R.string.insights_mode_recent
}
