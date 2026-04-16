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
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.report.R
import com.example.tracer.ui.components.TracerSegmentedButtonDefaults
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
internal fun ReportChartResultContent(
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
    onLoadChart: () -> Unit,
    modifier: Modifier = Modifier
) {
    val normalizedSemanticMode = chartSemanticMode.normalizeForReportMode(reportMode)
    val allowTrendChart = reportMode != ReportMode.DAY
    val normalizedRoots = remember(trendChartRoots) { trendChartRoots.distinct() }
    val rootOptions = remember(normalizedRoots) { listOf("") + normalizedRoots }
    val sortedChartPoints = remember(trendChartRenderModel) {
        trendChartRenderModel?.points
            ?.sortedWith(
                compareBy<ReportChartPoint>(
                    { it.epochDay ?: parseEpochDayOrNull(it.date) ?: Long.MAX_VALUE },
                    { it.date }
                )
            )
            ?: emptyList()
    }
    val chartAverageDurationSeconds = trendChartRenderModel?.averageDurationSeconds
    val chartUsesLegacyStatsFallback = trendChartRenderModel?.usesLegacyStatsFallback == true
    val compositionSlices = compositionChartRenderModel?.slices ?: emptyList()
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
    var chartVisualMode by rememberSaveable { mutableStateOf(ReportChartVisualMode.LINE) }
    val effectiveCompositionVisualMode = compositionVisualMode

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

        if (allowTrendChart) {
            ReportChartSemanticModeSelector(
                chartSemanticMode = normalizedSemanticMode,
                onChartSemanticModeChange = onChartSemanticModeChange
            )
        }

        ReportChartParameterSection(
            chartSemanticMode = normalizedSemanticMode,
            rootOptions = rootOptions,
            trendChartSelectedRoot = trendChartSelectedRoot,
            chartLoading = if (normalizedSemanticMode == ReportChartSemanticMode.TREND) {
                trendChartLoading
            } else {
                compositionChartLoading
            },
            chartLastTrace = if (normalizedSemanticMode == ReportChartSemanticMode.TREND) {
                trendChartLastTrace
            } else {
                compositionChartLastTrace
            },
            onChartRootChange = onChartRootChange,
            onLoadChart = onLoadChart
        )

        if (normalizedSemanticMode == ReportChartSemanticMode.TREND) {
            ReportChartVisualizationSection(
                chartError = trendChartError,
                reportMode = reportMode,
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
        } else {
            ReportCompositionVisualizationSection(
                chartError = compositionChartError,
                reportMode = reportMode,
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
private fun ReportChartSemanticModeSelector(
    chartSemanticMode: ReportChartSemanticMode,
    onChartSemanticModeChange: (ReportChartSemanticMode) -> Unit
) {
    Text(
        text = stringResource(R.string.report_label_chart_semantic),
        style = MaterialTheme.typography.labelSmall,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
    val modes = ReportChartSemanticMode.entries
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
