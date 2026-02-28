package com.example.tracer

import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.report.R

@Composable
internal fun ReportChartVisualizationSection(
    chartError: String,
    chartDateInputMode: ChartDateInputMode,
    sortedChartPoints: List<ReportChartPoint>,
    chartVisualMode: ReportChartVisualMode,
    onChartVisualModeChange: (ReportChartVisualMode) -> Unit,
    selectedPointIndex: Int,
    onPointSelected: (Int) -> Unit,
    chartAverageDurationSeconds: Long?,
    chartUsesLegacyStatsFallback: Boolean,
    chartShowAverageLine: Boolean,
    onChartShowAverageLineChange: (Boolean) -> Unit,
    heatmapTomlConfig: ReportHeatmapTomlConfig,
    heatmapStylePreference: ReportHeatmapStylePreference,
    onHeatmapThemePolicyChange: (ReportHeatmapThemePolicy) -> Unit,
    onHeatmapPaletteNameChange: (String) -> Unit,
    heatmapApplyMessage: String,
    isAppDarkThemeActive: Boolean
) {
    if (chartError.isNotBlank()) {
        Text(
            text = chartError,
            style = MaterialTheme.typography.bodyMedium,
            color = MaterialTheme.colorScheme.error,
            modifier = Modifier.fillMaxWidth()
        )
        return
    }

    if (sortedChartPoints.isEmpty()) {
        val showRangeEmpty = chartDateInputMode == ChartDateInputMode.RANGE
        Text(
            text = if (showRangeEmpty) {
                stringResource(R.string.report_chart_empty_in_range)
            } else {
                stringResource(R.string.report_chart_empty)
            },
            style = MaterialTheme.typography.bodyMedium,
            modifier = Modifier.fillMaxWidth()
        )
        return
    }

    ReportChartVisualModeSelector(
        chartVisualMode = chartVisualMode,
        onChartVisualModeChange = onChartVisualModeChange
    )

    ReportChartVisualizationHintSection(
        chartVisualMode = chartVisualMode,
        chartShowAverageLine = chartShowAverageLine,
        onChartShowAverageLineChange = onChartShowAverageLineChange,
        heatmapTomlConfig = heatmapTomlConfig,
        heatmapStylePreference = heatmapStylePreference,
        onHeatmapThemePolicyChange = onHeatmapThemePolicyChange,
        onHeatmapPaletteNameChange = onHeatmapPaletteNameChange,
        heatmapApplyMessage = heatmapApplyMessage
    )

    when (chartVisualMode) {
        ReportChartVisualMode.LINE -> {
            ReportLineChart(
                points = sortedChartPoints,
                selectedIndex = selectedPointIndex,
                averageDurationSeconds = chartAverageDurationSeconds,
                usesLegacyStatsFallback = chartUsesLegacyStatsFallback,
                showAverageLine = chartShowAverageLine,
                onPointSelected = onPointSelected,
                modifier = Modifier
                    .fillMaxWidth()
                    .height(220.dp)
                    .clip(MaterialTheme.shapes.medium)
            )
        }

        ReportChartVisualMode.BAR -> {
            ReportBarChart(
                points = sortedChartPoints,
                selectedIndex = selectedPointIndex,
                averageDurationSeconds = chartAverageDurationSeconds,
                usesLegacyStatsFallback = chartUsesLegacyStatsFallback,
                showAverageLine = chartShowAverageLine,
                onPointSelected = onPointSelected,
                modifier = Modifier
                    .fillMaxWidth()
                    .height(220.dp)
                    .clip(MaterialTheme.shapes.medium)
            )
        }

        ReportChartVisualMode.PIE -> {
            ReportPieChart(
                points = sortedChartPoints,
                selectedIndex = selectedPointIndex,
                onPointSelected = onPointSelected,
                modifier = Modifier
                    .fillMaxWidth()
                    .height(220.dp)
                    .clip(MaterialTheme.shapes.medium)
            )
        }

        ReportChartVisualMode.HEATMAP_MONTH -> {
            ReportHeatmapChart(
                points = sortedChartPoints,
                selectedIndex = selectedPointIndex,
                mode = ReportHeatmapMode.MONTH,
                heatmapTomlConfig = heatmapTomlConfig,
                heatmapStylePreference = heatmapStylePreference,
                isAppDarkThemeActive = isAppDarkThemeActive,
                onPointSelected = onPointSelected,
                modifier = Modifier
                    .fillMaxWidth()
                    .height(220.dp)
                    .clip(MaterialTheme.shapes.medium)
            )
        }

        ReportChartVisualMode.HEATMAP_YEAR -> {
            ReportHeatmapChart(
                points = sortedChartPoints,
                selectedIndex = selectedPointIndex,
                mode = ReportHeatmapMode.YEAR,
                heatmapTomlConfig = heatmapTomlConfig,
                heatmapStylePreference = heatmapStylePreference,
                isAppDarkThemeActive = isAppDarkThemeActive,
                onPointSelected = onPointSelected,
                modifier = Modifier
                    .fillMaxWidth()
                    .height(220.dp)
                    .clip(MaterialTheme.shapes.medium)
            )
        }
    }

    ReportChartVisualizationSummary(
        sortedChartPoints = sortedChartPoints,
        selectedPointIndex = selectedPointIndex,
        chartAverageDurationSeconds = chartAverageDurationSeconds,
        chartUsesLegacyStatsFallback = chartUsesLegacyStatsFallback,
        chartVisualMode = chartVisualMode
    )
}
