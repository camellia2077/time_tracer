package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.wrapContentHeight
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.gestures.snapping.rememberSnapFlingBehavior
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.AccountTree
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.Edit
import androidx.compose.material.icons.filled.Description
import androidx.compose.material.icons.filled.PlayArrow
import androidx.compose.material.icons.filled.Stop
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.OutlinedCard
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.focus.onFocusChanged
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.unit.dp
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.runtime.getValue
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.runtime.snapshotFlow
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.hapticfeedback.HapticFeedbackType
import androidx.compose.ui.platform.LocalHapticFeedback
import kotlinx.coroutines.flow.drop
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale
import java.util.concurrent.TimeUnit
import com.example.tracer.feature.record.R
import com.example.tracer.ui.components.TracerSegmentedButtonDefaults

@OptIn(ExperimentalMaterial3Api::class)
@Composable
internal fun RecordInputCard(
    authoringMode: RecordAuthoringMode,
    onAuthoringModeChange: (RecordAuthoringMode) -> Unit,
    recordContent: String,
    onRecordContentChange: (String) -> Unit,
    recordRemark: String,
    onRecordRemarkChange: (String) -> Unit,
    intervalStart: String,
    onIntervalStartChange: (String) -> Unit,
    intervalEnd: String,
    onIntervalEndChange: (String) -> Unit,
    intervalStartedAtEpochMs: Long,
    attributionDateIso: String = "",
    currentTimeMillis: Long,
    lastRecordedActivityAlias: String,
    lastRecordedDuration: String,
    onOpenCanonicalCatalog: () -> Unit = {},
    onOpenTxtPreview: () -> Unit,
    onStartIntervalRecording: () -> Unit,
    onStopIntervalRecording: () -> Unit,
    onDiscardIntervalDraft: () -> Unit,
    onRecordNow: () -> Unit
) {
    ElevatedCard(
        modifier = Modifier.fillMaxWidth(),
        shape = MaterialTheme.shapes.extraLarge
    ) {
        var isIntervalTimeEditorVisible by remember { mutableStateOf(false) }
        var isDiscardConfirmationVisible by remember { mutableStateOf(false) }

        Column(
            modifier = Modifier.padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                Text(
                    text = stringResource(R.string.record_title_record_input),
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.primary
                )
                Row(
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    IconButton(onClick = onOpenCanonicalCatalog) {
                        Icon(
                            imageVector = Icons.Default.AccountTree,
                            contentDescription = stringResource(
                                R.string.record_cd_open_canonical_catalog
                            )
                        )
                    }
                    IconButton(
                        onClick = onOpenTxtPreview,
                        modifier = Modifier.testTag(recordTxtPreviewButtonTestTag())
                    ) {
                        Icon(
                            imageVector = Icons.Default.Description,
                            contentDescription = stringResource(R.string.record_cd_open_txt_preview)
                        )
                    }
                }
            }

            if (lastRecordedActivityAlias.isNotBlank() && lastRecordedDuration.isNotBlank()) {
                Text(
                    text = stringResource(
                        R.string.record_last_recorded_summary,
                        lastRecordedActivityAlias,
                        lastRecordedDuration
                    ),
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }

            SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
                listOf(
                    RecordAuthoringMode.INTERVAL to stringResource(R.string.record_mode_interval),
                    RecordAuthoringMode.POINT to stringResource(R.string.record_mode_point)
                ).forEachIndexed { index, option ->
                    val (mode, label) = option
                    val selected = authoringMode == mode
                    SegmentedButton(
                        shape = SegmentedButtonDefaults.itemShape(index = index, count = 2),
                        onClick = { onAuthoringModeChange(mode) },
                        selected = selected,
                        modifier = Modifier.weight(1f),
                        colors = TracerSegmentedButtonDefaults.colors(),
                        label = {
                            Text(
                                text = label,
                                maxLines = 1,
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

            OutlinedTextField(
                value = recordContent,
                onValueChange = onRecordContentChange,
                label = { Text(stringResource(R.string.record_label_activity_name)) },
                leadingIcon = { Icon(Icons.Default.Edit, contentDescription = null) },
                singleLine = true,
                shape = MaterialTheme.shapes.large,
                modifier = Modifier
                    .fillMaxWidth()
                    .testTag(recordActivityNameInputTestTag())
            )

            val isIntervalRunning = intervalStart.isNotBlank() && intervalEnd.isBlank()
            val hasIntervalDraft = intervalStart.isNotBlank() && intervalEnd.isNotBlank()
            if (authoringMode == RecordAuthoringMode.INTERVAL) {
                if (isIntervalRunning) {
                    Text(
                        text = stringResource(R.string.record_interval_active_title),
                        style = MaterialTheme.typography.titleSmall,
                        color = MaterialTheme.colorScheme.primary
                    )
                    val elapsedSeconds = if (intervalStartedAtEpochMs > 0L) {
                        TimeUnit.MILLISECONDS.toSeconds(
                            (currentTimeMillis - intervalStartedAtEpochMs).coerceAtLeast(0L)
                        )
                    } else {
                        0L
                    }
                    Text(
                        text = stringResource(
                            R.string.record_interval_started_at,
                            formatCompactClockTime(intervalStart)
                        ),
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                    if (attributionDateIso.isNotBlank()) {
                        Text(
                            text = stringResource(
                                R.string.record_interval_attribution_date,
                                attributionDateIso
                            ),
                            style = MaterialTheme.typography.bodyMedium,
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    }
                    Text(
                        text = stringResource(
                            R.string.record_interval_elapsed,
                            formatDurationSummary(elapsedSeconds)
                        ),
                        style = MaterialTheme.typography.headlineMedium,
                        color = MaterialTheme.colorScheme.primary
                    )
                } else if (hasIntervalDraft) {
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Column(modifier = Modifier.weight(1f)) {
                            Text(
                                text = stringResource(R.string.record_interval_draft_title),
                                style = MaterialTheme.typography.titleSmall,
                                color = MaterialTheme.colorScheme.primary
                            )
                            Text(
                                text = stringResource(
                                    R.string.record_interval_summary,
                                    formatCompactClockTime(intervalStart),
                                    formatCompactClockTime(intervalEnd),
                                    formatDurationSummary(intervalDurationSeconds(intervalStart, intervalEnd))
                                ),
                                style = MaterialTheme.typography.bodyLarge,
                                color = MaterialTheme.colorScheme.onSurfaceVariant
                            )
                            if (attributionDateIso.isNotBlank()) {
                                Text(
                                    text = stringResource(
                                        R.string.record_interval_attribution_date,
                                        attributionDateIso
                                    ),
                                    style = MaterialTheme.typography.bodyMedium,
                                    color = MaterialTheme.colorScheme.onSurfaceVariant
                                )
                            }
                        }
                        IconButton(
                            onClick = { isIntervalTimeEditorVisible = true }
                        ) {
                            Icon(
                                imageVector = Icons.Default.Edit,
                                contentDescription = stringResource(
                                    R.string.record_cd_edit_interval_time
                                )
                            )
                        }
                    }
                }
            }

            if (isIntervalTimeEditorVisible && intervalStart.isNotBlank() && intervalEnd.isNotBlank()) {
                IntervalTimeEditSheet(
                    start = intervalStart,
                    end = intervalEnd,
                    attributionDateIso = attributionDateIso,
                    currentTimeMillis = currentTimeMillis,
                    onDismiss = { isIntervalTimeEditorVisible = false },
                    onApply = { editedStart, editedEnd ->
                        onIntervalStartChange(editedStart)
                        onIntervalEndChange(editedEnd)
                        isIntervalTimeEditorVisible = false
                    }
                )
            }

            val remarkHasLineBreak = recordRemark.contains('\n')
            OutlinedTextField(
                value = recordRemark,
                onValueChange = onRecordRemarkChange,
                label = { Text(stringResource(R.string.record_label_remark_optional)) },
                leadingIcon = { Icon(Icons.Default.Edit, contentDescription = null) },
                singleLine = false,
                minLines = if (remarkHasLineBreak) 2 else 1,
                maxLines = 4,
                keyboardOptions = KeyboardOptions.Default.copy(
                    imeAction = ImeAction.Default
                ),
                shape = MaterialTheme.shapes.large,
                modifier = Modifier
                    .fillMaxWidth()
                    .testTag(recordRemarkInputTestTag())
            )

            if (authoringMode == RecordAuthoringMode.INTERVAL) {
                when {
                    isIntervalRunning -> Button(
                        onClick = onStopIntervalRecording,
                        modifier = Modifier
                            .fillMaxWidth()
                            .height(56.dp),
                        colors = ButtonDefaults.buttonColors(
                            containerColor = MaterialTheme.colorScheme.error,
                            contentColor = MaterialTheme.colorScheme.onError
                        )
                    ) {
                        Icon(Icons.Default.Stop, contentDescription = null)
                        Spacer(modifier = Modifier.width(8.dp))
                        Text(stringResource(R.string.record_action_stop_interval))
                    }

                    hasIntervalDraft -> {
                        Row(
                            modifier = Modifier.fillMaxWidth(),
                            horizontalArrangement = Arrangement.spacedBy(8.dp),
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            TextButton(
                                onClick = { isDiscardConfirmationVisible = true },
                                modifier = Modifier.height(56.dp)
                            ) {
                                Text(
                                    text = stringResource(R.string.record_action_discard_interval),
                                    color = MaterialTheme.colorScheme.error
                                )
                            }
                            Button(
                                onClick = onRecordNow,
                                modifier = Modifier
                                    .weight(1f)
                                    .height(56.dp)
                            ) {
                                Icon(Icons.Default.Check, contentDescription = null)
                                Spacer(modifier = Modifier.width(8.dp))
                                Text(stringResource(R.string.record_action_save_interval))
                            }
                        }
                    }

                    else -> Button(
                        onClick = onStartIntervalRecording,
                        modifier = Modifier
                            .fillMaxWidth()
                            .height(56.dp)
                    ) {
                        Icon(Icons.Default.PlayArrow, contentDescription = null)
                        Spacer(modifier = Modifier.width(8.dp))
                        Text(stringResource(R.string.record_action_start_interval))
                    }
                }
            } else {
                Button(
                    onClick = onRecordNow,
                    modifier = Modifier
                        .fillMaxWidth()
                        .height(56.dp)
                ) {
                    Icon(Icons.Default.Check, contentDescription = null)
                    Spacer(modifier = Modifier.width(8.dp))
                    Text(stringResource(R.string.record_action_record_activity))
                }
            }

            if (isDiscardConfirmationVisible) {
                AlertDialog(
                    onDismissRequest = { isDiscardConfirmationVisible = false },
                    title = {
                        Text(stringResource(R.string.record_interval_discard_title))
                    },
                    text = {
                        Text(stringResource(R.string.record_interval_discard_description))
                    },
                    dismissButton = {
                        TextButton(onClick = { isDiscardConfirmationVisible = false }) {
                            Text(stringResource(R.string.record_action_cancel))
                        }
                    },
                    confirmButton = {
                        TextButton(
                            onClick = {
                                isDiscardConfirmationVisible = false
                                onDiscardIntervalDraft()
                            }
                        ) {
                            Text(
                                text = stringResource(R.string.record_action_discard_interval),
                                color = MaterialTheme.colorScheme.error
                            )
                        }
                    }
                )
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun IntervalTimeEditSheet(
    start: String,
    end: String,
    attributionDateIso: String,
    currentTimeMillis: Long,
    onDismiss: () -> Unit,
    onApply: (String, String) -> Unit
) {
    var editedStart by remember(start) { mutableStateOf(start) }
    var editedEnd by remember(end) { mutableStateOf(end) }
    var editingEndpoint by remember { mutableStateOf<IntervalEndpoint?>(null) }

    ModalBottomSheet(onDismissRequest = onDismiss) {
        Column(
            modifier = Modifier.padding(start = 24.dp, end = 24.dp, bottom = 24.dp),
            verticalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            Text(
                text = stringResource(R.string.record_interval_edit_title),
                style = MaterialTheme.typography.headlineSmall
            )
            Text(
                text = stringResource(
                    R.string.record_interval_current_time,
                    formatCurrentClockTime(currentTimeMillis)
                ),
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.primary
            )
            if (attributionDateIso.isNotBlank()) {
                Text(
                    text = stringResource(
                        R.string.record_interval_attribution_date,
                        attributionDateIso
                    ),
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
            IntervalTimeSummaryRow(
                label = stringResource(R.string.record_label_interval_start),
                value = editedStart,
                selected = editingEndpoint == IntervalEndpoint.START,
                onClick = { editingEndpoint = IntervalEndpoint.START }
            )
            if (editingEndpoint == IntervalEndpoint.START) {
                IntervalTimeEditor(
                    label = stringResource(R.string.record_label_interval_start),
                    value = editedStart,
                    isStart = true,
                    comparisonTime = editedEnd,
                    onValueChange = { candidate ->
                        if (
                            isCrossMidnightInterval(editedStart, editedEnd) ||
                            intervalTimeSeconds(candidate) <= intervalTimeSeconds(editedEnd)
                        ) {
                            editedStart = candidate
                        }
                    }
                )
            }
            IntervalTimeSummaryRow(
                label = stringResource(R.string.record_label_interval_end),
                value = editedEnd,
                selected = editingEndpoint == IntervalEndpoint.END,
                onClick = { editingEndpoint = IntervalEndpoint.END }
            )
            if (editingEndpoint == IntervalEndpoint.END) {
                IntervalTimeEditor(
                    label = stringResource(R.string.record_label_interval_end),
                    isStart = false,
                    comparisonTime = editedStart,
                    value = editedEnd,
                    onValueChange = { editedEnd = it }
                )
            }
            Text(
                text = stringResource(
                    R.string.record_interval_duration,
                    formatDurationSummary(intervalDurationSeconds(editedStart, editedEnd))
                ),
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary
            )
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.End,
                verticalAlignment = Alignment.CenterVertically
            ) {
                TextButton(onClick = onDismiss) {
                    Text(stringResource(R.string.record_action_cancel))
                }
                Button(onClick = { onApply(editedStart, editedEnd) }) {
                    Text(stringResource(R.string.record_action_apply))
                }
            }
        }
    }
}

private enum class IntervalEndpoint { START, END }

@Composable
private fun IntervalTimeSummaryRow(
    label: String,
    value: String,
    selected: Boolean,
    onClick: () -> Unit
) {
    OutlinedCard(
        onClick = onClick,
        modifier = Modifier.fillMaxWidth()
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 16.dp, vertical = 12.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            Text(
                text = label,
                style = MaterialTheme.typography.labelLarge,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            Text(
                text = formatCompactClockTime(value).take(5),
                style = MaterialTheme.typography.headlineSmall,
                color = if (selected) {
                    MaterialTheme.colorScheme.primary
                } else {
                    MaterialTheme.colorScheme.onSurface
                }
            )
        }
    }
}

@Composable
private fun IntervalTimeEditor(
    label: String,
    value: String,
    isStart: Boolean,
    comparisonTime: String,
    onValueChange: (String) -> Unit
) {
    Column(verticalArrangement = Arrangement.spacedBy(6.dp)) {
        Text(
            text = label,
            style = MaterialTheme.typography.labelLarge,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            val parts = splitIntervalTime(value)
            val comparisonParts = splitIntervalTime(comparisonTime)
            val isCrossMidnight = isStart && isCrossMidnightInterval(value, comparisonTime)
            val maximumHour = if (isStart && !isCrossMidnight) {
                comparisonParts[0].toIntOrNull()?.coerceIn(0, 23) ?: 23
            } else {
                23
            }
            val selectedHour = parts[0].toIntOrNull()?.coerceIn(0, maximumHour) ?: 0
            val maximumMinute = if (
                isStart && !isCrossMidnight && selectedHour == maximumHour
            ) {
                comparisonParts[1].toIntOrNull()?.coerceIn(0, 59) ?: 59
            } else {
                59
            }
            val selectedMinute = parts[1].toIntOrNull()?.coerceIn(0, maximumMinute) ?: 0
            val maximumSecond = if (
                isStart &&
                !isCrossMidnight &&
                selectedHour == maximumHour &&
                selectedMinute == maximumMinute
            ) {
                comparisonParts[2].toIntOrNull()?.coerceIn(0, 59) ?: 59
            } else {
                59
            }
            WheelNumberPicker(
                label = stringResource(R.string.record_time_hour),
                value = selectedHour,
                values = 0..maximumHour,
                modifier = Modifier.weight(1f),
                onValueChange = { hour ->
                    val minute = parts[1].toIntOrNull()?.coerceIn(0, 59) ?: 0
                    val adjustedMinute = if (hour == maximumHour) {
                        minute.coerceAtMost(maximumMinute)
                    } else {
                        minute
                    }
                    val second = parts[2].toIntOrNull()?.coerceIn(0, 59) ?: 0
                    val adjustedSecond = if (
                        hour == maximumHour && adjustedMinute == maximumMinute
                    ) {
                        second.coerceAtMost(maximumSecond)
                    } else {
                        second
                    }
                    onValueChange("%02d%02d%02d".format(hour, adjustedMinute, adjustedSecond))
                }
            )
            WheelNumberPicker(
                label = stringResource(R.string.record_time_minute),
                value = parts[1].toIntOrNull()?.coerceIn(0, maximumMinute) ?: 0,
                values = 0..maximumMinute,
                modifier = Modifier.weight(1f),
                onValueChange = { minute ->
                    val second = parts[2].toIntOrNull()?.coerceIn(0, 59) ?: 0
                    val adjustedSecond = if (
                        selectedHour == maximumHour && minute == maximumMinute
                    ) {
                        second.coerceAtMost(maximumSecond)
                    } else {
                        second
                    }
                    onValueChange("%02d%02d%02d".format(selectedHour, minute, adjustedSecond))
                }
            )
            WheelNumberPicker(
                label = stringResource(R.string.record_time_second),
                value = parts[2].toIntOrNull()?.coerceIn(0, maximumSecond) ?: 0,
                values = 0..maximumSecond,
                modifier = Modifier.weight(1f),
                onValueChange = { second ->
                    onValueChange("%02d%02d%02d".format(selectedHour, selectedMinute, second))
                }
            )
        }
    }
}

@Composable
private fun WheelNumberPicker(
    label: String,
    value: Int,
    values: IntRange,
    modifier: Modifier = Modifier,
    onValueChange: (Int) -> Unit
) {
    val itemHeight = 48.dp
    val listState = rememberLazyListState(initialFirstVisibleItemIndex = value)
    val flingBehavior = rememberSnapFlingBehavior(lazyListState = listState)
    val hapticFeedback = LocalHapticFeedback.current

    LaunchedEffect(value) {
        val target = value.coerceIn(values.first, values.last)
        if (listState.firstVisibleItemIndex != target) {
            listState.animateScrollToItem(target)
        }
    }
    LaunchedEffect(listState) {
        snapshotFlow { listState.firstVisibleItemIndex }
            .drop(1)
            .collect { index ->
                if (index in 0 until values.count()) {
                    hapticFeedback.performHapticFeedback(HapticFeedbackType.LongPress)
                    onValueChange(values.elementAt(index))
                }
            }
    }

    Column(
        modifier = modifier,
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Text(
            text = label,
            style = MaterialTheme.typography.labelMedium,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )
        LazyColumn(
            state = listState,
            flingBehavior = flingBehavior,
            modifier = Modifier
                .fillMaxWidth()
                .height(itemHeight * 3),
            contentPadding = PaddingValues(vertical = itemHeight),
            horizontalAlignment = androidx.compose.ui.Alignment.CenterHorizontally
        ) {
            items(values.toList()) { number ->
                Text(
                    text = "%02d".format(number),
                    style = MaterialTheme.typography.titleLarge,
                    modifier = Modifier
                        .alpha(
                            if (number == listState.firstVisibleItemIndex + values.first) {
                                1f
                            } else {
                                0.38f
                            }
                        )
                        .height(itemHeight)
                        .wrapContentHeight()
                )
            }
        }
    }
}

private fun splitIntervalTime(value: String): List<String> {
    val digits = value.filter(Char::isDigit).padStart(6, '0').takeLast(6)
    return listOf(digits.substring(0, 2), digits.substring(2, 4), digits.substring(4, 6))
}

private fun formatCompactClockTime(value: String): String {
    val digits = value.filter(Char::isDigit).padStart(6, '0').takeLast(6)
    return "${digits.substring(0, 2)}:${digits.substring(2, 4)}:${digits.substring(4, 6)}"
}

private fun formatCurrentClockTime(currentTimeMillis: Long): String =
    SimpleDateFormat("HH:mm:ss", Locale.getDefault()).format(Date(currentTimeMillis))

private fun intervalDurationSeconds(start: String, end: String): Long {
    fun toSeconds(value: String): Long {
        val digits = value.filter(Char::isDigit).padStart(6, '0').takeLast(6)
        return digits.substring(0, 2).toLong() * 3600L +
            digits.substring(2, 4).toLong() * 60L +
            digits.substring(4, 6).toLong()
    }

    val duration = toSeconds(end) - toSeconds(start)
    return if (duration < 0L) duration + TimeUnit.DAYS.toSeconds(1) else duration
}

private fun intervalTimeSeconds(value: String): Long {
    val digits = value.filter(Char::isDigit).padStart(6, '0').takeLast(6)
    return digits.substring(0, 2).toLong() * 3600L +
        digits.substring(2, 4).toLong() * 60L +
        digits.substring(4, 6).toLong()
}

private fun isCrossMidnightInterval(start: String, end: String): Boolean =
    intervalTimeSeconds(end) < intervalTimeSeconds(start)

private fun formatDurationSummary(totalSeconds: Long): String {
    val normalizedSeconds = totalSeconds.coerceAtLeast(0L)
    val hours = normalizedSeconds / 3600L
    val minutes = (normalizedSeconds % 3600L) / 60L
    val seconds = normalizedSeconds % 60L
    return buildList {
        if (hours > 0L) add("${hours}h")
        if (minutes > 0L) add("${minutes}m")
        if (seconds > 0L || isEmpty()) add("${seconds}s")
    }.joinToString(" ")
}

internal fun recordTxtPreviewButtonTestTag(): String = "record_txt_preview_button"

internal fun recordActivityNameInputTestTag(): String = "record_activity_name_input"

internal fun recordRemarkInputTestTag(): String = "record_remark_input"
