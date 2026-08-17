package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.outlined.Delete
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.OutlinedCard
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.rememberModalBottomSheetState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.record.R

@Composable
internal fun TxtStructuredDayEditor(
    result: TxtDayEditResolveResult,
    roots: List<CanonicalPathNode>,
    catalogLoading: Boolean,
    catalogStatusText: String,
    collapsedRootPaths: Set<String>,
    orderedRootPaths: List<String>,
    onCollapsedRootPathsChange: (Set<String>) -> Unit,
    onOrderedRootPathsChange: (List<String>) -> Unit,
    onApply: (dayRemark: String, events: List<TxtDayEditEvent>) -> Unit
) {
    var dayRemark by remember(result.normalizedDayMarker, result.dayRemark) {
        mutableStateOf(result.dayRemark)
    }
    var events by remember(result.normalizedDayMarker, result.events) {
        mutableStateOf(result.events)
    }
    var editingDayRemark by remember { mutableStateOf(false) }
    var editingTimeIndex by remember { mutableStateOf<Int?>(null) }
    var choosingActivityIndex by remember { mutableStateOf<Int?>(null) }
    var editingActivityRemarkIndex by remember { mutableStateOf<Int?>(null) }
    var deletingActivityIndex by remember { mutableStateOf<Int?>(null) }

    Column(verticalArrangement = Arrangement.spacedBy(10.dp)) {
        OutlinedCard(modifier = Modifier.fillMaxWidth()) {
            Row(
                modifier = Modifier.padding(12.dp).fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = dayRemark.ifBlank {
                        stringResource(R.string.txt_day_edit_day_remark)
                    },
                    modifier = Modifier.weight(1f),
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
                TextButton(onClick = { editingDayRemark = true }) {
                    Text(stringResource(R.string.txt_day_edit_remark))
                }
            }
        }
        if (events.isEmpty()) {
            Text(
                text = stringResource(R.string.txt_day_edit_empty),
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
        }
        events.forEachIndexed { index, event ->
            OutlinedCard(modifier = Modifier.fillMaxWidth()) {
                Column(
                    modifier = Modifier.padding(12.dp),
                    verticalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.SpaceBetween,
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Text(
                            text = formatTxtDayEventTime(event),
                            style = MaterialTheme.typography.titleMedium
                        )
                        TextButton(onClick = { editingTimeIndex = index }) {
                            Text(stringResource(R.string.txt_day_edit_time))
                        }
                    }
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.SpaceBetween,
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Text(
                            text = event.activityToken,
                            modifier = Modifier.weight(1f),
                            style = MaterialTheme.typography.bodyLarge
                        )
                        TextButton(onClick = { choosingActivityIndex = index }) {
                            Text(stringResource(R.string.txt_day_edit_activity))
                        }
                    }
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.SpaceBetween,
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Text(
                            text = event.remark,
                            modifier = Modifier.weight(1f),
                            style = MaterialTheme.typography.bodySmall,
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                        TextButton(onClick = { editingActivityRemarkIndex = index }) {
                            Text(stringResource(R.string.txt_day_edit_remark))
                        }
                    }
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.End
                    ) {
                        IconButton(onClick = { deletingActivityIndex = index }) {
                            Icon(
                                imageVector = Icons.Outlined.Delete,
                                contentDescription = stringResource(
                                    R.string.txt_day_edit_delete_activity
                                ),
                                tint = MaterialTheme.colorScheme.error
                            )
                        }
                    }
                }
            }
        }
    }

    editingTimeIndex?.let { index ->
        val event = events.getOrNull(index) ?: return@let
        val nextTimelineEvent = events.drop(index + 1).firstOrNull {
            it.startTimelineSeconds != null
        }
        TxtDayTimeEditSheet(
            event = event,
            previousEndTimelineSeconds = event.previousEndTimelineSeconds,
            nextStartTimelineSeconds = event.nextStartTimelineSeconds,
            nextEventIsInterval = nextTimelineEvent?.isInterval == true,
            onDismiss = { editingTimeIndex = null },
            onApply = { edited ->
                val updatedEvents = events.toMutableList().also { it[index] = edited }
                events = updatedEvents
                onApply(dayRemark, updatedEvents)
                editingTimeIndex = null
            }
        )
    }
    choosingActivityIndex?.let { index ->
        RecordCanonicalCatalogScreen(
            isLoading = catalogLoading,
            roots = roots,
            statusText = catalogStatusText,
            displayMode = RecordFrequentOutputMode.CANONICAL,
            target = CanonicalBrowserTarget.TXT_DAY_EDIT,
            collapsedRootPaths = collapsedRootPaths,
            orderedRootPaths = orderedRootPaths,
            onDismissRequest = { choosingActivityIndex = null },
            onDisplayModeChange = {},
            onCollapsedRootPathsChange = onCollapsedRootPathsChange,
            onOrderedRootPathsChange = onOrderedRootPathsChange,
            onCanonicalEntryClick = { entry ->
                val updatedEvents = events.toMutableList().also { current ->
                    current[index] = current[index].copy(activityToken = entry.canonicalPath)
                }
                events = updatedEvents
                onApply(dayRemark, updatedEvents)
                choosingActivityIndex = null
            }
        )
    }
    if (editingDayRemark) {
        TxtDayRemarkEditSheet(
            title = stringResource(R.string.txt_day_edit_day_remark_title),
            initialRemark = dayRemark,
            onDismiss = { editingDayRemark = false },
            onApply = { editedRemark ->
                dayRemark = editedRemark
                onApply(dayRemark, events)
                editingDayRemark = false
            }
        )
    }
    editingActivityRemarkIndex?.let { index ->
        val event = events.getOrNull(index) ?: return@let
        TxtDayRemarkEditSheet(
            title = stringResource(R.string.txt_day_edit_activity_remark_title),
            initialRemark = event.remark,
            onDismiss = { editingActivityRemarkIndex = null },
            onApply = { editedRemark ->
                val updatedEvents = events.toMutableList().also { current ->
                    current[index] = current[index].copy(remark = editedRemark)
                }
                events = updatedEvents
                onApply(dayRemark, updatedEvents)
                editingActivityRemarkIndex = null
            }
        )
    }
    deletingActivityIndex?.let { index ->
        val event = events.getOrNull(index) ?: return@let
        AlertDialog(
            onDismissRequest = { deletingActivityIndex = null },
            title = { Text(stringResource(R.string.txt_day_edit_delete_activity_title)) },
            text = {
                Text(
                    stringResource(
                        R.string.txt_day_edit_delete_activity_message,
                        event.activityToken
                    )
                )
            },
            confirmButton = {
                TextButton(
                    onClick = {
                        val updatedEvents = events.toMutableList().also { it.removeAt(index) }
                        events = updatedEvents
                        onApply(dayRemark, updatedEvents)
                        deletingActivityIndex = null
                    }
                ) {
                    Text(
                        text = stringResource(R.string.txt_day_edit_delete_activity),
                        color = MaterialTheme.colorScheme.error
                    )
                }
            },
            dismissButton = {
                TextButton(onClick = { deletingActivityIndex = null }) {
                    Text(stringResource(R.string.txt_action_close))
                }
            }
        )
    }
}

@Composable
@OptIn(ExperimentalMaterial3Api::class)
private fun TxtDayRemarkEditSheet(
    title: String,
    initialRemark: String,
    onDismiss: () -> Unit,
    onApply: (String) -> Unit
) {
    var remark by remember(initialRemark) { mutableStateOf(initialRemark) }
    val sheetState = rememberModalBottomSheetState(skipPartiallyExpanded = true)
    ModalBottomSheet(
        onDismissRequest = onDismiss,
        sheetState = sheetState
    ) {
        Column(
            modifier = Modifier.padding(start = 24.dp, top = 8.dp, end = 24.dp, bottom = 24.dp),
            verticalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            Text(text = title, style = MaterialTheme.typography.headlineSmall)
            OutlinedTextField(
                value = remark,
                onValueChange = { remark = it },
                modifier = Modifier.fillMaxWidth(),
                label = { Text(stringResource(R.string.txt_day_edit_remark)) },
                minLines = 3
            )
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.End
            ) {
                TextButton(onClick = onDismiss) {
                    Text(stringResource(R.string.txt_action_close))
                }
                Button(onClick = { onApply(remark) }) {
                    Text(stringResource(R.string.txt_day_edit_save))
                }
            }
        }
    }
}

@Composable
@OptIn(ExperimentalMaterial3Api::class)
private fun TxtDayTimeEditSheet(
    event: TxtDayEditEvent,
    previousEndTimelineSeconds: Int?,
    nextStartTimelineSeconds: Int?,
    nextEventIsInterval: Boolean,
    onDismiss: () -> Unit,
    onApply: (TxtDayEditEvent) -> Unit
) {
    var startTimeline by remember(event.startTimelineSeconds) {
        mutableStateOf(event.startTimelineSeconds ?: parseClockSeconds(event.startTime))
    }
    var endTimeline by remember(event.endTimelineSeconds) {
        mutableStateOf(event.endTimelineSeconds ?: parseClockSeconds(event.endTime))
    }
    val lowerBoundary = previousEndTimelineSeconds
    val upperBoundary = nextStartTimelineSeconds
    val sheetState = rememberModalBottomSheetState(skipPartiallyExpanded = true)
    ModalBottomSheet(
        onDismissRequest = onDismiss,
        sheetState = sheetState
    ) {
        Column(
            modifier = Modifier.padding(start = 24.dp, top = 8.dp, end = 24.dp, bottom = 24.dp),
            verticalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            Text(
                text = stringResource(R.string.txt_day_edit_time_title),
                style = MaterialTheme.typography.headlineSmall
            )
            if (event.isInterval) {
                Text(
                    text = stringResource(R.string.record_label_interval_start),
                    style = MaterialTheme.typography.labelLarge
                )
                TimelineTimeOfDayEditor(
                    valueTimelineSeconds = startTimeline,
                    minimumTimelineSeconds = lowerBoundary ?: 0,
                    maximumTimelineSeconds = (endTimeline - 1).coerceAtLeast(
                        lowerBoundary ?: 0
                    ),
                    onValueChange = { startTimeline = it }
                )
            }
            Text(
                text = if (event.isInterval) {
                    stringResource(R.string.record_label_interval_end)
                } else {
                    stringResource(R.string.txt_day_edit_time)
                },
                style = MaterialTheme.typography.labelLarge
            )
            TimelineTimeOfDayEditor(
                valueTimelineSeconds = endTimeline,
                minimumTimelineSeconds = if (event.isInterval) {
                    startTimeline + 1
                } else {
                    (lowerBoundary ?: -1) + 1
                },
                maximumTimelineSeconds = upperBoundary?.let { boundary ->
                    if (event.isInterval || nextEventIsInterval) boundary else boundary - 1
                } ?: if (event.isInterval) {
                    startTimeline + SECONDS_PER_DAY - 1
                } else {
                    ((endTimeline / SECONDS_PER_DAY) + 1) * SECONDS_PER_DAY - 1
                },
                onValueChange = { endTimeline = it }
            )
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.End
            ) {
                TextButton(onClick = onDismiss) {
                    Text(stringResource(R.string.txt_action_close))
                }
                Button(
                    onClick = {
                        onApply(
                            event.copy(
                                startTime = formatClockSeconds(startTimeline),
                                endTime = formatClockSeconds(endTimeline),
                                startTimelineSeconds = startTimeline,
                                endTimelineSeconds = endTimeline
                            )
                        )
                    }
                ) {
                    Text(stringResource(R.string.txt_day_edit_save))
                }
            }
        }
    }
}

@Composable
private fun TimelineTimeOfDayEditor(
    valueTimelineSeconds: Int,
    minimumTimelineSeconds: Int,
    maximumTimelineSeconds: Int,
    onValueChange: (Int) -> Unit
) {
    val safeMinimum = minimumTimelineSeconds.coerceAtLeast(0)
    val safeMaximum = maximumTimelineSeconds.coerceAtLeast(safeMinimum)
    val selected = valueTimelineSeconds.coerceIn(safeMinimum, safeMaximum)
    val hour = selected / SECONDS_PER_HOUR
    val minute = (selected % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE
    val second = selected % SECONDS_PER_MINUTE
    val minimumHour = safeMinimum / SECONDS_PER_HOUR
    val maximumHour = safeMaximum / SECONDS_PER_HOUR
    fun minimumMinuteForHour(candidateHour: Int): Int =
        if (candidateHour == minimumHour) {
            (safeMinimum % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE
        } else {
            0
        }
    fun maximumMinuteForHour(candidateHour: Int): Int =
        if (candidateHour == maximumHour) {
            (safeMaximum % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE
        } else {
            59
        }
    val minimumMinute = minimumMinuteForHour(hour)
    val maximumMinute = maximumMinuteForHour(hour)
    fun minimumSecond(candidateMinute: Int): Int =
        if (hour == minimumHour && candidateMinute == minimumMinute) {
            safeMinimum % SECONDS_PER_MINUTE
        } else {
            0
        }
    fun maximumSecond(candidateMinute: Int): Int =
        if (hour == maximumHour && candidateMinute == maximumMinute) {
            safeMaximum % SECONDS_PER_MINUTE
        } else {
            59
        }
    val minimumSecond = minimumSecond(minute)
    val maximumSecond = maximumSecond(minute)
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        WheelNumberPicker(
            label = stringResource(R.string.record_time_hour),
            value = hour,
            values = minimumHour..maximumHour,
            valueText = { "%02d".format(it % 24) },
            modifier = Modifier.weight(1f),
            onValueChange = { nextHour ->
                val nextMinute = minute.coerceIn(
                    minimumMinuteForHour(nextHour), maximumMinuteForHour(nextHour)
                )
                val nextSecond = second.coerceIn(
                    if (nextHour == minimumHour &&
                        nextMinute == minimumMinuteForHour(nextHour)
                    ) {
                        safeMinimum % SECONDS_PER_MINUTE
                    } else 0,
                    if (nextHour == maximumHour &&
                        nextMinute == maximumMinuteForHour(nextHour)
                    ) {
                        safeMaximum % SECONDS_PER_MINUTE
                    } else 59
                )
                onValueChange(nextHour * SECONDS_PER_HOUR +
                    nextMinute * SECONDS_PER_MINUTE + nextSecond)
            }
        )
        WheelNumberPicker(
            label = stringResource(R.string.record_time_minute),
            value = minute,
            values = minimumMinute..maximumMinute,
            modifier = Modifier.weight(1f),
            onValueChange = { nextMinute ->
                val nextSecond = second.coerceIn(
                    minimumSecond(nextMinute), maximumSecond(nextMinute)
                )
                onValueChange(hour * SECONDS_PER_HOUR +
                    nextMinute * SECONDS_PER_MINUTE + nextSecond)
            }
        )
        WheelNumberPicker(
            label = stringResource(R.string.record_time_second),
            value = second,
            values = minimumSecond..maximumSecond,
            modifier = Modifier.weight(1f),
            onValueChange = { nextSecond ->
                onValueChange(hour * SECONDS_PER_HOUR +
                    minute * SECONDS_PER_MINUTE + nextSecond)
            }
        )
    }
}

private fun parseClockSeconds(value: String): Int {
    val digits = value.filter(Char::isDigit).padStart(6, '0').take(6)
    return digits.substring(0, 2).toInt() * SECONDS_PER_HOUR +
        digits.substring(2, 4).toInt() * SECONDS_PER_MINUTE +
        digits.substring(4, 6).toInt()
}

internal fun formatClockSeconds(value: Int): String {
    val clockValue = ((value % SECONDS_PER_DAY) + SECONDS_PER_DAY) % SECONDS_PER_DAY
    val hour = clockValue / SECONDS_PER_HOUR
    val minute = (clockValue % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE
    val second = clockValue % SECONDS_PER_MINUTE
    // Structured day-edit events use ISO clock strings on the Android/Core
    // boundary. Core validates them, then serializes Raw TXT as HHMMSS.
    return "%02d:%02d:%02d".format(hour, minute, second)
}

private const val SECONDS_PER_MINUTE = 60
private const val SECONDS_PER_HOUR = 60 * SECONDS_PER_MINUTE
private const val SECONDS_PER_DAY = 24 * SECONDS_PER_HOUR

internal fun formatTxtDayEventTime(event: TxtDayEditEvent): String {
    fun format(value: String): String = formatIsoClockTime(value)
    return if (event.isInterval) {
        "${format(event.startTime)} – ${format(event.endTime)}"
    } else {
        format(event.endTime)
    }
}
