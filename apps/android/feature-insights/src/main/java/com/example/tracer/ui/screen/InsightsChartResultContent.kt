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
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.insights.R
import com.example.tracer.ui.components.CalendarAvailability
import com.example.tracer.ui.components.TracerSegmentedButtonDefaults
import java.time.LocalDate

@Composable
fun InsightsResultModeSwitcher(
    mode: InsightsResultDisplayMode,
    onModeChange: (InsightsResultDisplayMode) -> Unit,
    modifier: Modifier = Modifier
) {
    val modes = InsightsResultDisplayMode.entries
    SingleChoiceSegmentedButtonRow(modifier = modifier.fillMaxWidth()) {
        modes.forEachIndexed { index, item ->
            val selected = mode == item
            SegmentedButton(
                shape = SegmentedButtonDefaults.itemShape(
                    index = index,
                    count = modes.size
                ),
                onClick = { onModeChange(item) },
                selected = selected,
                colors = TracerSegmentedButtonDefaults.colors(),
                label = {
                    Text(
                        text = stringResource(item.labelRes()),
                        fontWeight = if (selected) {
                            TracerSegmentedButtonDefaults.activeLabelFontWeight
                        } else {
                            TracerSegmentedButtonDefaults.inactiveLabelFontWeight
                        }
                    )
                }
            )
        }
    }
}

@Composable
internal fun InsightsChartResultContent(
    chartSemanticMode: InsightsChartSemanticMode,
    chartVisualMode: InsightsChartVisualMode,
    compositionVisualMode: InsightsCompositionVisualMode,
    trendChartRoots: List<String>,
    trendChartSelectedRoot: String,
    insightsMode: InsightsMode,
    calendarAvailability: CalendarAvailability,
    trendChartError: String,
    trendChartRenderModel: ChartRenderModel?,
    trendChartComparison: InsightsPeriodComparisonState,
    canCompareChartPreviousPeriod: Boolean,
    onChartPeriodComparisonToggle: () -> Unit,
    onChartComparisonPeriodSelected: (InsightsPeriodSelection) -> Unit,
    compositionChartError: String,
    compositionChartRenderModel: CompositionChartRenderModel?,
    chartShowAverageLine: Boolean,
    piePalettePreset: InsightsPiePalettePreset,
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
    modifier: Modifier = Modifier
) {
    val normalizedSemanticMode = chartSemanticMode.normalizeForInsightsMode(insightsMode)
    val normalizedRoots = remember(trendChartRoots) { trendChartRoots.distinct() }
    val chartRootTree = remember(trendChartRenderModel, normalizedRoots) {
        trendChartRenderModel?.rootTree?.takeIf { it.isNotEmpty() }
            ?: normalizedRoots.map { root -> TreeNode(name = root, path = root) }
    }
    val rawSortedChartPoints = remember(trendChartRenderModel) {
        trendChartRenderModel?.points?.sortedChartPoints() ?: emptyList()
    }
    val useMonthlyChartAggregation = shouldAggregateChartPointsByMonth(
        insightsMode = insightsMode,
        fromDateIso = trendChartRenderModel?.fromDateIso,
        toDateIso = trendChartRenderModel?.toDateIso
    )
    val sortedChartPoints = remember(
        insightsMode,
        rawSortedChartPoints,
        useMonthlyChartAggregation,
        trendChartRenderModel?.fromDateIso,
        trendChartRenderModel?.toDateIso
    ) {
        if (useMonthlyChartAggregation) {
            aggregateYearChartPoints(
                points = rawSortedChartPoints,
                fromDateIso = trendChartRenderModel?.fromDateIso,
                toDateIso = trendChartRenderModel?.toDateIso
            )
        } else {
            rawSortedChartPoints
        }
    }
    val comparisonChartModel = (trendChartComparison as? InsightsPeriodComparisonState.Ready)
        ?.chartRenderModel
    val sortedComparisonChartPoints = remember(comparisonChartModel) {
        comparisonChartModel?.points?.sortedChartPoints() ?: emptyList()
    }
    val chartAverageDurationSeconds = trendChartRenderModel?.averageDurationSeconds ?: 0L
    val chartTotalOccurrenceCount = trendChartRenderModel?.totalOccurrenceCount ?: 0L
    val chartTotalDurationSeconds = trendChartRenderModel?.totalDurationSeconds ?: 0L
    val chartAverageDurationPerOccurrenceSeconds =
        trendChartRenderModel?.averageDurationPerOccurrenceSeconds ?: 0L
    val chartModeDurationSeconds = trendChartRenderModel?.modeDurationSeconds
    val chartMedianDurationSeconds = trendChartRenderModel?.medianDurationSeconds
    val chartMinimumDurationSeconds = trendChartRenderModel?.minimumDurationSeconds
    val chartMaximumDurationSeconds = trendChartRenderModel?.maximumDurationSeconds
    val chartLowerQuartileDurationSeconds = trendChartRenderModel?.lowerQuartileDurationSeconds
    val chartUpperQuartileDurationSeconds = trendChartRenderModel?.upperQuartileDurationSeconds
    val chartCoefficientOfVariation = trendChartRenderModel?.coefficientOfVariation
    val chartMeanAbsoluteDeviationSeconds = trendChartRenderModel?.meanAbsoluteDeviationSeconds
    val compositionSlices = compositionChartRenderModel?.tree.orEmpty().toInsightsCompositionSlices()
    var selectedPointIndex by remember(sortedChartPoints) {
        mutableIntStateOf(
            if (sortedChartPoints.isEmpty()) {
                -1
            } else {
                sortedChartPoints.lastIndex
            }
        )
    }
    var selectedItemIndex by remember(compositionSlices) {
        mutableIntStateOf(
            if (compositionSlices.isEmpty()) {
                -1
            } else {
                0
            }
        )
    }
    var rootPickerVisible by remember { mutableStateOf(false) }
    val effectiveCompositionVisualMode = compositionVisualMode

    if (rootPickerVisible) {
        InsightsChartRootPickerDialog(
            rootNodes = chartRootTree,
            selectedPath = trendChartSelectedRoot,
            onPathSelected = { path ->
                onChartRootChange(path)
                rootPickerVisible = false
            },
            onDismiss = { rootPickerVisible = false }
        )
    }

    Column(
        modifier = modifier
            .fillMaxWidth()
            .padding(top = 4.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        InsightsChartParameterSection(
            chartSemanticMode = normalizedSemanticMode,
            rootTree = chartRootTree,
            trendChartSelectedRoot = trendChartSelectedRoot,
            onOpenRootPicker = { rootPickerVisible = true }
        )

        if (normalizedSemanticMode == InsightsChartSemanticMode.TREND) {
            InsightsChartVisualizationSection(
                chartError = trendChartError,
                insightsMode = insightsMode,
                sortedChartPoints = sortedChartPoints,
                rawSortedChartPoints = rawSortedChartPoints,
                useMonthlyChartAggregation = useMonthlyChartAggregation,
                chartFromDateIso = trendChartRenderModel?.fromDateIso,
                chartToDateIso = trendChartRenderModel?.toDateIso,
                chartVisualMode = chartVisualMode,
                onChartVisualModeChange = onChartVisualModeChange,
                selectedPointIndex = selectedPointIndex,
                onPointSelected = { selectedPointIndex = it },
                chartAverageDurationSeconds = chartAverageDurationSeconds,
                comparisonChartPoints = sortedComparisonChartPoints,
                comparisonPeriodLabel = (trendChartComparison as?
                    InsightsPeriodComparisonState.Ready)?.label.orEmpty(),
                periodComparison = trendChartComparison,
                canComparePreviousPeriod = canCompareChartPreviousPeriod,
                calendarAvailability = calendarAvailability,
                onPeriodComparisonToggle = onChartPeriodComparisonToggle,
                onComparisonPeriodSelected = onChartComparisonPeriodSelected,
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
                chartShowAverageLine = chartShowAverageLine,
                onChartShowAverageLineChange = onChartShowAverageLineChange,
                heatmapTomlConfig = heatmapTomlConfig,
                heatmapStylePreference = heatmapStylePreference,
                onHeatmapThemePolicyChange = onHeatmapThemePolicyChange,
                onHeatmapPaletteNameChange = onHeatmapPaletteNameChange,
                heatmapApplyMessage = heatmapApplyMessage,
                isAppDarkThemeActive = isAppDarkThemeActive
            )
        } else {
            InsightsCompositionVisualizationSection(
                chartError = compositionChartError,
                insightsMode = insightsMode,
                renderModel = compositionChartRenderModel,
                compositionVisualMode = effectiveCompositionVisualMode,
                piePalettePreset = piePalettePreset,
                selectedItemIndex = selectedItemIndex,
                onItemSelected = { selectedItemIndex = it },
                onCompositionVisualModeChange = onCompositionVisualModeChange
            )
        }
    }
}

private fun List<InsightsChartPoint>.sortedChartPoints(): List<InsightsChartPoint> =
    sortedWith(
        compareBy<InsightsChartPoint>(
            { it.epochDay ?: parseEpochDayOrNull(it.date) ?: Long.MAX_VALUE },
            { it.date }
        )
    )

@Composable
internal fun InsightsChartSemanticModeSelector(
    chartSemanticMode: InsightsChartSemanticMode,
    onChartSemanticModeChange: (InsightsChartSemanticMode) -> Unit
) {
    val modes = InsightsChartSemanticMode.entries
    SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
        modes.forEachIndexed { index, item ->
            val selected = chartSemanticMode == item
            SegmentedButton(
                shape = SegmentedButtonDefaults.itemShape(
                    index = index,
                    count = modes.size
                ),
                onClick = { onChartSemanticModeChange(item) },
                selected = selected,
                colors = TracerSegmentedButtonDefaults.colors(),
                label = {
                    Text(
                        text = stringResource(item.labelRes()),
                        fontWeight = if (selected) {
                            TracerSegmentedButtonDefaults.activeLabelFontWeight
                        } else {
                            TracerSegmentedButtonDefaults.inactiveLabelFontWeight
                        }
                    )
                }
            )
        }
    }
}

private fun parseEpochDayOrNull(dateIso: String): Long? =
    try {
        LocalDate.parse(dateIso).toEpochDay()
    } catch (_: Exception) {
        null
    }
