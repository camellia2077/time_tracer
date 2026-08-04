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
internal fun IntervalTimeEditSheet(
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

