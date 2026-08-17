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
import com.example.tracer.feature.insights.R
import com.example.tracer.ui.components.CalendarAvailability

@Composable
internal fun InsightsChartVisualizationSection(
    chartError: String,
    insightsMode: InsightsMode,
    sortedChartPoints: List<InsightsChartPoint>,
    rawSortedChartPoints: List<InsightsChartPoint>,
    useMonthlyChartAggregation: Boolean,
    chartFromDateIso: String?,
    chartToDateIso: String?,
    chartVisualMode: InsightsChartVisualMode,
    onChartVisualModeChange: (InsightsChartVisualMode) -> Unit,
    selectedPointIndex: Int,
    onPointSelected: (Int) -> Unit,
    chartAverageDurationSeconds: Long,
    comparisonChartPoints: List<InsightsChartPoint>,
    comparisonPeriodLabel: String,
    periodComparison: InsightsPeriodComparisonState,
    canComparePreviousPeriod: Boolean,
    calendarAvailability: CalendarAvailability,
    onPeriodComparisonToggle: () -> Unit,
    onComparisonPeriodSelected: (InsightsPeriodSelection) -> Unit,
    chartTotalOccurrenceCount: Long,
    chartTotalDurationSeconds: Long,
    chartAverageDurationPerOccurrenceSeconds: Long,
    chartModeDurationSeconds: Double?,
    chartMedianDurationSeconds: Double?,
    chartMinimumDurationSeconds: Double?,
    chartMaximumDurationSeconds: Double?,
    chartLowerQuartileDurationSeconds: Double?,
    chartUpperQuartileDurationSeconds: Double?,
    chartCoefficientOfVariation: Double?,
    chartMeanAbsoluteDeviationSeconds: Double?,
    chartShowAverageLine: Boolean,
    onChartShowAverageLineChange: (Boolean) -> Unit,
    heatmapTomlConfig: InsightsHeatmapTomlConfig,
    heatmapStylePreference: InsightsHeatmapStylePreference,
    onHeatmapThemePolicyChange: (InsightsHeatmapThemePolicy) -> Unit,
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
        points = rawSortedChartPoints,
        fromDateIso = chartFromDateIso,
        toDateIso = chartToDateIso
    )
    val chartVisualizationAverageDurationSeconds = if (useMonthlyChartAggregation) {
        sortedChartPoints.map { it.durationSeconds }.average().toLong()
    } else {
        chartAverageDurationSeconds
    }

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

    val comparisonVisible = insightsMode != InsightsMode.YEAR &&
        effectiveChartVisualMode in setOf(
            InsightsChartVisualMode.LINE,
            InsightsChartVisualMode.BAR
        )
    if (comparisonVisible) {
        InsightsPeriodComparisonControl(
            periodComparison = periodComparison,
            canComparePreviousPeriod = canComparePreviousPeriod,
            insightsMode = insightsMode,
            calendarAvailability = calendarAvailability,
            onPeriodComparisonToggle = onPeriodComparisonToggle,
            onComparisonPeriodSelected = onComparisonPeriodSelected
        )
    }

    when (effectiveChartVisualMode) {
        InsightsChartVisualMode.LINE -> {
            InsightsLineChart(
                points = sortedChartPoints,
                comparisonPoints = if (comparisonVisible) comparisonChartPoints else emptyList(),
                comparisonPeriodLabel = comparisonPeriodLabel,
                selectedIndex = selectedPointIndex,
                averageDurationSeconds = chartVisualizationAverageDurationSeconds,
                showAverageLine = chartShowAverageLine,
                onPointSelected = onPointSelected,
                modifier = Modifier
                    .fillMaxWidth()
                    .clip(MaterialTheme.shapes.medium)
            )
        }

        InsightsChartVisualMode.BAR -> {
            InsightsBarChart(
                points = sortedChartPoints,
                comparisonPoints = if (comparisonVisible) comparisonChartPoints else emptyList(),
                comparisonPeriodLabel = comparisonPeriodLabel,
                selectedIndex = selectedPointIndex,
                averageDurationSeconds = chartVisualizationAverageDurationSeconds,
                showAverageLine = chartShowAverageLine,
                onPointSelected = onPointSelected,
                modifier = Modifier
                    .fillMaxWidth()
                    .clip(MaterialTheme.shapes.medium)
            )
        }

        InsightsChartVisualMode.HEATMAP_MONTH -> {
            InsightsHeatmapChart(
                points = rawSortedChartPoints,
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
                points = rawSortedChartPoints,
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
        recordedPoints = rawSortedChartPoints,
        selectedPointIndex = selectedPointIndex,
        chartFromDateIso = chartFromDateIso,
        chartToDateIso = chartToDateIso,
        chartAverageDurationSeconds = chartAverageDurationSeconds,
        chartTotalOccurrenceCount = chartTotalOccurrenceCount,
        chartTotalDurationSeconds = chartTotalDurationSeconds,
        chartAverageDurationPerOccurrenceSeconds =
            chartAverageDurationPerOccurrenceSeconds,
        chartModeDurationSeconds = chartModeDurationSeconds,
        chartMedianDurationSeconds = chartMedianDurationSeconds,
        chartMinimumDurationSeconds = chartMinimumDurationSeconds,
        chartMaximumDurationSeconds = chartMaximumDurationSeconds,
        chartLowerQuartileDurationSeconds = chartLowerQuartileDurationSeconds,
        chartUpperQuartileDurationSeconds = chartUpperQuartileDurationSeconds,
        chartCoefficientOfVariation = chartCoefficientOfVariation,
        chartMeanAbsoluteDeviationSeconds = chartMeanAbsoluteDeviationSeconds,
        chartVisualMode = effectiveChartVisualMode
    )
}
