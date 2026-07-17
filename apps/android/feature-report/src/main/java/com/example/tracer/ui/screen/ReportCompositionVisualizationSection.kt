package com.example.tracer

import android.util.Log
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ChevronRight
import androidx.compose.material3.Icon
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.MaterialTheme
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
import com.example.tracer.feature.report.R
import com.example.tracer.ui.components.TracerSegmentedButtonDefaults

internal enum class ReportCompositionMeasure {
    DURATION,
    FREQUENCY
}

private fun ReportCompositionMeasure.labelRes(): Int =
    when (this) {
        ReportCompositionMeasure.DURATION -> R.string.report_chart_measure_duration
        ReportCompositionMeasure.FREQUENCY -> R.string.report_chart_measure_frequency
    }

private fun formatFrequencyCount(value: Long): String = "${value.coerceAtLeast(0L)}×"

private const val REPORT_COMPOSITION_LOG_TAG = "TracerComposition"

private fun logReportCompositionInfo(message: String) {
    runCatching { Log.i(REPORT_COMPOSITION_LOG_TAG, message) }
}

@Composable
internal fun ReportCompositionVisualizationSection(
    chartError: String,
    reportMode: ReportMode,
    renderModel: CompositionChartRenderModel?,
    compositionVisualMode: ReportCompositionVisualMode,
    piePalettePreset: ReportPiePalettePreset,
    selectedItemIndex: Int,
    onItemSelected: (Int) -> Unit,
    onCompositionVisualModeChange: (ReportCompositionVisualMode) -> Unit
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

    val rootSlices = renderModel?.tree.orEmpty().toReportCompositionSlices()
    if (rootSlices.isEmpty()) {
        Text(
            text = if (reportMode == ReportMode.RANGE) {
                stringResource(R.string.report_chart_empty_in_range)
            } else {
                stringResource(R.string.report_chart_empty_composition)
            },
            style = MaterialTheme.typography.bodyMedium,
            modifier = Modifier.fillMaxWidth()
        )
        return
    }

    val effectiveVisualMode = compositionVisualMode
    var compositionMeasure by remember { mutableStateOf(ReportCompositionMeasure.DURATION) }
    var drilldownPath by remember(renderModel?.tree, reportMode) {
        mutableStateOf(emptyList<String>())
    }
    val drilldownNodes = remember(renderModel?.tree, drilldownPath) {
        resolveCompositionDrilldownNodes(renderModel?.tree.orEmpty(), drilldownPath)
    }
    val visibleSlices = drilldownNodes.toReportCompositionSlices(compositionMeasure)
    val nodesWithOccurrences = drilldownNodes.count {
        (it.occurrenceCount ?: 0L) > 0L
    }
    LaunchedEffect(compositionMeasure, drilldownPath, nodesWithOccurrences, visibleSlices) {
        if (compositionMeasure == ReportCompositionMeasure.FREQUENCY) {
            logReportCompositionInfo(
                "Frequency breakdown: pathDepth=${drilldownPath.size}, " +
                    "currentNodes=${drilldownNodes.size}, " +
                    "nodesWithOccurrences=$nodesWithOccurrences, " +
                    "visibleSlices=${visibleSlices.size}"
            )
        }
    }
    val compositionValueLabel: (Long) -> String = { value ->
        when (compositionMeasure) {
            ReportCompositionMeasure.DURATION -> formatDurationHoursMinutes(value)
            ReportCompositionMeasure.FREQUENCY -> formatFrequencyCount(value)
        }
    }
    val treemapValueLabel: (Long) -> String = { value ->
        when (compositionMeasure) {
            ReportCompositionMeasure.DURATION -> formatTreemapDurationHoursMinutes(value)
            ReportCompositionMeasure.FREQUENCY -> formatFrequencyCount(value)
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
        text = stringResource(R.string.report_label_chart_visual),
        style = MaterialTheme.typography.labelSmall,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
    ReportCompositionVisualModeSelector(
        compositionVisualMode = effectiveVisualMode,
        onCompositionVisualModeChange = onCompositionVisualModeChange
    )
    Text(
        text = stringResource(R.string.report_label_chart_measure),
        style = MaterialTheme.typography.labelSmall,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
    ReportCompositionMeasureSelector(
        compositionMeasure = compositionMeasure,
        onCompositionMeasureChange = { compositionMeasure = it }
    )

    Text(
        text = when (effectiveVisualMode) {
            ReportCompositionVisualMode.HORIZONTAL_BAR ->
                stringResource(R.string.report_chart_bar_drilldown_hint)
            ReportCompositionVisualMode.TREEMAP ->
                stringResource(R.string.report_chart_composition_treemap_hint)
            ReportCompositionVisualMode.PIE ->
                stringResource(R.string.report_chart_pie_drilldown_hint)
        },
        style = MaterialTheme.typography.labelSmall,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )

    if (drilldownPath.isNotEmpty()) {
        Text(
            text = stringResource(
                R.string.report_chart_pie_drilldown_path,
                drilldownPath.joinToString(" › ")
            ),
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.primary
        )
        TextButton(
            onClick = {
                drilldownPath = drilldownPath.dropLast(1)
                onItemSelected(-1)
            }
        ) {
            Text(
                text = stringResource(
                    R.string.report_chart_pie_drilldown_back,
                    drilldownPath.last()
                )
            )
        }
    }

    when (effectiveVisualMode) {
        ReportCompositionVisualMode.HORIZONTAL_BAR -> {
            ReportCompositionBarChart(
                slices = visibleSlices,
                palettePreset = piePalettePreset,
                selectedIndex = selectedItemIndex,
                onItemSelected = onVisibleSliceSelected,
                valueLabel = compositionValueLabel,
                modifier = Modifier
                    .fillMaxWidth()
                    .height(280.dp)
                    .clip(MaterialTheme.shapes.medium)
            )
        }
        ReportCompositionVisualMode.TREEMAP -> {
            var treemapLegendColors by remember(visibleSlices, piePalettePreset) {
                mutableStateOf<List<Color>>(emptyList())
            }
            ReportCompositionTreemapChart(
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
                valueLabel = treemapValueLabel,
                onSliceSelected = onVisibleSliceSelected
            )
        }
        ReportCompositionVisualMode.PIE -> {
            val pieSliceColors = rememberPieSliceColors(visibleSlices, piePalettePreset)
            ReportPieChart(
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
                valueLabel = compositionValueLabel,
                onSliceSelected = onVisibleSliceSelected
            )
        }
    }

    Text(
        text = stringResource(R.string.report_chart_pie_drilldown_summary),
        style = MaterialTheme.typography.bodySmall,
        color = MaterialTheme.colorScheme.primary
    )
    Text(
        text = when (compositionMeasure) {
            ReportCompositionMeasure.DURATION -> stringResource(
                R.string.report_chart_total_duration,
                formatDurationHoursMinutes(renderModel?.totalDurationSeconds ?: 0L)
            )
            ReportCompositionMeasure.FREQUENCY -> stringResource(
                R.string.report_chart_total_frequency,
                visibleSlices.sumOf { it.durationSeconds }
            )
        },
        style = MaterialTheme.typography.bodySmall,
        color = MaterialTheme.colorScheme.primary
    )
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(
            text = stringResource(
                R.string.report_chart_active_roots,
                renderModel?.activeRootCount ?: 0
            ),
            style = MaterialTheme.typography.bodySmall
        )
        Text(
            text = stringResource(
                R.string.report_chart_range_days,
                renderModel?.rangeDays ?: 0
            ),
            style = MaterialTheme.typography.bodySmall
        )
    }

    val selectedSlice = visibleSlices.getOrNull(selectedItemIndex)
    if (selectedSlice != null) {
        Text(
            text = when (compositionMeasure) {
                ReportCompositionMeasure.DURATION -> stringResource(
                    R.string.report_chart_composition_selected_detail,
                    selectedSlice.root,
                    formatDurationHoursMinutes(selectedSlice.durationSeconds),
                    selectedSlice.percent
                )
                ReportCompositionMeasure.FREQUENCY -> stringResource(
                    R.string.report_chart_composition_selected_frequency_detail,
                    selectedSlice.root,
                    selectedSlice.durationSeconds,
                    selectedSlice.percent
                )
            },
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.primary
        )
    }
}

@Composable
private fun CompositionSliceLegend(
    slices: List<ReportCompositionSlice>,
    colors: List<androidx.compose.ui.graphics.Color>,
    parentPath: List<String>,
    nodes: List<TreeNode>,
    valueLabel: (Long) -> String,
    onSliceSelected: (Int) -> Unit
) {
    Column(
        modifier = Modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(6.dp)
    ) {
        slices.forEachIndexed { index, slice ->
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .heightIn(min = 48.dp)
                    .clickable { onSliceSelected(index) }
                    .padding(horizontal = 4.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Box(
                    modifier = Modifier
                        .size(10.dp)
                        .background(
                            color = colors.getOrElse(index) { androidx.compose.ui.graphics.Color.Gray },
                            shape = CircleShape
                        )
                )
                Text(
                    text = (parentPath + slice.root).joinToString(" › "),
                    modifier = Modifier
                        .weight(1f)
                        .padding(start = 8.dp),
                    style = MaterialTheme.typography.bodySmall,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis
                )
                Text(
                    text = "${valueLabel(slice.durationSeconds)} · " +
                        String.format("%.1f%%", slice.percent),
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
                if (nodes.firstOrNull { it.name == slice.root }?.children?.isNotEmpty() == true) {
                    Icon(
                        imageVector = Icons.Filled.ChevronRight,
                        contentDescription = null,
                        modifier = Modifier.padding(start = 4.dp),
                        tint = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }
        }
    }
}

internal fun resolveCompositionDrilldownNodes(
    tree: List<TreeNode>,
    path: List<String>
): List<TreeNode> {
    var currentLevel = tree
    for (segment in path) {
        val selected = currentLevel.firstOrNull { it.name == segment } ?: return tree
        currentLevel = selected.children
    }
    return currentLevel
}

internal fun List<TreeNode>.toReportCompositionSlices(
    compositionMeasure: ReportCompositionMeasure = ReportCompositionMeasure.DURATION
): List<ReportCompositionSlice> {
    val totalValue = sumOf { node ->
        when (compositionMeasure) {
            ReportCompositionMeasure.DURATION -> node.durationSeconds ?: 0L
            ReportCompositionMeasure.FREQUENCY -> node.occurrenceCount ?: 0L
        }
    }.coerceAtLeast(0L)
    return mapNotNull { node ->
        val value = when (compositionMeasure) {
            ReportCompositionMeasure.DURATION -> node.durationSeconds
            ReportCompositionMeasure.FREQUENCY -> node.occurrenceCount
        }?.coerceAtLeast(0L) ?: return@mapNotNull null
        if (node.name.isBlank() || value <= 0L) {
            return@mapNotNull null
        }
        ReportCompositionSlice(
            root = node.name,
            durationSeconds = value,
            percent = if (totalValue > 0L) {
                value.toFloat() * 100f / totalValue.toFloat()
            } else {
                0f
            }
        )
    }.sortedWith(
        compareByDescending<ReportCompositionSlice> { it.durationSeconds }
            .thenBy { it.root }
    )
}

@Composable
private fun ReportCompositionMeasureSelector(
    compositionMeasure: ReportCompositionMeasure,
    onCompositionMeasureChange: (ReportCompositionMeasure) -> Unit
) {
    val measures = ReportCompositionMeasure.entries
    SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
        measures.forEachIndexed { index, item ->
            val selected = compositionMeasure == item
            SegmentedButton(
                shape = SegmentedButtonDefaults.itemShape(index, measures.size),
                onClick = { onCompositionMeasureChange(item) },
                selected = selected,
                colors = TracerSegmentedButtonDefaults.colors(),
                label = { Text(stringResource(item.labelRes())) }
            )
        }
    }
}

@Composable
private fun ReportCompositionVisualModeSelector(
    compositionVisualMode: ReportCompositionVisualMode,
    onCompositionVisualModeChange: (ReportCompositionVisualMode) -> Unit
) {
    val modes = ReportCompositionVisualMode.entries
    SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
        modes.forEachIndexed { index, item ->
            val selected = compositionVisualMode == item
            SegmentedButton(
                shape = SegmentedButtonDefaults.itemShape(
                    index = index,
                    count = modes.size
                ),
                onClick = { onCompositionVisualModeChange(item) },
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
