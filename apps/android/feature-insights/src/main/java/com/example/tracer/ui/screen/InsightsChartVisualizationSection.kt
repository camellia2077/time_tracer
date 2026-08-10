package com.example.tracer

import android.util.Log
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.insights.R

@Composable
internal fun InsightsChartVisualizationSection(
    chartError: String,
    chartLoading: Boolean,
    insightsMode: InsightsMode,
    sortedChartPoints: List<InsightsChartPoint>,
    chartFromDateIso: String?,
    chartToDateIso: String?,
    chartVisualMode: InsightsChartVisualMode,
    onChartVisualModeChange: (InsightsChartVisualMode) -> Unit,
    selectedPointIndex: Int,
    onPointSelected: (Int) -> Unit,
    chartAverageDurationSeconds: Long?,
    chartUsesLegacyStatsFallback: Boolean,
    chartShowAverageLine: Boolean,
    onChartShowAverageLineChange: (Boolean) -> Unit,
    heatmapTomlConfig: InsightsHeatmapTomlConfig,
    heatmapStylePreference: InsightsHeatmapStylePreference,
    onHeatmapThemePolicyChange: (InsightsHeatmapThemePolicy) -> Unit,
    onHeatmapPaletteNameChange: (String) -> Unit,
    heatmapApplyMessage: String,
    isAppDarkThemeActive: Boolean
) {
    // The ViewModel logs query completion. This companion UI log records the exact inputs the
    // composable receives after recomposition, distinguishing a rendering-state overwrite from
    // a query that actually returned no points during cold-start diagnosis.
    LaunchedEffect(chartLoading, chartError, insightsMode, sortedChartPoints) {
        runCatching {
            Log.i(
                "TracerInsightsChart",
                "trend render inputs; loading=$chartLoading mode=$insightsMode " +
                    "points=${sortedChartPoints.size} error=${chartError.ifBlank { "<none>" }}"
            )
        }
    }
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
        val showRangeEmpty = insightsMode == InsightsMode.RANGE
        Text(
            text = if (showRangeEmpty) {
                stringResource(R.string.insights_chart_empty_in_range)
            } else {
                stringResource(R.string.insights_chart_empty)
            },
            style = MaterialTheme.typography.bodyMedium,
            modifier = Modifier.fillMaxWidth()
        )
        return
    }

    val availableVisualModes = availableInsightsChartVisualModes(insightsMode)
    val selectedChartVisualMode = chartVisualMode.takeIf {
        it in availableVisualModes
    } ?: availableVisualModes.first()
    // Keep the selector's logical value separate from the renderer's value:
    // Range/Recent display one `Heatmap` choice but resolve it to a concrete
    // single- or multi-month heatmap based on the actual query window.
    val effectiveChartVisualMode = resolveInsightsChartVisualMode(
        insightsMode = insightsMode,
        requestedMode = selectedChartVisualMode,
        points = sortedChartPoints,
        fromDateIso = chartFromDateIso,
        toDateIso = chartToDateIso
    )

    InsightsChartVisualModeSelector(
        insightsMode = insightsMode,
        chartVisualMode = selectedChartVisualMode,
        onChartVisualModeChange = onChartVisualModeChange
    )

    InsightsChartVisualizationHintSection(
        chartVisualMode = effectiveChartVisualMode,
        chartShowAverageLine = chartShowAverageLine,
        onChartShowAverageLineChange = onChartShowAverageLineChange,
        heatmapTomlConfig = heatmapTomlConfig,
        heatmapStylePreference = heatmapStylePreference,
        onHeatmapThemePolicyChange = onHeatmapThemePolicyChange,
        onHeatmapPaletteNameChange = onHeatmapPaletteNameChange,
        heatmapApplyMessage = heatmapApplyMessage
    )

    when (effectiveChartVisualMode) {
        InsightsChartVisualMode.LINE -> {
            InsightsLineChart(
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

        InsightsChartVisualMode.BAR -> {
            InsightsBarChart(
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

        InsightsChartVisualMode.HEATMAP_MONTH -> {
            InsightsHeatmapChart(
                points = sortedChartPoints,
                selectedIndex = selectedPointIndex,
                mode = InsightsHeatmapMode.MONTH,
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

        InsightsChartVisualMode.HEATMAP_MULTI_MONTH -> {
            InsightsMultiMonthHeatmap(
                points = sortedChartPoints,
                selectedIndex = selectedPointIndex,
                insightsMode = insightsMode,
                heatmapTomlConfig = heatmapTomlConfig,
                heatmapStylePreference = heatmapStylePreference,
                isAppDarkThemeActive = isAppDarkThemeActive,
                onPointSelected = onPointSelected,
                modifier = Modifier.fillMaxWidth()
            )
        }
    }

    InsightsChartVisualizationSummary(
        sortedChartPoints = sortedChartPoints,
        selectedPointIndex = selectedPointIndex,
        chartAverageDurationSeconds = chartAverageDurationSeconds,
        chartUsesLegacyStatsFallback = chartUsesLegacyStatsFallback,
        chartVisualMode = effectiveChartVisualMode
    )
}
