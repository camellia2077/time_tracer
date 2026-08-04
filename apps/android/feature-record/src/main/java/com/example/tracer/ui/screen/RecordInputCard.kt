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
internal fun recordTxtPreviewButtonTestTag(): String = "record_txt_preview_button"

internal fun recordActivityNameInputTestTag(): String = "record_activity_name_input"

internal fun recordRemarkInputTestTag(): String = "record_remark_input"
