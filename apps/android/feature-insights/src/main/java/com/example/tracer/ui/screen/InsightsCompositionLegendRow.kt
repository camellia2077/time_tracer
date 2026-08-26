package com.example.tracer

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ChevronRight
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.insights.R
import java.util.Locale

@Composable
internal fun CompositionLegendRow(
    slice: InsightsCompositionSlice,
    showAverage: Boolean,
    showFrequency: Boolean,
    label: String = slice.root,
    color: Color,
    hasChildren: Boolean,
    onClick: () -> Unit,
    modifier: Modifier = Modifier
) {
    val averageDurationPerOccurrenceSeconds =
        slice.averageDurationPerOccurrenceSeconds
    Column(
        modifier = modifier
            .fillMaxWidth()
            .clickable(onClick = onClick)
            .padding(horizontal = 4.dp, vertical = 10.dp),
        verticalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Box(
                modifier = Modifier
                    .size(10.dp)
                    .background(color = color, shape = CircleShape)
            )
            Text(
                text = label,
                modifier = Modifier
                    .weight(1f)
                    .padding(start = 8.dp, end = 12.dp),
                style = MaterialTheme.typography.bodyMedium,
                fontWeight = FontWeight.Medium,
                maxLines = 1,
                overflow = TextOverflow.Ellipsis
            )
            Text(
                text = String.format(Locale.ROOT, "%.1f%%", slice.percent),
                style = MaterialTheme.typography.bodyMedium,
                fontWeight = FontWeight.SemiBold,
                color = MaterialTheme.colorScheme.primary
            )
            if (hasChildren) {
                Icon(
                    imageVector = Icons.Filled.ChevronRight,
                    contentDescription = null,
                    modifier = Modifier.padding(start = 4.dp),
                    tint = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
        }
        Column(
            modifier = Modifier.padding(start = 18.dp),
            verticalArrangement = Arrangement.spacedBy(4.dp)
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
            if (averageDurationPerOccurrenceSeconds != null &&
                (slice.occurrenceCount ?: 0L) > 0L
            ) {
                Text(
                    text = stringResource(
                        R.string.insights_chart_composition_average_per_occurrence,
                        formatDurationHoursMinutes(
                            averageDurationPerOccurrenceSeconds
                        )
                    ),
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
        }
    }
}

@Composable
internal fun CompositionBarLegendRow(
    slice: InsightsCompositionSlice,
    showAverage: Boolean,
    showFrequency: Boolean,
    modifier: Modifier = Modifier
) {
    val averageDurationPerOccurrenceSeconds = slice.averageDurationPerOccurrenceSeconds
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
                text = slice.root,
                style = MaterialTheme.typography.bodySmall,
                maxLines = 1,
                overflow = TextOverflow.Ellipsis
            )
            Text(
                text = String.format(Locale.ROOT, "%.1f%%", slice.percent),
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
            if (averageDurationPerOccurrenceSeconds != null &&
                (slice.occurrenceCount ?: 0L) > 0L
            ) {
                Text(
                    text = stringResource(
                        R.string.insights_chart_composition_average_per_occurrence,
                        formatDurationHoursMinutes(averageDurationPerOccurrenceSeconds)
                    ),
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
        }
    }
}
