package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.report.R
import com.example.tracer.ui.components.TracerSegmentedButtonDefaults

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

    val slices = renderModel?.slices ?: emptyList()
    if (slices.isEmpty()) {
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

    val effectiveVisualMode = compositionVisualMode.normalizeForReportMode(reportMode)
    Text(
        text = stringResource(R.string.report_label_chart_visual),
        style = MaterialTheme.typography.labelSmall,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
    ReportCompositionVisualModeSelector(
        reportMode = reportMode,
        compositionVisualMode = effectiveVisualMode,
        onCompositionVisualModeChange = onCompositionVisualModeChange
    )

    Text(
        text = when (effectiveVisualMode) {
            ReportCompositionVisualMode.HORIZONTAL_BAR ->
                stringResource(R.string.report_chart_composition_bar_hint)
            ReportCompositionVisualMode.TREEMAP ->
                stringResource(R.string.report_chart_composition_treemap_hint)
            ReportCompositionVisualMode.PIE ->
                stringResource(R.string.report_chart_pie_hint)
        },
        style = MaterialTheme.typography.labelSmall,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )

    when (effectiveVisualMode) {
        ReportCompositionVisualMode.HORIZONTAL_BAR -> {
            ReportCompositionBarChart(
                slices = slices,
                palettePreset = piePalettePreset,
                selectedIndex = selectedItemIndex,
                onItemSelected = onItemSelected,
                modifier = Modifier
                    .fillMaxWidth()
                    .height(280.dp)
                    .clip(MaterialTheme.shapes.medium)
            )
        }
        ReportCompositionVisualMode.TREEMAP -> {
            ReportCompositionTreemapChart(
                slices = slices,
                palettePreset = piePalettePreset,
                selectedIndex = selectedItemIndex,
                onItemSelected = onItemSelected,
                modifier = Modifier
                    .fillMaxWidth()
                    .height(260.dp)
                    .clip(MaterialTheme.shapes.medium)
            )
        }
        ReportCompositionVisualMode.PIE -> {
            ReportPieChart(
                slices = slices,
                palettePreset = piePalettePreset,
                selectedIndex = selectedItemIndex,
                onSliceSelected = onItemSelected,
                modifier = Modifier
                    .fillMaxWidth()
                    .height(220.dp)
                    .clip(MaterialTheme.shapes.medium)
            )
        }
    }

    Text(
        text = stringResource(R.string.report_chart_composition_summary),
        style = MaterialTheme.typography.bodySmall,
        color = MaterialTheme.colorScheme.primary
    )
    Text(
        text = stringResource(
            R.string.report_chart_total_duration,
            formatDurationHoursMinutes(renderModel?.totalDurationSeconds ?: 0L)
        ),
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

    val selectedSlice = slices.getOrNull(selectedItemIndex)
    if (selectedSlice != null) {
        Text(
            text = stringResource(
                R.string.report_chart_composition_selected_detail,
                selectedSlice.root,
                formatDurationHoursMinutes(selectedSlice.durationSeconds),
                selectedSlice.percent
            ),
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.primary
        )
    }
}

@Composable
private fun ReportCompositionVisualModeSelector(
    reportMode: ReportMode,
    compositionVisualMode: ReportCompositionVisualMode,
    onCompositionVisualModeChange: (ReportCompositionVisualMode) -> Unit
) {
    val modes = buildList {
        add(ReportCompositionVisualMode.PIE)
        add(ReportCompositionVisualMode.HORIZONTAL_BAR)
        if (reportMode == ReportMode.DAY) {
            add(ReportCompositionVisualMode.TREEMAP)
        }
    }
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
