package com.example.tracer

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import androidx.compose.foundation.layout.fillMaxWidth
import com.example.tracer.feature.report.R

internal sealed interface ReportTimelineEntry {
    val activity: ActivityTimelineItem

    data class Interval(override val activity: ActivityTimelineItem) : ReportTimelineEntry

    data class EndOnly(override val activity: ActivityTimelineItem) : ReportTimelineEntry
}

internal fun ActivityTimelineItem.toReportTimelineEntry(): ReportTimelineEntry =
    when (kind) {
        ActivityTimelineRecordKind.INTERVAL -> ReportTimelineEntry.Interval(this)
        ActivityTimelineRecordKind.END_ONLY -> ReportTimelineEntry.EndOnly(this)
    }

@Composable
internal fun ReportTimelineEntryRow(
    entry: ReportTimelineEntry,
    colors: ReportSemanticColors,
    onEditRemark: (ActivityTimelineItem) -> Unit
) {
    val activity = entry.activity
    val height = when (entry) {
        is ReportTimelineEntry.Interval -> timelineIntervalHeight(activity.durationSeconds)
        is ReportTimelineEntry.EndOnly -> 72.dp
    }

    Row(
        modifier = Modifier
            .fillMaxWidth()
            .heightIn(min = height),
        verticalAlignment = Alignment.Top
    ) {
        TimelineEntryTimes(entry, height)
        TimelineEntryRail(entry, height, colors)
        ActivityTimelineCard(
            activity = activity,
            showDuration = entry is ReportTimelineEntry.Interval,
            colors = colors,
            modifier = Modifier.weight(1f),
            onEditRemark = onEditRemark
        )
    }
}

@Composable
private fun TimelineEntryTimes(entry: ReportTimelineEntry, height: Dp) {
    val activity = entry.activity
    Column(
        modifier = Modifier
            .width(64.dp)
            .height(height),
        horizontalAlignment = Alignment.End,
        verticalArrangement = Arrangement.SpaceBetween
    ) {
        when (entry) {
            is ReportTimelineEntry.Interval -> {
                Text(
                    text = activity.startTime,
                    style = MaterialTheme.typography.labelMedium,
                    color = MaterialTheme.colorScheme.onSurface
                )
                Text(
                    text = activity.endTime,
                    style = MaterialTheme.typography.labelSmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
            is ReportTimelineEntry.EndOnly -> {
                // End-only activities have no reliable start time. Keep their sole
                // timestamp aligned with the bottom/end marker instead of
                // implying a start event at the top of the timeline row.
                Spacer(modifier = Modifier.weight(1f))
                Text(
                    text = stringResource(
                        R.string.report_activity_timeline_end_only_time,
                        activity.endTime
                    ),
                    style = MaterialTheme.typography.labelMedium,
                    color = MaterialTheme.colorScheme.onSurface
                )
            }
        }
    }
}

@Composable
private fun TimelineEntryRail(
    entry: ReportTimelineEntry,
    height: Dp,
    colors: ReportSemanticColors
) {
    Column(
        modifier = Modifier
            .width(28.dp)
            .height(height),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        when (entry) {
            is ReportTimelineEntry.Interval -> {
                Box(
                    modifier = Modifier
                        .size(12.dp)
                        .background(colors.node, CircleShape)
                )
                Box(
                    modifier = Modifier
                        .weight(1f)
                        .width(2.dp)
                        .background(colors.track)
                )
                Box(
                    modifier = Modifier
                        .size(12.dp)
                        .border(2.dp, colors.node, CircleShape)
                        .background(MaterialTheme.colorScheme.surface, CircleShape)
                )
            }
            is ReportTimelineEntry.EndOnly -> {
                // The hollow bottom marker represents the known end time;
                // drawing a solid top marker would falsely suggest a start time.
                Spacer(modifier = Modifier.weight(1f))
                Box(
                    modifier = Modifier
                        .size(12.dp)
                        .border(2.dp, colors.node, CircleShape)
                        .background(MaterialTheme.colorScheme.surface, CircleShape)
                )
            }
        }
    }
}

@Composable
private fun ActivityTimelineCard(
    activity: ActivityTimelineItem,
    showDuration: Boolean,
    colors: ReportSemanticColors,
    modifier: Modifier,
    onEditRemark: (ActivityTimelineItem) -> Unit
) {
    Surface(
        modifier = modifier
            .heightIn(min = if (showDuration) timelineIntervalHeight(activity.durationSeconds) else 72.dp),
        tonalElevation = 1.dp,
        shape = MaterialTheme.shapes.medium
    ) {
        Column(modifier = Modifier.padding(12.dp)) {
            val nameParts = splitTimelineActivityName(activity.activityName)
            Text(
                text = nameParts.primary,
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.onSurface
            )
            nameParts.secondary?.let { secondaryName ->
                Text(
                    text = secondaryName.replace("_", " > "),
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
            if (showDuration) {
                Text(
                    text = formatTimelineDuration(activity.durationSeconds),
                    style = MaterialTheme.typography.labelMedium,
                    color = colors.progress
                )
            }
            TextButton(
                onClick = { onEditRemark(activity) },
                modifier = Modifier.padding(top = 2.dp)
            ) {
                Text(stringResource(R.string.report_edit_activity_remark))
            }
            activity.remark?.takeIf { it.isNotBlank() }?.let { remark ->
                Spacer(modifier = Modifier.height(6.dp))
                Text(
                    text = remark,
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
        }
    }
}

private data class TimelineActivityNameParts(
    val primary: String,
    val secondary: String?
)

private fun splitTimelineActivityName(activityName: String): TimelineActivityNameParts {
    val separatorIndex = activityName.indexOf('_')
    if (separatorIndex <= 0 || separatorIndex == activityName.lastIndex) {
        return TimelineActivityNameParts(primary = activityName, secondary = null)
    }
    return TimelineActivityNameParts(
        primary = activityName.substring(0, separatorIndex),
        secondary = activityName.substring(separatorIndex + 1)
    )
}

private fun timelineIntervalHeight(durationSeconds: Long): Dp {
    val durationMinutes = durationSeconds.coerceAtLeast(1L) / 60f
    val heightInDp = (durationMinutes * 2f).coerceIn(128f, 240f)
    return heightInDp.dp
}

private fun formatTimelineDuration(durationSeconds: Long): String {
    val totalSeconds = durationSeconds.coerceAtLeast(0L)
    val hours = totalSeconds / 3600
    val minutes = (totalSeconds % 3600) / 60
    val seconds = totalSeconds % 60
    return when {
        hours > 0 -> "${hours}h ${minutes}m"
        minutes > 0 -> "${minutes}m ${seconds}s"
        else -> "${seconds}s"
    }
}
