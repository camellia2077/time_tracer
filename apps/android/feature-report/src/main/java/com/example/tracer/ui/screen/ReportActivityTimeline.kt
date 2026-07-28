package com.example.tracer

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.Dp
import com.example.tracer.feature.report.R
import kotlinx.coroutines.launch


@Composable
internal fun ReportActivityTimeline(
    report: StructuredDailyReport,
    modifier: Modifier = Modifier,
    onUpdateActivityRemark: suspend (ActivityTimelineItem, String) -> RecordActionResult,
    onUpdateDayRemark: suspend (String) -> RecordActionResult
) {
    val colors = reportSemanticColors()
    var editingActivity by remember { mutableStateOf<ActivityTimelineItem?>(null) }
    var draftRemark by remember { mutableStateOf("") }
    var editError by remember { mutableStateOf("") }
    var saving by remember { mutableStateOf(false) }
    var editingDayRemark by remember { mutableStateOf(false) }
    var dayRemarkDraft by remember { mutableStateOf("") }
    val scope = rememberCoroutineScope()


    Column(modifier = modifier.fillMaxWidth()) {
        Text(
            text = stringResource(R.string.report_result_title_activity_timeline),
            style = MaterialTheme.typography.titleMedium,
            color = colors.root
        )
        Spacer(modifier = Modifier.height(8.dp))
        report.dayRemark.let { dayRemark ->
            Surface(
                modifier = Modifier.fillMaxWidth(),
                tonalElevation = 1.dp,
                shape = MaterialTheme.shapes.medium
            ) {
                Column(modifier = Modifier.padding(12.dp)) {
                    Text(
                        text = stringResource(R.string.report_day_remark_label),
                        style = MaterialTheme.typography.labelMedium,
                        color = colors.child
                    )
                    Spacer(modifier = Modifier.height(4.dp))
                    Text(
                        text = dayRemark,
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onSurface
                    )
                    TextButton(onClick = {
                        dayRemarkDraft = dayRemark
                        editError = ""
                        editingDayRemark = true
                    }) {
                        Text(stringResource(R.string.report_edit_day_remark))
                    }
                }
            }
            Spacer(modifier = Modifier.height(8.dp))
        }
        if (report.activities.isEmpty()) {
            Text(
                text = stringResource(R.string.report_activity_timeline_empty),
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            return
        }

        Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
            report.activities.forEach { activity ->
                val intervalHeight = timelineIntervalHeight(activity.durationSeconds)
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .heightIn(min = intervalHeight),
                    verticalAlignment = Alignment.Top
                ) {
                    Column(
                        modifier = Modifier
                            .width(64.dp)
                            .height(intervalHeight),
                        horizontalAlignment = Alignment.End,
                        verticalArrangement = Arrangement.SpaceBetween
                    ) {
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
                    Column(
                        modifier = Modifier
                            .width(28.dp)
                            .height(intervalHeight),
                        horizontalAlignment = Alignment.CenterHorizontally
                    ) {
                        // The filled node is the activity start; it must stay at the
                        // top of the duration interval rather than in its center.
                        Box(
                            modifier = Modifier
                                .size(12.dp)
                                .background(colors.node, CircleShape)
                        )
                        // This flexible segment represents the activity duration.
                        // Its parent row height is derived from durationSeconds.
                        Box(
                            modifier = Modifier
                                .weight(1f)
                                .width(2.dp)
                                .background(colors.track)
                        )
                        // The hollow node marks the activity end at the bottom of
                        // the same interval.
                        Box(
                            modifier = Modifier
                                .size(12.dp)
                                .border(
                                    width = 2.dp,
                                    color = colors.node,
                                    shape = CircleShape
                                )
                                .background(MaterialTheme.colorScheme.surface, CircleShape)
                        )
                    }
                    Surface(
                        modifier = Modifier
                            .weight(1f)
                            .heightIn(min = intervalHeight),
                        tonalElevation = 1.dp,
                        shape = MaterialTheme.shapes.medium
                    ) {
                        Column(modifier = Modifier.padding(12.dp)) {
                            val nameParts = splitTimelineActivityName(activity.activityName)
                            // The first underscore is the product-level project/category
                            // boundary: keep that primary category on its own prominent line.
                            Text(
                                text = nameParts.primary,
                                style = MaterialTheme.typography.titleMedium,
                                color = MaterialTheme.colorScheme.onSurface
                            )
                            nameParts.secondary?.let { secondaryName ->
                                Text(
                                    // Remaining underscores represent nested activity folders;
                                    // render them as a readable hierarchy and let Compose wrap
                                    // the path when the timeline card is narrow.
                                    text = secondaryName.replace("_", " > "),
                                    style = MaterialTheme.typography.bodySmall,
                                    color = MaterialTheme.colorScheme.onSurfaceVariant
                                )
                            }
                            Text(
                                text = formatTimelineDuration(activity.durationSeconds),
                                style = MaterialTheme.typography.labelMedium,
                                color = colors.progress
                            )
                            TextButton(
                                onClick = {
                                    editingActivity = activity
                                    draftRemark = activity.remark.orEmpty()
                                    editError = ""
                                },
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
            }
        }
    }

    editingActivity?.let { activity ->
        AlertDialog(
            onDismissRequest = { if (!saving) editingActivity = null },
            title = { Text(stringResource(R.string.report_edit_activity_remark)) },
            text = {
                Column {
                    Text(
                        text = activity.activityName,
                        style = MaterialTheme.typography.labelMedium
                    )
                    Spacer(modifier = Modifier.height(8.dp))
                    OutlinedTextField(
                        value = draftRemark,
                        onValueChange = { draftRemark = it },
                        enabled = !saving,
                        label = { Text(stringResource(R.string.report_activity_remark_label)) },
                        modifier = Modifier.fillMaxWidth()
                    )
                    if (editError.isNotBlank()) {
                        Spacer(modifier = Modifier.height(8.dp))
                        Text(
                            text = editError,
                            color = MaterialTheme.colorScheme.error,
                            style = MaterialTheme.typography.bodySmall
                        )
                    }
                }
            },
            confirmButton = {
                TextButton(
                    enabled = !saving,
                    onClick = {
                        saving = true
                        editError = ""
                        scope.launch {
                            val result = onUpdateActivityRemark(activity, draftRemark)
                            saving = false
                            if (result.ok) {
                                editingActivity = null
                            } else {
                                editError = result.message.ifBlank {
                                    "Activity remark update failed."
                                }
                            }
                        }
                    }
                ) {
                    Text(stringResource(R.string.report_save_activity_remark))
                }
            },
            dismissButton = {
                TextButton(
                    enabled = !saving,
                    onClick = { editingActivity = null }
                ) {
                    Text(stringResource(R.string.report_cancel_activity_remark))
                }
            }
        )
    }

    if (editingDayRemark) {
        AlertDialog(
            onDismissRequest = { if (!saving) editingDayRemark = false },
            title = { Text(stringResource(R.string.report_edit_day_remark)) },
            text = {
                Column {
                    OutlinedTextField(
                        value = dayRemarkDraft,
                        onValueChange = { dayRemarkDraft = it },
                        enabled = !saving,
                        label = { Text(stringResource(R.string.report_day_remark_label)) },
                        modifier = Modifier.fillMaxWidth()
                    )
                    if (editError.isNotBlank()) {
                        Spacer(modifier = Modifier.height(8.dp))
                        Text(editError, color = MaterialTheme.colorScheme.error)
                    }
                }
            },
            confirmButton = {
                TextButton(enabled = !saving, onClick = {
                    saving = true
                    editError = ""
                    scope.launch {
                        val result = onUpdateDayRemark(dayRemarkDraft)
                        saving = false
                        if (result.ok) editingDayRemark = false
                        else editError = result.message.ifBlank { "Day remark update failed." }
                    }
                }) { Text(stringResource(R.string.report_save_day_remark)) }
            },
            dismissButton = {
                TextButton(enabled = !saving, onClick = { editingDayRemark = false }) {
                    Text(stringResource(R.string.report_cancel_day_remark))
                }
            }
        )
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
    // Keep enough room for the category, sub-path, duration, and multiline remark;
    // longer activities still grow beyond this minimum according to their duration.
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
