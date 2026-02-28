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
    analysisError: String,
    chartRoots: List<String>,
    chartSelectedRoot: String,
    chartDateInputMode: ChartDateInputMode,
    chartLookbackDays: String,
    chartRangeStartDate: String,
    chartRangeEndDate: String,
    chartLoading: Boolean,
    chartError: String,
    chartRenderModel: ChartRenderModel?,
    chartLastTrace: ChartQueryTrace?,
    chartShowAverageLine: Boolean,
    heatmapTomlConfig: ReportHeatmapTomlConfig,
    heatmapStylePreference: ReportHeatmapStylePreference,
    onHeatmapThemePolicyChange: (ReportHeatmapThemePolicy) -> Unit,
    onHeatmapPaletteNameChange: (String) -> Unit,
    heatmapApplyMessage: String,
    isAppDarkThemeActive: Boolean,
    onChartRootChange: (String) -> Unit,
    onChartDateInputModeChange: (ChartDateInputMode) -> Unit,
    onChartLookbackDaysChange: (String) -> Unit,
    onChartRangeStartDateChange: (String) -> Unit,
    onChartRangeEndDateChange: (String) -> Unit,
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
                    chartRoots = chartRoots,
                    chartSelectedRoot = chartSelectedRoot,
                    chartDateInputMode = chartDateInputMode,
                    chartLookbackDays = chartLookbackDays,
                    chartRangeStartDate = chartRangeStartDate,
                    chartRangeEndDate = chartRangeEndDate,
                    chartLoading = chartLoading,
                    chartError = chartError,
                    chartRenderModel = chartRenderModel,
                    chartLastTrace = chartLastTrace,
                    chartShowAverageLine = chartShowAverageLine,
                    heatmapTomlConfig = heatmapTomlConfig,
                    heatmapStylePreference = heatmapStylePreference,
                    onHeatmapThemePolicyChange = onHeatmapThemePolicyChange,
                    onHeatmapPaletteNameChange = onHeatmapPaletteNameChange,
                    heatmapApplyMessage = heatmapApplyMessage,
                    isAppDarkThemeActive = isAppDarkThemeActive,
                    onChartRootChange = onChartRootChange,
                    onChartDateInputModeChange = onChartDateInputModeChange,
                    onChartLookbackDaysChange = onChartLookbackDaysChange,
                    onChartRangeStartDateChange = onChartRangeStartDateChange,
                    onChartRangeEndDateChange = onChartRangeEndDateChange,
                    onChartShowAverageLineChange = onChartShowAverageLineChange,
                    onLoadChart = onLoadChart
                )
            }
        }
        return
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
