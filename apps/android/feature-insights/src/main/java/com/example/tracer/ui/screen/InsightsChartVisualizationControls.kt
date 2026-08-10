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
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import com.example.tracer.feature.insights.R
import com.example.tracer.ui.components.TracerSegmentedButtonDefaults

@Composable
internal fun InsightsChartVisualModeSelector(
    insightsMode: InsightsMode,
    chartVisualMode: InsightsChartVisualMode,
    onChartVisualModeChange: (InsightsChartVisualMode) -> Unit
) {
    Text(
        text = stringResource(R.string.insights_label_chart_visual),
        style = MaterialTheme.typography.labelSmall,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
    SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
        val visualModes = availableInsightsChartVisualModes(insightsMode)
        visualModes.forEachIndexed { index, visualMode ->
            val selected = chartVisualMode == visualMode
            SegmentedButton(
                shape = SegmentedButtonDefaults.itemShape(
                    index = index,
                    count = visualModes.size
                ),
                onClick = { onChartVisualModeChange(visualMode) },
                selected = selected,
                colors = TracerSegmentedButtonDefaults.colors(),
                icon = {},
                label = {
                    Text(
                        text = stringResource(visualMode.labelRes()),
                        maxLines = 1,
                        softWrap = false,
                        overflow = TextOverflow.Ellipsis,
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
internal fun InsightsChartVisualizationHintSection(
    chartVisualMode: InsightsChartVisualMode,
    chartShowAverageLine: Boolean,
    onChartShowAverageLineChange: (Boolean) -> Unit,
    heatmapTomlConfig: InsightsHeatmapTomlConfig,
    heatmapStylePreference: InsightsHeatmapStylePreference,
    onHeatmapThemePolicyChange: (InsightsHeatmapThemePolicy) -> Unit,
    onHeatmapPaletteNameChange: (String) -> Unit,
    heatmapApplyMessage: String
) {
    when (chartVisualMode) {
        InsightsChartVisualMode.HEATMAP_MONTH,
        InsightsChartVisualMode.HEATMAP_MULTI_MONTH -> {
            InsightsChartHeatmapSettings(
                heatmapTomlConfig = heatmapTomlConfig,
                heatmapStylePreference = heatmapStylePreference,
                onHeatmapPaletteNameChange = onHeatmapPaletteNameChange,
                heatmapApplyMessage = heatmapApplyMessage
            )
        }

        else -> {
            Text(
                text = "${stringResource(R.string.insights_chart_axis_x_date)} · " +
                    stringResource(R.string.insights_chart_axis_y_hours),
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
            text = stringResource(R.string.insights_chart_toggle_average_line),
            style = MaterialTheme.typography.labelSmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )
        Switch(
            checked = checked,
            onCheckedChange = onCheckedChange
        )
    }
}
