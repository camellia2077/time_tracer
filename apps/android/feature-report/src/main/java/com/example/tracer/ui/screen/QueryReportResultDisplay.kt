package com.example.tracer

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.compose.ui.res.stringResource
import com.example.tracer.feature.report.R

@Composable
internal fun QueryReportResultDisplay(
    resultDisplayMode: ReportResultDisplayMode,
    activeResult: QueryResult?,
    reportSummary: ReportSummary?,
    reportError: String,
    analysisError: String,
    chartSemanticMode: ReportChartSemanticMode,
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
    onChartSemanticModeChange: (ReportChartSemanticMode) -> Unit,
    onCompositionVisualModeChange: (ReportCompositionVisualMode) -> Unit,
    onChartRootChange: (String) -> Unit,
    onChartShowAverageLineChange: (Boolean) -> Unit,
    onLoadChart: () -> Unit
) {
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
                    onChartSemanticModeChange = onChartSemanticModeChange,
                    onCompositionVisualModeChange = onCompositionVisualModeChange,
                    onChartRootChange = onChartRootChange,
                    onChartShowAverageLineChange = onChartShowAverageLineChange,
                    onLoadChart = onLoadChart
                )
            }
        }
        return
    }

    if (reportSummary != null) {
        ReportSummaryCard(
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
                        Text(
                            text = stringResource(R.string.report_result_title_report),
                            style = MaterialTheme.typography.titleMedium,
                            color = MaterialTheme.colorScheme.primary
                        )
                        Spacer(modifier = Modifier.height(8.dp))
                        ReportMarkdownText(
                            markdown = activeResult.text,
                            modifier = Modifier.fillMaxWidth()
                        )
                    }

                    is QueryResult.Stats -> {
                        val periodLabel = stringResource(activeResult.period.reportModeResId())
                        Text(
                            text = stringResource(
                                R.string.report_result_title_stats,
                                periodLabel
                            ),
                            style = MaterialTheme.typography.titleMedium,
                            color = MaterialTheme.colorScheme.primary
                        )
                        Spacer(modifier = Modifier.height(8.dp))
                        ReportMarkdownText(
                            markdown = activeResult.text,
                            modifier = Modifier.fillMaxWidth()
                        )
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
private fun ReportSummaryCard(
    summary: ReportSummary,
    modifier: Modifier = Modifier
) {
    ElevatedCard(modifier = modifier) {
        Column(modifier = Modifier.padding(16.dp)) {
            when (summary) {
                is ReportSummary.MissingTarget -> {
                    val periodLabel = stringResource(summary.period.reportModeResId())
                    Text(
                        text = stringResource(
                            R.string.report_result_title_report_missing_target,
                            periodLabel
                        ),
                        style = MaterialTheme.typography.titleMedium,
                        color = MaterialTheme.colorScheme.error
                    )
                    Spacer(modifier = Modifier.height(8.dp))
                    Text(
                        text = stringResource(
                            R.string.report_summary_missing_target_body,
                            periodLabel
                        ),
                        style = MaterialTheme.typography.bodyMedium
                    )
                    if (summary.errorCode.isNotBlank()) {
                        SummaryLine(
                            text = stringResource(
                                R.string.report_summary_error_code,
                                summary.errorCode
                            )
                        )
                    }
                    if (summary.errorCategory.isNotBlank()) {
                        SummaryLine(
                            text = stringResource(
                                R.string.report_summary_error_category,
                                summary.errorCategory
                            )
                        )
                    }
                    if (summary.hints.isNotEmpty()) {
                        SummaryLine(
                            text = stringResource(
                                R.string.report_summary_hints,
                                summary.hints.joinToString(separator = " | ")
                            )
                        )
                    }
                }

                is ReportSummary.WindowMetadata -> {
                    val periodLabel = stringResource(summary.period.reportModeResId())
                    val metadata = summary.metadata
                    Text(
                        text = stringResource(
                            R.string.report_result_title_report_window_summary,
                            periodLabel
                        ),
                        style = MaterialTheme.typography.titleMedium,
                        color = MaterialTheme.colorScheme.primary
                    )
                    Spacer(modifier = Modifier.height(8.dp))
                    Text(
                        text = if (metadata.hasRecords) {
                            stringResource(
                                R.string.report_summary_window_has_records_body,
                                periodLabel
                            )
                        } else {
                            stringResource(
                                R.string.report_summary_window_empty_body,
                                periodLabel
                            )
                        },
                        style = MaterialTheme.typography.bodyMedium
                    )
                    if (metadata.startDate.isNotBlank() || metadata.endDate.isNotBlank()) {
                        SummaryLine(
                            text = stringResource(
                                R.string.report_summary_window_range,
                                metadata.startDate.ifBlank { "-" },
                                metadata.endDate.ifBlank { "-" }
                            )
                        )
                    }
                    if (metadata.requestedDays > 0) {
                        SummaryLine(
                            text = stringResource(
                                R.string.report_summary_requested_days,
                                metadata.requestedDays
                            )
                        )
                    }
                    SummaryLine(
                        text = stringResource(
                            R.string.report_summary_matched_days,
                            metadata.matchedDayCount
                        )
                    )
                    SummaryLine(
                        text = stringResource(
                            R.string.report_summary_matched_records,
                            metadata.matchedRecordCount
                        )
                    )
                }
            }
        }
    }
}

@Composable
private fun SummaryLine(text: String) {
    Spacer(modifier = Modifier.height(6.dp))
    Text(
        text = text,
        style = MaterialTheme.typography.bodySmall,
        modifier = Modifier.fillMaxWidth()
    )
}

private fun DataTreePeriod.reportModeResId(): Int {
    return when (this) {
        DataTreePeriod.DAY -> R.string.report_mode_day
        DataTreePeriod.WEEK -> R.string.report_mode_week
        DataTreePeriod.MONTH -> R.string.report_mode_month
        DataTreePeriod.YEAR -> R.string.report_mode_year
        DataTreePeriod.RECENT -> R.string.report_mode_recent
        DataTreePeriod.RANGE -> R.string.report_mode_range
    }
}
