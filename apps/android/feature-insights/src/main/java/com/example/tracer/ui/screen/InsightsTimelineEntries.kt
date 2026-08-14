package com.example.tracer

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
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
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.semantics.semantics
import com.example.tracer.feature.insights.R
import androidx.compose.ui.semantics.testTag

private const val ParentColorIndicatorTestTag = "insights-parent-color-indicator"

internal sealed interface InsightsTimelineEntry {
    val activity: ActivityTimelineItem

    data class Interval(override val activity: ActivityTimelineItem) : InsightsTimelineEntry

    data class EndOnly(override val activity: ActivityTimelineItem) : InsightsTimelineEntry
}

/** A day uses duration-scaled rails; period browsing uses a fixed reading rhythm. */
internal enum class InsightsTimelineLayout {
    DURATION_SCALED,
    FIXED
}

internal fun ActivityTimelineItem.toInsightsTimelineEntry(): InsightsTimelineEntry =
    when (kind) {
        ActivityTimelineRecordKind.INTERVAL -> InsightsTimelineEntry.Interval(this)
        ActivityTimelineRecordKind.END_ONLY -> InsightsTimelineEntry.EndOnly(this)
    }

@Composable
internal fun InsightsTimelineEntryRow(
    entry: InsightsTimelineEntry,
    onEditRemark: ((ActivityTimelineItem) -> Unit)? = null,
    layout: InsightsTimelineLayout = InsightsTimelineLayout.DURATION_SCALED
) {
    val activity = entry.activity
    val height = timelineEntryHeight(entry, layout)
    // Read the same theme token source in every context (Day and period
    // browsing) so a caller cannot accidentally inject a divergent palette.
    val colors = insightsSemanticColors()

    Row(
        modifier = Modifier
            .fillMaxWidth()
            .heightIn(min = height),
        verticalAlignment = Alignment.Top
    ) {
        TimelineEntryParentColorIndicator(
            color = activity.parentColor?.toTimelineParentColor(),
            height = height
        )
        TimelineEntryTimes(entry, height)
        TimelineEntryRail(entry, height, colors)
        ActivityTimelineCard(
            activity = activity,
            showDuration = entry is InsightsTimelineEntry.Interval,
            colors = colors,
            modifier = Modifier.weight(1f),
            onEditRemark = onEditRemark,
            height = height
        )
    }
}

@Composable
internal fun InsightsTimelineRecordList(
    activities: List<ActivityTimelineItem>,
    layout: InsightsTimelineLayout,
    onEditRemark: ((ActivityTimelineItem) -> Unit)? = null,
    modifier: Modifier = Modifier
) {
    Column(
        modifier = modifier,
        verticalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        activities.forEach { activity ->
            InsightsTimelineEntryRow(
                entry = activity.toInsightsTimelineEntry(),
                onEditRemark = onEditRemark,
                layout = layout
            )
        }
    }
}

/** Shared period grouping plane: theme surface outside, timeline cards inside. */
@Composable
internal fun InsightsTimelineExpandableGroup(
    expanded: Boolean,
    onToggle: () -> Unit,
    title: String,
    trailing: String,
    summary: String,
    content: @Composable () -> Unit
) {
    Surface(
        modifier = Modifier.fillMaxWidth(),
        shape = MaterialTheme.shapes.medium,
        color = MaterialTheme.colorScheme.surface
    ) {
        Column {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .clickable(onClick = onToggle)
                    .padding(horizontal = 12.dp, vertical = 10.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = if (expanded) "▼" else "▶",
                    color = MaterialTheme.colorScheme.primary,
                    style = MaterialTheme.typography.labelMedium
                )
                Spacer(modifier = Modifier.width(8.dp))
                Column(modifier = Modifier.weight(1f)) {
                    Text(text = title, style = MaterialTheme.typography.titleSmall)
                    Text(
                        text = summary,
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
                Text(
                    text = trailing,
                    color = MaterialTheme.colorScheme.primary,
                    style = MaterialTheme.typography.labelMedium
                )
            }
            if (expanded) {
                Column(
                    modifier = Modifier.padding(start = 12.dp, end = 12.dp, bottom = 12.dp),
                    verticalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    content()
                }
            }
        }
    }
}

private fun timelineEntryHeight(entry: InsightsTimelineEntry, layout: InsightsTimelineLayout): Dp =
    when (layout) {
        InsightsTimelineLayout.FIXED -> 112.dp
        InsightsTimelineLayout.DURATION_SCALED -> when (entry) {
            is InsightsTimelineEntry.Interval -> timelineIntervalHeight(entry.activity.durationSeconds)
            is InsightsTimelineEntry.EndOnly -> 72.dp
        }
    }

@Composable
private fun TimelineEntryTimes(entry: InsightsTimelineEntry, height: Dp) {
    val activity = entry.activity
    Column(
        modifier = Modifier
            .width(64.dp)
            .height(height),
        horizontalAlignment = Alignment.End,
        verticalArrangement = Arrangement.SpaceBetween
    ) {
        when (entry) {
            is InsightsTimelineEntry.Interval -> {
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
            is InsightsTimelineEntry.EndOnly -> {
                // End-only activities have no reliable start time. Keep their sole
                // timestamp aligned with the bottom/end marker instead of
                // implying a start event at the top of the timeline row.
                Spacer(modifier = Modifier.weight(1f))
                Text(
                    text = stringResource(
                        R.string.insights_activity_timeline_end_only_time,
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
    entry: InsightsTimelineEntry,
    height: Dp,
    colors: InsightsSemanticColors
) {
    Column(
        modifier = Modifier
            .width(28.dp)
            .height(height),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        when (entry) {
            is InsightsTimelineEntry.Interval -> {
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
            is InsightsTimelineEntry.EndOnly -> {
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
private fun TimelineEntryParentColorIndicator(color: Color?, height: Dp) {
    // Keep the parent cue in the timeline's far-left breathing room. The
    // timestamps then sit immediately beside their rail, preserving the
    // time-axis reading order without changing the card width.
    Box(
        modifier = Modifier
            .width(12.dp)
            .height(height),
        contentAlignment = Alignment.Center
    ) {
        color?.let {
            Box(
                modifier = Modifier
                    .width(8.dp)
                    .heightIn(min = height)
                    .fillMaxHeight()
                    .background(it)
                    .semantics { testTag = ParentColorIndicatorTestTag }
            )
        }
    }
}

@Composable
private fun ActivityTimelineCard(
    activity: ActivityTimelineItem,
    showDuration: Boolean,
    colors: InsightsSemanticColors,
    modifier: Modifier,
    onEditRemark: ((ActivityTimelineItem) -> Unit)?,
    height: Dp
) {
    Surface(
        modifier = modifier.heightIn(min = height),
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
            onEditRemark?.let { editRemark ->
                TextButton(
                    onClick = { editRemark(activity) },
                    modifier = Modifier.padding(top = 2.dp)
                ) {
                    Text(stringResource(R.string.insights_edit_activity_remark))
                }
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

private fun String.toTimelineParentColor(): Color? {
    if (length != 7 || firstOrNull() != '#') return null
    val rgb = substring(1).toLongOrNull(16) ?: return null
    return Color(0xFF000000L or rgb)
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
    return formatInsightsDuration(durationSeconds, InsightsDurationFormat.COMPACT)
}
