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
import com.example.tracer.feature.report.R
import kotlinx.coroutines.launch


@Composable
internal fun QueryReportResultDisplay(
    resultDisplayMode: ReportResultDisplayMode,
    activeResult: QueryResult?,
    reportSummary: ReportSummary?,
    dayTimeline: StructuredDailyReport?,
    parameterSection: ReportParameterSection,
    reportError: String,
    analysisError: String,
    chartSemanticMode: ReportChartSemanticMode,
    chartVisualMode: ReportChartVisualMode,
    compositionVisualMode: ReportCompositionVisualMode,
    trendChartRoots: List<String>,
    trendChartSelectedRoot: String,
    reportMode: ReportMode,
    trendChartLoading: Boolean,
    trendChartError: String,
    trendChartRenderModel: ChartRenderModel?,
    trendChartLastTrace: ChartQueryTrace?,
    compositionChartLoading: Boolean,
    compositionChartError: String,
    compositionChartRenderModel: CompositionChartRenderModel?,
    compositionChartLastTrace: ChartQueryTrace?,
    chartShowAverageLine: Boolean,
    piePalettePreset: ReportPiePalettePreset,
    heatmapTomlConfig: ReportHeatmapTomlConfig,
    heatmapStylePreference: ReportHeatmapStylePreference,
    onHeatmapThemePolicyChange: (ReportHeatmapThemePolicy) -> Unit,
    onHeatmapPaletteNameChange: (String) -> Unit,
    heatmapApplyMessage: String,
    isAppDarkThemeActive: Boolean,
    onCompositionVisualModeChange: (ReportCompositionVisualMode) -> Unit,
    onChartRootChange: (String) -> Unit,
    onChartShowAverageLineChange: (Boolean) -> Unit,
    onChartVisualModeChange: (ReportChartVisualMode) -> Unit,
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


    if (resultDisplayMode == ReportResultDisplayMode.CHART) {
        ElevatedCard(
            modifier = Modifier.fillMaxWidth()
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                Text(
                    text = stringResource(R.string.report_result_title_chart),
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.primary
                )
                Spacer(modifier = Modifier.height(8.dp))
                ReportChartResultContent(
                    chartSemanticMode = chartSemanticMode,
                    chartVisualMode = chartVisualMode,
                    compositionVisualMode = compositionVisualMode,
                    trendChartRoots = trendChartRoots,
                    trendChartSelectedRoot = trendChartSelectedRoot,
                    reportMode = reportMode,
                    trendChartLoading = trendChartLoading,
                    trendChartError = trendChartError,
                    trendChartRenderModel = trendChartRenderModel,
                    trendChartLastTrace = trendChartLastTrace,
                    compositionChartLoading = compositionChartLoading,
                    compositionChartError = compositionChartError,
                    compositionChartRenderModel = compositionChartRenderModel,
                    compositionChartLastTrace = compositionChartLastTrace,
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

    if (reportSummary != null) {
        QueryReportSummaryCard(
            summary = reportSummary,
            modifier = Modifier.fillMaxWidth()
        )
    }

    if (activeResult != null) {
        ElevatedCard(
            modifier = Modifier.fillMaxWidth()
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                when (activeResult) {
                    is QueryResult.Report -> {
                        if (parameterSection == ReportParameterSection.TIMELINE &&
                            reportMode == ReportMode.DAY
                        ) {
                            ReportActivityTimeline(
                                report = dayTimeline ?: StructuredDailyReport(
                                    date = "",
                                    totalDurationSeconds = 0L
                                ),
                                onUpdateActivityRemark = onUpdateActivityRemark,
                                onUpdateDayRemark = onUpdateDayRemark
                            )
                        } else {
                            MarkdownResultHeader(
                                title = stringResource(R.string.report_result_title_report),
                                markdown = activeResult.text,
                                showDailyStatusEditor = reportMode == ReportMode.DAY,
                                onCopyMarkdown = {
                                    clipboardScope.launch {
                                        clipboard.setClipEntry(
                                            ClipEntry(
                                                ClipData.newPlainText(
                                                    "Time Tracer report",
                                                    activeResult.text
                                                )
                                            )
                                        )
                                    }
                                },
                                onEditDailyStatuses = onEditDailyStatuses
                            )
                            Spacer(modifier = Modifier.height(8.dp))
                            ReportMarkdownText(
                                markdown = activeResult.text,
                                modifier = Modifier.fillMaxWidth()
                            )
                        }
                    }

                    is QueryResult.Tree -> {
                        val periodLabel = stringResource(activeResult.period.reportModeResId())
                        Text(
                            text = stringResource(
                                R.string.report_result_title_tree,
                                periodLabel
                            ),
                            style = MaterialTheme.typography.titleMedium,
                            color = MaterialTheme.colorScheme.primary
                        )
                        Spacer(modifier = Modifier.height(8.dp))
                        QueryReportTreeResultContent(
                            result = activeResult,
                            modifier = Modifier.fillMaxWidth()
                        )
                    }
                }
            }
        }
    }

    if (reportError.isNotBlank()) {
        ElevatedCard(
            modifier = Modifier.fillMaxWidth()
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                Text(
                    text = stringResource(R.string.report_result_title_report_error),
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.error
                )
                Spacer(modifier = Modifier.height(8.dp))
                Text(
                    text = reportError,
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
                    text = stringResource(R.string.report_result_title_analysis_error),
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
                contentDescription = stringResource(R.string.report_cd_copy_markdown)
            )
        }
        if (showDailyStatusEditor) {
            IconButton(onClick = onEditDailyStatuses) {
                Icon(
                    imageVector = Icons.Default.Edit,
                    contentDescription = stringResource(R.string.report_cd_edit_daily_statuses)
                )
            }
        }
    }
}
