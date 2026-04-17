package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.navigationBarsPadding
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.widthIn
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material.icons.automirrored.filled.ArrowForward
import androidx.compose.material3.Button
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.Text
import androidx.compose.material3.rememberModalBottomSheetState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.record.R
import com.example.tracer.ui.components.NativeMultilineTextEditor
import com.example.tracer.ui.components.NativeMultilineTextEditorController
import com.example.tracer.ui.components.SegmentedMonthDayInput
import com.example.tracer.ui.components.filterDigits
import com.example.tracer.ui.components.splitYearMonthDigits
import java.time.DayOfWeek
import java.time.LocalDate

@Composable
internal fun TxtEditorContentCard(
    selectedHistoryFile: String,
    selectedMonth: String,
    currentDay: LocalDate?,
    outputMode: TxtOutputMode,
    onOutputModeChange: (TxtOutputMode) -> Unit,
    dayBlockEditorState: TxtDayBlockResolveResult,
    dayMarkerInput: String,
    onDayMarkerInputChange: (String) -> Unit,
    inlineStatusText: String,
    isEditorContentVisible: Boolean,
    onToggleEditorContentVisibility: () -> Unit,
    editorText: String,
    hasUnsavedChanges: Boolean,
    canEditDay: Boolean,
    canIngest: Boolean,
    onEditorTextChange: (String) -> Unit,
    onIngest: () -> Unit
) {
    val (selectedYear, selectedMonthDigits) = splitYearMonthDigits(selectedMonth)
    val (markerMonthDigits, markerDayDigits) = splitDayMarkerDigits(dayBlockEditorState.normalizedDayMarker)
    val monthForInput = if (markerMonthDigits.isNotBlank()) markerMonthDigits else selectedMonthDigits
    val dayForInput = markerDayDigits
    val dayContentIsoDate = dayBlockEditorState.dayContentIsoDate
    val currentDayText = currentDay?.let { formatEditorCurrentDayText(it) }
    val dayMarkerText = dayBlockEditorState.normalizedDayMarker.ifBlank { dayMarkerInput }

    ElevatedCard(modifier = Modifier.fillMaxWidth()) {
        Column(
            modifier = Modifier.padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Text(
                text = stringResource(R.string.txt_label_editor_file, selectedHistoryFile),
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary
            )

            val outputModes = TxtOutputMode.entries
            SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
                outputModes.forEachIndexed { index, mode ->
                    SegmentedButton(
                        shape = SegmentedButtonDefaults.itemShape(index = index, count = outputModes.size),
                        onClick = { onOutputModeChange(mode) },
                        selected = outputMode == mode,
                        modifier = Modifier.weight(1f),
                        label = {
                            Text(
                                text = when (mode) {
                                    TxtOutputMode.ALL -> stringResource(R.string.txt_mode_all)
                                    TxtOutputMode.DAY -> stringResource(R.string.txt_mode_day)
                                }
                            )
                        }
                    )
                }
            }
            if (outputMode == TxtOutputMode.DAY) {
                val numericKeyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number)
                SegmentedMonthDayInput(
                    title = if (selectedYear.isNotBlank()) {
                        stringResource(R.string.txt_label_target_day_with_year, selectedYear)
                    } else {
                        stringResource(R.string.txt_label_target_day)
                    },
                    month = monthForInput,
                    day = dayForInput,
                    keyboardOptions = numericKeyboardOptions,
                    onMonthChange = { nextMonth ->
                        onDayMarkerInputChange(
                            filterDigits(nextMonth, 2) + filterDigits(dayForInput, 2)
                        )
                    },
                    onDayChange = { nextDay ->
                        onDayMarkerInputChange(
                            filterDigits(monthForInput, 2) + filterDigits(nextDay, 2)
                        )
                    }
                )
            }

            if (inlineStatusText.isNotBlank()) {
                val isError = inlineStatusText.contains("fail", ignoreCase = true) ||
                    inlineStatusText.contains("error", ignoreCase = true) ||
                    inlineStatusText.contains("invalid", ignoreCase = true) ||
                    inlineStatusText.contains("blocked", ignoreCase = true) ||
                    inlineStatusText.contains("duplicate", ignoreCase = true) ||
                    inlineStatusText.contains("mismatch", ignoreCase = true) ||
                    inlineStatusText.contains("missing", ignoreCase = true)
                val statusColor = if (isError) {
                    MaterialTheme.colorScheme.error
                } else {
                    MaterialTheme.colorScheme.primary
                }
                Text(
                    text = inlineStatusText,
                    style = MaterialTheme.typography.bodySmall,
                    color = statusColor,
                    modifier = Modifier.fillMaxWidth()
                )
            }

            if (currentDayText != null) {
                Text(
                    text = currentDayText,
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onSurface,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis,
                    modifier = Modifier.fillMaxWidth()
                )
            }

            Button(
                onClick = onToggleEditorContentVisibility,
                modifier = Modifier.fillMaxWidth()
            ) {
                Text(
                    if (isEditorContentVisible) {
                        stringResource(R.string.txt_action_hide_content)
                    } else {
                        stringResource(R.string.txt_action_show_content)
                    }
                )
            }
        }
    }

    if (isEditorContentVisible) {
        TxtEditorBottomSheet(
            value = editorText,
            outputMode = outputMode,
            currentDayText = currentDayText,
            dayMarkerText = dayMarkerText,
            dayContentIsoDate = dayContentIsoDate,
            hasUnsavedChanges = hasUnsavedChanges,
            canEditDay = canEditDay,
            canIngest = canIngest,
            onEditorTextChange = onEditorTextChange,
            onIngest = onIngest,
            onDismissRequest = onToggleEditorContentVisibility
        )
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun TxtEditorBottomSheet(
    value: String,
    outputMode: TxtOutputMode,
    currentDayText: String?,
    dayMarkerText: String,
    dayContentIsoDate: String?,
    hasUnsavedChanges: Boolean,
    canEditDay: Boolean,
    canIngest: Boolean,
    onEditorTextChange: (String) -> Unit,
    onIngest: () -> Unit,
    onDismissRequest: () -> Unit
) {
    val sheetState = rememberModalBottomSheetState(skipPartiallyExpanded = true)
    // Undo/redo history is scoped to the currently open editor sheet. Closing the sheet or
    // ingesting successfully starts a new editing session the next time users open TXT content.
    val editorController = remember { NativeMultilineTextEditorController() }

    ModalBottomSheet(
        onDismissRequest = onDismissRequest,
        sheetState = sheetState,
        // Treat the TXT editor as a stable editing surface instead of a draggable bottom sheet.
        // The raw editor now embeds a native EditText, and letting the sheet drag at the same
        // time causes gesture contention: upward swipes can bounce between moving the sheet and
        // scrolling the editor, which shows up as repeated vertical jumping in ALL mode.
        // Disabling sheet gestures here leaves text scrolling fully owned by the editor and keeps
        // the layout steady, closer to a phone's built-in note editor.
        sheetGesturesEnabled = false
    ) {
        // Design intent:
        // Keep the action area fixed at the top like a phone note editor so users can always
        // reach ingest without scrolling long content to the bottom. The text area owns the
        // scrolling, while the header stays stable when the IME opens or the document grows.
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .fillMaxHeight(if (outputMode == TxtOutputMode.DAY) 0.96f else 0.92f)
                .navigationBarsPadding()
                .padding(horizontal = 16.dp, vertical = 8.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            TxtEditorSheetHeader(
                title = if (outputMode == TxtOutputMode.DAY) {
                    if (dayContentIsoDate != null) {
                        stringResource(R.string.txt_label_day_content_with_date, dayContentIsoDate)
                    } else {
                        stringResource(R.string.txt_label_day_content)
                    }
                } else {
                    stringResource(R.string.txt_label_content)
                },
                subtitle = if (outputMode == TxtOutputMode.DAY) currentDayText else null,
                meta = if (outputMode == TxtOutputMode.DAY) {
                    stringResource(R.string.record_txt_preview_day_marker, dayMarkerText)
                } else {
                    null
                },
                canUndo = editorController.canUndo,
                canRedo = editorController.canRedo,
                onUndo = editorController::requestUndo,
                onRedo = editorController::requestRedo,
                hasUnsavedChanges = hasUnsavedChanges,
                canIngest = canIngest,
                onClose = onDismissRequest,
                onIngest = {
                    onIngest()
                }
            )

            NativeMultilineTextEditor(
                value = value,
                onValueChange = onEditorTextChange,
                modifier = Modifier
                    .fillMaxWidth()
                    .weight(1f),
                minLines = if (outputMode == TxtOutputMode.DAY) 18 else 12,
                monospace = true,
                controller = editorController,
                readOnly = outputMode == TxtOutputMode.DAY && !canEditDay
            )
        }
    }
}

@Composable
private fun TxtEditorSheetHeader(
    title: String,
    subtitle: String?,
    meta: String?,
    canUndo: Boolean,
    canRedo: Boolean,
    onUndo: () -> Unit,
    onRedo: () -> Unit,
    hasUnsavedChanges: Boolean,
    canIngest: Boolean,
    onClose: () -> Unit,
    onIngest: () -> Unit
) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(12.dp),
        verticalAlignment = Alignment.Top
    ) {
        Column(
            modifier = Modifier.weight(1f),
            verticalArrangement = Arrangement.spacedBy(4.dp)
        ) {
            Text(
                text = title,
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary
            )
            if (subtitle != null) {
                Text(
                    text = subtitle,
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onSurface
                )
            }
            if (meta != null) {
                Text(
                    text = meta,
                    style = MaterialTheme.typography.labelLarge,
                    color = MaterialTheme.colorScheme.primary
                )
            }
        }

        Column(
            horizontalAlignment = Alignment.End,
            verticalArrangement = Arrangement.spacedBy(6.dp)
        ) {
            Row(
                horizontalArrangement = Arrangement.spacedBy(4.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                IconButton(
                    onClick = onUndo,
                    enabled = canUndo
                ) {
                    Icon(
                        imageVector = Icons.AutoMirrored.Filled.ArrowBack,
                        contentDescription = stringResource(R.string.txt_cd_undo)
                    )
                }
                IconButton(
                    onClick = onRedo,
                    enabled = canRedo
                ) {
                    Icon(
                        imageVector = Icons.AutoMirrored.Filled.ArrowForward,
                        contentDescription = stringResource(R.string.txt_cd_redo)
                    )
                }
            }
            Text(
                text = if (hasUnsavedChanges) {
                    stringResource(R.string.txt_status_unsaved)
                } else {
                    stringResource(R.string.txt_status_saved)
                },
                style = MaterialTheme.typography.labelSmall,
                color = if (hasUnsavedChanges) {
                    MaterialTheme.colorScheme.primary
                } else {
                    MaterialTheme.colorScheme.onSurfaceVariant
                }
            )
            Row(
                horizontalArrangement = Arrangement.spacedBy(8.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                OutlinedButton(
                    onClick = onClose,
                    modifier = Modifier.widthIn(min = 88.dp)
                ) {
                    Text(stringResource(R.string.txt_action_close))
                }
                Button(
                    onClick = onIngest,
                    enabled = canIngest,
                    modifier = Modifier.widthIn(min = 88.dp)
                ) {
                    Text(stringResource(R.string.txt_cd_ingest))
                }
            }
        }
    }
}

private fun splitDayMarkerDigits(value: String): Pair<String, String> {
    val digits = filterDigits(value, 4)
    return Pair(digits.take(2), digits.drop(2).take(2))
}

@Composable
private fun formatEditorCurrentDayText(date: LocalDate): String {
    val weekdayLabel = stringResource(
        when (date.dayOfWeek) {
            DayOfWeek.MONDAY -> R.string.txt_weekday_mon
            DayOfWeek.TUESDAY -> R.string.txt_weekday_tue
            DayOfWeek.WEDNESDAY -> R.string.txt_weekday_wed
            DayOfWeek.THURSDAY -> R.string.txt_weekday_thu
            DayOfWeek.FRIDAY -> R.string.txt_weekday_fri
            DayOfWeek.SATURDAY -> R.string.txt_weekday_sat
            DayOfWeek.SUNDAY -> R.string.txt_weekday_sun
        }
    )
    return "$date $weekdayLabel"
}
