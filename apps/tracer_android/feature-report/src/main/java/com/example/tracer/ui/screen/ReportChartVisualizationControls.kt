package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextOverflow
import com.example.tracer.feature.report.R

@Composable
internal fun ReportChartVisualModeSelector(
    chartVisualMode: ReportChartVisualMode,
    onChartVisualModeChange: (ReportChartVisualMode) -> Unit
) {
    Text(
        text = stringResource(R.string.report_label_chart_visual),
        style = MaterialTheme.typography.labelSmall,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
    SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
        val visualModes = ReportChartVisualMode.entries
        visualModes.forEachIndexed { index, visualMode ->
            SegmentedButton(
                shape = SegmentedButtonDefaults.itemShape(
                    index = index,
                    count = visualModes.size
                ),
                onClick = { onChartVisualModeChange(visualMode) },
                selected = chartVisualMode == visualMode,
                icon = {},
                label = {
                    Text(
                        text = stringResource(visualMode.labelRes()),
                        maxLines = 1,
                        softWrap = false,
                        overflow = TextOverflow.Ellipsis
                    )
                }
            )
        }
    }
}

@Composable
internal fun ReportChartVisualizationHintSection(
    chartVisualMode: ReportChartVisualMode,
    chartShowAverageLine: Boolean,
    onChartShowAverageLineChange: (Boolean) -> Unit,
    heatmapTomlConfig: ReportHeatmapTomlConfig,
    heatmapStylePreference: ReportHeatmapStylePreference,
    onHeatmapThemePolicyChange: (ReportHeatmapThemePolicy) -> Unit,
    onHeatmapPaletteNameChange: (String) -> Unit,
    heatmapApplyMessage: String
) {
    when (chartVisualMode) {
        ReportChartVisualMode.PIE -> {
            Text(
                text = stringResource(R.string.report_chart_pie_hint),
                style = MaterialTheme.typography.labelSmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
        }

        ReportChartVisualMode.HEATMAP_MONTH,
        ReportChartVisualMode.HEATMAP_YEAR -> {
            ReportChartHeatmapSettings(
                heatmapTomlConfig = heatmapTomlConfig,
                heatmapStylePreference = heatmapStylePreference,
                onHeatmapThemePolicyChange = onHeatmapThemePolicyChange,
                onHeatmapPaletteNameChange = onHeatmapPaletteNameChange,
                heatmapApplyMessage = heatmapApplyMessage
            )
        }

        else -> {
            Text(
                text = "${stringResource(R.string.report_chart_axis_x_date)} · " +
                    stringResource(R.string.report_chart_axis_y_hours),
                style = MaterialTheme.typography.labelSmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            if (chartVisualMode.supportsAverageLineToggle()) {
                AverageLineToggleRow(
                    checked = chartShowAverageLine,
                    onCheckedChange = onChartShowAverageLineChange
                )
            }
        }
    }
}

@Composable
private fun AverageLineToggleRow(
    checked: Boolean,
    onCheckedChange: (Boolean) -> Unit
) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(
            text = stringResource(R.string.report_chart_toggle_average_line),
            style = MaterialTheme.typography.labelSmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )
        Switch(
            checked = checked,
            onCheckedChange = onCheckedChange
        )
    }
}
