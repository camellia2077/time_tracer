package com.example.tracer

import android.util.Log
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material.icons.filled.ChevronRight
import androidx.compose.material3.Icon
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.insights.R
import com.example.tracer.ui.components.TracerSegmentedButtonDefaults

internal enum class InsightsCompositionMeasure {
    DURATION,
    FREQUENCY
}
internal fun InsightsCompositionMeasure.labelRes(): Int =
    when (this) {
        InsightsCompositionMeasure.DURATION -> R.string.insights_chart_measure_duration
        InsightsCompositionMeasure.FREQUENCY -> R.string.insights_chart_measure_frequency
    }

private fun formatFrequencyCount(value: Long): String = "${value.coerceAtLeast(0L)}×"

private const val INSIGHTS_COMPOSITION_LOG_TAG = "TracerComposition"

private fun logInsightsCompositionInfo(message: String) {
    runCatching { Log.i(INSIGHTS_COMPOSITION_LOG_TAG, message) }
}
@Composable
internal fun InsightsCompositionVisualizationSection(
    chartError: String,
    insightsMode: InsightsMode,
    renderModel: CompositionChartRenderModel?,
    compositionVisualMode: InsightsCompositionVisualMode,
    piePalettePreset: InsightsPiePalettePreset,
    selectedItemIndex: Int,
    onItemSelected: (Int) -> Unit,
    onCompositionVisualModeChange: (InsightsCompositionVisualMode) -> Unit
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

    val rootSlices = renderModel?.tree.orEmpty().toInsightsCompositionSlices()
    if (rootSlices.isEmpty()) {
        Text(
            text = if (insightsMode == InsightsMode.RANGE) {
                stringResource(R.string.insights_chart_empty_in_range)
            } else {
                stringResource(R.string.insights_chart_empty_composition)
            },
            style = MaterialTheme.typography.bodyMedium,
            modifier = Modifier.fillMaxWidth()
        )
        return
    }

    val effectiveVisualMode = compositionVisualMode
    var compositionMeasure by remember { mutableStateOf(InsightsCompositionMeasure.DURATION) }
    var drilldownPath by remember(renderModel?.tree, renderModel?.displayPath, insightsMode) {
        mutableStateOf(renderModel?.displayPath.orEmpty())
    }
    val drilldownNodes = remember(renderModel?.tree, drilldownPath) {
        resolveCompositionDrilldownNodes(renderModel?.tree.orEmpty(), drilldownPath)
    }
    val visibleSlices = drilldownNodes.toInsightsCompositionSlices(
        compositionMeasure = compositionMeasure
    )
    val nodesWithOccurrences = drilldownNodes.count {
        (it.occurrenceCount ?: 0L) > 0L
    }
    LaunchedEffect(compositionMeasure, drilldownPath, nodesWithOccurrences, visibleSlices) {
        if (compositionMeasure == InsightsCompositionMeasure.FREQUENCY) {
            logInsightsCompositionInfo(
                "Frequency breakdown: pathDepth=${drilldownPath.size}, " +
                    "currentNodes=${drilldownNodes.size}, " +
                    "nodesWithOccurrences=$nodesWithOccurrences, " +
                    "visibleSlices=${visibleSlices.size}"
            )
        }
    }
    val treemapValueLabel: (Long) -> String = { value ->
        when (compositionMeasure) {
            InsightsCompositionMeasure.DURATION -> formatTreemapDurationHoursMinutes(value)
            InsightsCompositionMeasure.FREQUENCY -> formatFrequencyCount(value)
        }
    }
    val onVisibleSliceSelected: (Int) -> Unit = { index ->
        val selectedNode = visibleSlices
            .getOrNull(index)
            ?.root
            ?.let { root -> drilldownNodes.firstOrNull { it.name == root } }
        onItemSelected(index)
        if (selectedNode != null && selectedNode.children.isNotEmpty()) {
            drilldownPath = drilldownPath + selectedNode.name
            onItemSelected(-1)
        }
    }
    Text(
        text = stringResource(R.string.insights_label_chart_visual),
        style = MaterialTheme.typography.labelSmall,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
    InsightsCompositionVisualModeSelector(
        compositionVisualMode = effectiveVisualMode,
        onCompositionVisualModeChange = onCompositionVisualModeChange
    )
    Text(
        text = stringResource(R.string.insights_label_chart_measure),
        style = MaterialTheme.typography.labelSmall,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
    InsightsCompositionMeasureSelector(
        compositionMeasure = compositionMeasure,
        onCompositionMeasureChange = { compositionMeasure = it }
    )

    Text(
        text = when (effectiveVisualMode) {
            InsightsCompositionVisualMode.HORIZONTAL_BAR ->
                stringResource(R.string.insights_chart_bar_drilldown_hint)
            InsightsCompositionVisualMode.TREEMAP ->
                stringResource(R.string.insights_chart_composition_treemap_hint)
            InsightsCompositionVisualMode.PIE ->
                stringResource(R.string.insights_chart_pie_drilldown_hint)
        },
        style = MaterialTheme.typography.labelSmall,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )

    if (drilldownPath.isNotEmpty()) {
        Spacer(modifier = Modifier.height(8.dp))
        Column(
            modifier = Modifier.fillMaxWidth(),
            verticalArrangement = Arrangement.spacedBy(0.dp)
        ) {
            Text(
                text = stringResource(R.string.insights_chart_pie_drilldown_path_label),
                style = MaterialTheme.typography.labelMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            Row(
                verticalAlignment = Alignment.CenterVertically
            ) {
                drilldownPath.forEachIndexed { index, segment ->
                    TextButton(
                        onClick = {
                            drilldownPath = drilldownPath.take(index + 1)
                            onItemSelected(-1)
                        },
                        contentPadding = PaddingValues(horizontal = 1.dp, vertical = 0.dp)
                    ) {
                        Text(
                            text = segment,
                            style = MaterialTheme.typography.bodyMedium,
                            maxLines = 1,
                            overflow = TextOverflow.Ellipsis
                        )
                    }
                    if (index < drilldownPath.lastIndex) {
                        Icon(
                            imageVector = Icons.Filled.ChevronRight,
                            contentDescription = null,
                            modifier = Modifier.size(18.dp),
                            tint = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    }
                }
            }
            OutlinedButton(
                onClick = {
                    drilldownPath = drilldownPath.dropLast(1)
                    onItemSelected(-1)
                }
            ) {
                Icon(
                    imageVector = Icons.Filled.ArrowBack,
                    contentDescription = null
                )
                Text(
                    text = stringResource(
                        R.string.insights_chart_pie_drilldown_back,
                        drilldownPath.last()
                    )
                )
            }
        }
    }

    HorizontalDivider(
        modifier = Modifier.padding(vertical = 4.dp),
        color = MaterialTheme.colorScheme.outlineVariant
    )

    Text(
        text = when (compositionMeasure) {
            InsightsCompositionMeasure.DURATION -> stringResource(
                R.string.insights_chart_total_duration,
                formatDurationHoursMinutes(visibleSlices.sumOf { it.measureValue })
            )
            InsightsCompositionMeasure.FREQUENCY -> stringResource(
                R.string.insights_chart_total_frequency,
                visibleSlices.sumOf { it.measureValue }
            )
        },
        style = MaterialTheme.typography.bodySmall,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )

    when (effectiveVisualMode) {
        InsightsCompositionVisualMode.HORIZONTAL_BAR -> {
            InsightsCompositionBarChart(
                slices = visibleSlices,
                palettePreset = piePalettePreset,
                onItemSelected = onVisibleSliceSelected,
                showAverage = insightsMode != InsightsMode.DAY,
                showFrequency = compositionMeasure == InsightsCompositionMeasure.FREQUENCY,
                modifier = Modifier
                    .fillMaxWidth()
                    .clip(MaterialTheme.shapes.medium)
            )
        }
        InsightsCompositionVisualMode.TREEMAP -> {
            var treemapLegendColors by remember(visibleSlices, piePalettePreset) {
                mutableStateOf<List<Color>>(emptyList())
            }
            InsightsCompositionTreemapChart(
                slices = visibleSlices,
                palettePreset = piePalettePreset,
                selectedIndex = selectedItemIndex,
                onItemSelected = onVisibleSliceSelected,
                onSliceColorsResolved = { treemapLegendColors = it },
                valueLabel = treemapValueLabel,
                modifier = Modifier
                    .fillMaxWidth()
                    .height(260.dp)
                    .clip(MaterialTheme.shapes.medium)
            )
            CompositionSliceLegend(
                slices = visibleSlices,
                colors = treemapLegendColors,
                parentPath = drilldownPath,
                nodes = drilldownNodes,
                showAverage = insightsMode != InsightsMode.DAY,
                showFrequency = compositionMeasure == InsightsCompositionMeasure.FREQUENCY,
                onSliceSelected = onVisibleSliceSelected
            )
        }
        InsightsCompositionVisualMode.PIE -> {
            val pieSliceColors = rememberPieSliceColors(visibleSlices, piePalettePreset)
            InsightsPieChart(
                slices = visibleSlices,
                palettePreset = piePalettePreset,
                selectedIndex = selectedItemIndex,
                sliceColors = pieSliceColors,
                onSliceSelected = onVisibleSliceSelected,
                modifier = Modifier
                    .fillMaxWidth()
                    .height(220.dp)
                    .clip(MaterialTheme.shapes.medium)
            )
            CompositionSliceLegend(
                slices = visibleSlices,
                colors = pieSliceColors,
                parentPath = drilldownPath,
                nodes = drilldownNodes,
                showAverage = insightsMode != InsightsMode.DAY,
                showFrequency = compositionMeasure == InsightsCompositionMeasure.FREQUENCY,
                onSliceSelected = onVisibleSliceSelected
            )
        }
    }

}
