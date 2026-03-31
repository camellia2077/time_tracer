package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material3.Button
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.record.R
import com.example.tracer.ui.components.SegmentedMonthDayInput
import com.example.tracer.ui.components.filterDigits
import com.example.tracer.ui.components.splitYearMonthDigits

@Composable
internal fun TxtEditorContentCard(
    selectedHistoryFile: String,
    selectedMonth: String,
    outputMode: TxtOutputMode,
    onOutputModeChange: (TxtOutputMode) -> Unit,
    dayMarkerInput: String,
    onDayMarkerInputChange: (String) -> Unit,
    inlineStatusText: String,
    isEditorContentVisible: Boolean,
    onToggleEditorContentVisibility: () -> Unit,
    editableHistoryContent: String,
    onEditableHistoryContentChange: (String) -> Unit
) {
    val normalizedDayMarker = dayMarkerInput.filter { it.isDigit() }.take(4)
    val (selectedYear, selectedMonthDigits) = splitYearMonthDigits(selectedMonth)
    val (markerMonthDigits, markerDayDigits) = splitDayMarkerDigits(normalizedDayMarker)
    val monthForInput = if (markerMonthDigits.isNotBlank()) markerMonthDigits else selectedMonthDigits
    val dayForInput = markerDayDigits
    val dayContentIsoDate = buildDayContentIsoDate(
        selectedMonth = selectedMonth,
        dayMarker = normalizedDayMarker
    )
    val dayEditorState = buildDayBlockEditorState(
        content = editableHistoryContent,
        dayMarker = normalizedDayMarker
    )
    val editorText = if (outputMode == TxtOutputMode.ALL) {
        editableHistoryContent
    } else {
        dayEditorState.text
    }
    val dayModeHint = if (outputMode == TxtOutputMode.DAY) {
        when {
            !dayEditorState.isMarkerValid ->
                stringResource(R.string.txt_hint_invalid_day_marker)

            dayEditorState.found ->
                stringResource(R.string.txt_hint_day_edit_enabled)

            else ->
                stringResource(R.string.txt_hint_day_not_found, normalizedDayMarker)
        }
    } else {
        ""
    }

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
                        stringResource(R.string.txt_label_target_day) + " ($selectedYear)"
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

            if (dayModeHint.isNotBlank()) {
                Text(
                    text = dayModeHint,
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.tertiary
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

            if (isEditorContentVisible) {
                OutlinedTextField(
                    value = editorText,
                    onValueChange = { value ->
                        if (outputMode == TxtOutputMode.ALL) {
                            onEditableHistoryContentChange(value)
                        } else if (dayEditorState.isMarkerValid && dayEditorState.found) {
                            onEditableHistoryContentChange(
                                mergeDayBlockContent(
                                    fullContent = editableHistoryContent,
                                    dayMarker = normalizedDayMarker,
                                    editedDayBody = value
                                )
                            )
                        }
                    },
                    readOnly = false,
                    label = {
                        val label = if (outputMode == TxtOutputMode.ALL) {
                            stringResource(R.string.txt_label_content)
                        } else if (dayContentIsoDate != null) {
                            stringResource(R.string.txt_label_day_content_with_date, dayContentIsoDate)
                        } else {
                            stringResource(R.string.txt_label_day_content)
                        }
                        Text(label)
                    },
                    modifier = Modifier.fillMaxWidth(),
                    minLines = 8,
                    textStyle = MaterialTheme.typography.bodyMedium.copy(fontFamily = androidx.compose.ui.text.font.FontFamily.Monospace)
                )
            }
        }
    }
}

private fun buildDayContentIsoDate(selectedMonth: String, dayMarker: String): String? {
    if (!isValidDayMarkerForIsoTitle(dayMarker)) {
        return null
    }
    val yearMonth = Regex("""^(\d{4})-(\d{2})$""").matchEntire(selectedMonth.trim())
        ?: return null
    val year = yearMonth.groupValues[1]
    val month = dayMarker.substring(0, 2)
    val day = dayMarker.substring(2, 4)
    return "$year-$month-$day"
}

private fun isValidDayMarkerForIsoTitle(value: String): Boolean {
    if (value.length != 4 || !value.all { it.isDigit() }) {
        return false
    }
    val month = value.substring(0, 2).toIntOrNull() ?: return false
    val day = value.substring(2, 4).toIntOrNull() ?: return false
    return month in 1..12 && day in 1..31
}

private fun splitDayMarkerDigits(value: String): Pair<String, String> {
    val digits = filterDigits(value, 4)
    return Pair(digits.take(2), digits.drop(2).take(2))
}
