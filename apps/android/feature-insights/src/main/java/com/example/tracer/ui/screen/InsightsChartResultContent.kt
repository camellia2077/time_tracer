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
    trendChartLoading: Boolean,
    trendChartError: String,
    trendChartRenderModel: ChartRenderModel?,
    trendChartLastTrace: ChartQueryTrace?,
    compositionChartLoading: Boolean,
    compositionChartError: String,
    compositionChartRenderModel: CompositionChartRenderModel?,
    compositionChartLastTrace: ChartQueryTrace?,
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
    val rootOptions = remember(normalizedRoots) { listOf("") + normalizedRoots }
    val sortedChartPoints = remember(trendChartRenderModel) {
        trendChartRenderModel?.points
            ?.sortedWith(
                compareBy<InsightsChartPoint>(
                    { it.epochDay ?: parseEpochDayOrNull(it.date) ?: Long.MAX_VALUE },
                    { it.date }
                )
            )
            ?: emptyList()
    }
    val chartAverageDurationSeconds = trendChartRenderModel?.averageDurationSeconds
    val chartUsesLegacyStatsFallback = trendChartRenderModel?.usesLegacyStatsFallback == true
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
    val effectiveCompositionVisualMode = compositionVisualMode

    Column(
        modifier = modifier
            .fillMaxWidth()
            .padding(top = 4.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        InsightsChartParameterSection(
            chartSemanticMode = normalizedSemanticMode,
            rootOptions = rootOptions,
            trendChartSelectedRoot = trendChartSelectedRoot,
            onChartRootChange = onChartRootChange
        )

        if (normalizedSemanticMode == InsightsChartSemanticMode.TREND) {
            InsightsChartVisualizationSection(
                chartError = trendChartError,
                chartLoading = trendChartLoading,
                insightsMode = insightsMode,
                sortedChartPoints = sortedChartPoints,
                chartFromDateIso = trendChartRenderModel?.fromDateIso,
                chartToDateIso = trendChartRenderModel?.toDateIso,
                chartVisualMode = chartVisualMode,
                onChartVisualModeChange = onChartVisualModeChange,
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
