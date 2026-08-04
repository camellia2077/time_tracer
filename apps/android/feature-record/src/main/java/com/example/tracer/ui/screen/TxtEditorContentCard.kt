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
import com.example.tracer.ui.components.TracerSegmentedButtonDefaults
import java.time.DayOfWeek
import java.time.LocalDate
import java.time.YearMonth


@Composable
internal fun TxtEditorContentCard(
    selectedHistoryFile: String,
    selectedMonth: String,
    currentDay: LocalDate?,
    outputMode: TxtOutputMode,
    onOutputModeChange: (TxtOutputMode) -> Unit,
    activityNameTargetMode: TxtActivityNameTargetMode,
    onActivityNameTargetModeChange: (TxtActivityNameTargetMode) -> Unit,
    dayBlockEditorState: TxtDayBlockResolveResult,
    dayMarkerInput: String,
    onDayMarkerInputChange: (String) -> Unit,
    onOpenDay: (LocalDate) -> Unit,
    inlineStatusText: String,
    isEditorContentVisible: Boolean,
    onToggleEditorContentVisibility: () -> Unit,
    editorText: String,
    hasUnsavedChanges: Boolean,
    canEditDay: Boolean,
    canIngest: Boolean,
    onEditorTextChange: (String) -> Unit,
    onIngest: () -> Unit,
    dayMarkerReady: Boolean = false
) {
    val (selectedYear, selectedMonthDigits) = splitYearMonthDigits(selectedMonth)
    val (markerMonthDigits, markerDayDigits) = splitDayMarkerDigits(dayBlockEditorState.normalizedDayMarker)
    val dayContentIsoDate = dayBlockEditorState.dayContentIsoDate
    val currentDayText = currentDay?.let { formatEditorCurrentDayText(it) }
    val dayMarkerText = dayBlockEditorState.normalizedDayMarker.ifBlank { dayMarkerInput }
    val markerIsReady = dayMarkerReady || dayMarkerText.length == 4
    val monthForInput = if (markerIsReady) {
        if (markerMonthDigits.isNotBlank()) markerMonthDigits else selectedMonthDigits
    } else {
        ""
    }
    val dayForInput = if (markerIsReady) markerDayDigits else ""
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
                        colors = TracerSegmentedButtonDefaults.colors(),
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
                    },
                    dayFieldTestTag = targetDayDayFieldTestTag(),
                    dayPickerEnabled = true,
                    dayPickerDisplayMonth = currentDay?.let { YearMonth.from(it) },
                    dayPickerSelectedDate = currentDay,
                    onDayPicked = onOpenDay
                )
            }

            if (outputMode == TxtOutputMode.ALL) {
                val activityNameModes = TxtActivityNameTargetMode.entries
                SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
                    activityNameModes.forEachIndexed { index, mode ->
                        SegmentedButton(
                            shape = SegmentedButtonDefaults.itemShape(
                                index = index,
                                count = activityNameModes.size
                            ),
                            onClick = { onActivityNameTargetModeChange(mode) },
                            selected = activityNameTargetMode == mode,
                            colors = TracerSegmentedButtonDefaults.colors(),
                            modifier = Modifier.weight(1f),
                            label = {
                                Text(
                                    text = when (mode) {
                                        TxtActivityNameTargetMode.ALIAS ->
                                            stringResource(R.string.txt_mode_alias)
                                        TxtActivityNameTargetMode.CANONICAL ->
                                            stringResource(R.string.txt_mode_canonical)
                                    }
                                )
                            }
                        )
                    }
                }
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

internal fun targetDayDayFieldTestTag(): String = "txt_target_day_dd"
