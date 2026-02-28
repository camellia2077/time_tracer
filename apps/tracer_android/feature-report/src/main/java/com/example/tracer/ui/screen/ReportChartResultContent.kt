package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.report.R
import java.time.LocalDate

@Composable
fun ReportResultModeSwitcher(
    mode: ReportResultDisplayMode,
    onModeChange: (ReportResultDisplayMode) -> Unit,
    modifier: Modifier = Modifier
) {
    val modes = ReportResultDisplayMode.entries
    SingleChoiceSegmentedButtonRow(modifier = modifier.fillMaxWidth()) {
        modes.forEachIndexed { index, item ->
            SegmentedButton(
                shape = SegmentedButtonDefaults.itemShape(
                    index = index,
                    count = modes.size
                ),
                onClick = { onModeChange(item) },
                selected = mode == item,
                label = { Text(text = stringResource(item.labelRes())) }
            )
        }
    }
}

@Composable
fun ReportChartResultContent(
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
    onLoadChart: () -> Unit,
    modifier: Modifier = Modifier
) {
    val normalizedRoots = remember(chartRoots) { chartRoots.distinct() }
    val rootOptions = remember(normalizedRoots) { listOf("") + normalizedRoots }
    val sortedChartPoints = remember(chartRenderModel) {
        chartRenderModel?.points
            ?.sortedWith(
                compareBy<ReportChartPoint>(
                    { it.epochDay ?: parseEpochDayOrNull(it.date) ?: Long.MAX_VALUE },
                    { it.date }
                )
            )
            ?: emptyList()
    }
    val chartAverageDurationSeconds = chartRenderModel?.averageDurationSeconds
    val chartUsesLegacyStatsFallback = chartRenderModel?.usesLegacyStatsFallback == true
    var selectedPointIndex by remember(sortedChartPoints) {
        mutableIntStateOf(
            if (sortedChartPoints.isEmpty()) {
                -1
            } else {
                sortedChartPoints.lastIndex
            }
        )
    }
    var chartVisualMode by rememberSaveable { mutableStateOf(ReportChartVisualMode.LINE) }

    Column(
        modifier = modifier
            .fillMaxWidth()
            .padding(top = 4.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        Text(
            text = stringResource(R.string.report_title_chart_parameters),
            style = MaterialTheme.typography.titleMedium,
            color = MaterialTheme.colorScheme.primary
        )

        ReportChartParameterSection(
            rootOptions = rootOptions,
            chartSelectedRoot = chartSelectedRoot,
            chartDateInputMode = chartDateInputMode,
            chartLookbackDays = chartLookbackDays,
            chartRangeStartDate = chartRangeStartDate,
            chartRangeEndDate = chartRangeEndDate,
            chartLoading = chartLoading,
            chartLastTrace = chartLastTrace,
            onChartRootChange = onChartRootChange,
            onChartDateInputModeChange = onChartDateInputModeChange,
            onChartLookbackDaysChange = onChartLookbackDaysChange,
            onChartRangeStartDateChange = onChartRangeStartDateChange,
            onChartRangeEndDateChange = onChartRangeEndDateChange,
            onLoadChart = onLoadChart
        )

        ReportChartVisualizationSection(
            chartError = chartError,
            chartDateInputMode = chartDateInputMode,
            sortedChartPoints = sortedChartPoints,
            chartVisualMode = chartVisualMode,
            onChartVisualModeChange = { chartVisualMode = it },
            selectedPointIndex = selectedPointIndex,
            onPointSelected = { selectedPointIndex = it },
            chartAverageDurationSeconds = chartAverageDurationSeconds,
            chartUsesLegacyStatsFallback = chartUsesLegacyStatsFallback,
            chartShowAverageLine = chartShowAverageLine,
            onChartShowAverageLineChange = onChartShowAverageLineChange,
            heatmapTomlConfig = heatmapTomlConfig,
            heatmapStylePreference = heatmapStylePreference,
            onHeatmapThemePolicyChange = onHeatmapThemePolicyChange,
            onHeatmapPaletteNameChange = onHeatmapPaletteNameChange,
            heatmapApplyMessage = heatmapApplyMessage,
            isAppDarkThemeActive = isAppDarkThemeActive
        )
    }
}

private fun parseEpochDayOrNull(dateIso: String): Long? =
    try {
        LocalDate.parse(dateIso).toEpochDay()
    } catch (_: Exception) {
        null
    }
