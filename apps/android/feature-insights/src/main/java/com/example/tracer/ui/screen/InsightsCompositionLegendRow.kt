package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.insights.R

@Composable
internal fun CompositionLegendRow(
    slice: InsightsCompositionSlice,
    showAverage: Boolean,
    showFrequency: Boolean,
    label: String = slice.root,
    modifier: Modifier = Modifier
) {
    Row(
        modifier = modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.Top
    ) {
        Column(
            modifier = Modifier.weight(1f).padding(end = 12.dp),
            verticalArrangement = Arrangement.spacedBy(2.dp)
        ) {
            Text(
                text = label,
                style = MaterialTheme.typography.bodySmall,
                maxLines = 1,
                overflow = TextOverflow.Ellipsis
            )
            Text(
                text = String.format("%.1f%%", slice.percent),
                style = MaterialTheme.typography.labelSmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
        }
        Column(
            horizontalAlignment = Alignment.End,
            verticalArrangement = Arrangement.spacedBy(2.dp)
        ) {
            Text(
                text = stringResource(
                    if (showFrequency) R.string.insights_chart_composition_total_records
                    else R.string.insights_chart_composition_total_duration,
                    if (showFrequency) slice.occurrenceCount ?: 0L
                    else formatDurationHoursMinutes(slice.totalDurationSeconds ?: 0L)
                ),
                style = MaterialTheme.typography.bodySmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            if (showAverage) {
                Text(
                    text = stringResource(
                        if (showFrequency) R.string.insights_chart_composition_average_records
                        else R.string.insights_chart_composition_average_duration,
                        if (showFrequency) slice.averageOccurrenceCount ?: 0.0
                        else formatDurationHoursMinutes(slice.averageDurationSeconds ?: 0L)
                    ),
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
        }
    }
}
