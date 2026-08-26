package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.wrapContentHeight
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.gestures.snapping.rememberSnapFlingBehavior
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.PlaylistAdd
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.Edit
import androidx.compose.material.icons.filled.Description
import androidx.compose.material.icons.filled.Fullscreen
import androidx.compose.material.icons.filled.FullscreenExit
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
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.rememberModalBottomSheetState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.focus.onFocusChanged
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.input.TextFieldValue
import androidx.compose.ui.text.TextRange
import androidx.compose.ui.unit.dp
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.runtime.getValue
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.runtime.snapshotFlow
import androidx.compose.animation.core.Animatable
import androidx.compose.animation.core.LinearEasing
import androidx.compose.animation.core.tween
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.hapticfeedback.HapticFeedbackType
import androidx.compose.ui.platform.LocalHapticFeedback
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.LifecycleEventObserver
import androidx.lifecycle.compose.LocalLifecycleOwner
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
    lastRecordedActivityHierarchyLeaf: String,
    lastRecordedDuration: String,
    latestActivityRecord: LatestActivityRecord? = null,
    previousActivityTail: PreviousActivityTail? = null,
    onOpenCanonicalCatalog: () -> Unit = {},
    onOpenTxtPreview: () -> Unit,
    onStartIntervalRecording: () -> Unit,
    onStopIntervalRecording: () -> Unit,
    onDiscardIntervalDraft: () -> Unit,
    onUsePreviousActivityEndTime: () -> Unit = {},
    onRecordNow: () -> Unit
) {
    var activityNameInputValue by remember {
        mutableStateOf(TextFieldValue(recordContent))
    }
    LaunchedEffect(recordContent) {
        activityNameInputValue = syncActivityNameInputValue(
            currentValue = activityNameInputValue,
            recordContent = recordContent
        )
    }

    ElevatedCard(
        modifier = Modifier.fillMaxWidth(),
        shape = MaterialTheme.shapes.extraLarge
    ) {
        var isIntervalTimeEditorVisible by remember { mutableStateOf(false) }
        var isDiscardConfirmationVisible by remember { mutableStateOf(false) }
        var isRemarkEditorVisible by remember { mutableStateOf(false) }
        var isElapsedFullScreenVisible by remember { mutableStateOf(false) }

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
                            imageVector = Icons.AutoMirrored.Filled.PlaylistAdd,
                            contentDescription = stringResource(
                                R.string.record_cd_open_canonical_catalog
                            )
                        )
                    }
                    IconButton(
                        onClick = onOpenTxtPreview,
                        modifier = Modifier.testTag(RECORD_TXT_PREVIEW_BUTTON_TEST_TAG)
                    ) {
                        Icon(
                            imageVector = Icons.Default.Description,
                            contentDescription = stringResource(R.string.record_cd_open_txt_preview)
                        )
                    }
                }
            }

            if (latestActivityRecord != null) {
                Text(
                    text = stringResource(R.string.record_latest_activity_title, latestActivityRecord.activity),
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
                Text(
                    text = stringResource(
                        R.string.record_latest_activity_times,
                        formatLatestRecordBoundary(latestActivityRecord.startTime),
                        formatLatestRecordBoundary(latestActivityRecord.endTime),
                        formatExactDuration(latestActivityRecord.durationSeconds)
                    ),
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            } else if (lastRecordedActivityHierarchyLeaf.isNotBlank() &&
                lastRecordedDuration.isNotBlank()) {
                Text(
                    text = stringResource(
                        R.string.record_last_recorded_summary,
                        lastRecordedActivityHierarchyLeaf,
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
                value = activityNameInputValue,
                onValueChange = { updatedValue ->
                    activityNameInputValue = updatedValue
                    onRecordContentChange(updatedValue.text)
                },
                label = { Text(stringResource(R.string.record_label_activity_name)) },
                leadingIcon = { Icon(Icons.Default.Edit, contentDescription = null) },
                singleLine = true,
                shape = MaterialTheme.shapes.large,
                modifier = Modifier
                    .fillMaxWidth()
                    .testTag(RECORD_ACTIVITY_NAME_INPUT_TEST_TAG)
            )

            val isIntervalRunning = intervalStart.isNotBlank() && intervalEnd.isBlank()
            val hasIntervalDraft = intervalStart.isNotBlank() && intervalEnd.isNotBlank()
            if (authoringMode == RecordAuthoringMode.INTERVAL) {
                if (isIntervalRunning) {
                    val elapsedSeconds = if (intervalStartedAtEpochMs > 0L) {
                        TimeUnit.MILLISECONDS.toSeconds(
                            (currentTimeMillis - intervalStartedAtEpochMs).coerceAtLeast(0L)
                        )
                    } else {
                        0L
                    }
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Text(
                            text = stringResource(
                                R.string.record_interval_elapsed,
                                formatDurationSummary(elapsedSeconds)
                            ),
                            modifier = Modifier.weight(1f),
                            style = MaterialTheme.typography.headlineMedium,
                            color = MaterialTheme.colorScheme.primary
                        )
                        IconButton(
                            onClick = { isElapsedFullScreenVisible = true }
                        ) {
                            Icon(
                                imageVector = Icons.Default.Fullscreen,
                                contentDescription = stringResource(
                                    R.string.record_cd_open_elapsed_fullscreen
                                )
                            )
                        }
                    }
                    if (isElapsedFullScreenVisible) {
                        ElapsedFullScreenDialog(
                            elapsedSeconds = elapsedSeconds,
                            activityName = recordContent.trim(),
                            onDismiss = { isElapsedFullScreenVisible = false }
                        )
                    }
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
                                    formatIsoClockTime(intervalStart),
                                    formatIsoClockTime(intervalEnd),
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
                            previousActivityTail?.let { tail ->
                                Column(
                                    verticalArrangement = Arrangement.spacedBy(2.dp)
                                ) {
                                    Text(
                                        text = stringResource(
                                            R.string.record_previous_activity_tail,
                                            formatIsoClockTime(tail.endTime)
                                        ),
                                        style = MaterialTheme.typography.bodyMedium,
                                        color = MaterialTheme.colorScheme.onSurfaceVariant
                                    )
                                    TextButton(
                                        onClick = onUsePreviousActivityEndTime,
                                        modifier = Modifier.height(40.dp)
                                    ) {
                                        Text(
                                            stringResource(
                                                R.string.record_action_use_previous_activity_end,
                                                formatIsoClockTime(tail.endTime)
                                            )
                                        )
                                    }
                                }
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

            val remarkPreview = recordRemark.replace('\n', ' ').trim()
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .height(56.dp)
                    .clickable { isRemarkEditorVisible = true }
                    .padding(horizontal = 4.dp)
                    .testTag(RECORD_REMARK_INPUT_TEST_TAG),
                verticalAlignment = Alignment.CenterVertically
            ) {
                if (remarkPreview.isNotBlank()) {
                    Text(
                        text = stringResource(R.string.record_label_remark_optional),
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                    Spacer(modifier = Modifier.width(12.dp))
                }
                Text(
                    text = remarkPreview.ifBlank {
                        stringResource(R.string.record_label_remark_optional)
                    },
                    modifier = Modifier.weight(1f),
                    style = MaterialTheme.typography.bodyMedium,
                    color = if (remarkPreview.isBlank()) {
                        MaterialTheme.colorScheme.onSurfaceVariant
                    } else {
                        MaterialTheme.colorScheme.onSurface
                    },
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis
                )
                IconButton(onClick = { isRemarkEditorVisible = true }) {
                    Icon(
                        imageVector = if (remarkPreview.isBlank()) {
                            Icons.Default.Add
                        } else {
                            Icons.Default.Edit
                        },
                        contentDescription = stringResource(
                            if (remarkPreview.isBlank()) {
                                R.string.record_remark_add_hint
                            } else {
                                R.string.record_remark_edit_title
                            }
                        )
                    )
                }
            }

            if (isRemarkEditorVisible) {
                RecordRemarkEditSheet(
                    initialRemark = recordRemark,
                    onDismiss = { isRemarkEditorVisible = false },
                    onApply = { editedRemark ->
                        onRecordRemarkChange(editedRemark)
                        isRemarkEditorVisible = false
                    }
                )
            }

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

private fun formatLatestRecordBoundary(value: String): String =
    if (value.isBlank()) "—" else formatIsoClockTime(value).take(5)

@Composable
private fun ElapsedFullScreenDialog(
    elapsedSeconds: Long,
    activityName: String,
    onDismiss: () -> Unit
) {
    val safeElapsedSeconds = elapsedSeconds.coerceAtLeast(0L)
    // These are repeating clock faces, not progress targets: the outer arc advances in
    // completed minutes and completes hourly, while the inner arc advances continuously
    // through the current minute.
    val hourCycleProgress = hourCycleProgressForElapsedSeconds(safeElapsedSeconds)
    val minuteCyclePosition = safeElapsedSeconds.toFloat() / 60f
    val animatedMinutePosition = remember { Animatable(minuteCyclePosition) }
    var needsMinuteProgressResync by remember { mutableStateOf(false) }
    val lifecycleOwner = LocalLifecycleOwner.current
    var isTimerUiActive by remember(lifecycleOwner) {
        mutableStateOf(
            lifecycleOwner.lifecycle.currentState.isAtLeast(Lifecycle.State.STARTED)
        )
    }
    DisposableEffect(lifecycleOwner) {
        val observer = LifecycleEventObserver { _, event ->
            when (event) {
                Lifecycle.Event.ON_STOP -> {
                    isTimerUiActive = false
                    needsMinuteProgressResync = true
                }

                Lifecycle.Event.ON_START -> isTimerUiActive = true
                else -> Unit
            }
        }
        lifecycleOwner.lifecycle.addObserver(observer)
        onDispose { lifecycleOwner.lifecycle.removeObserver(observer) }
    }
    LaunchedEffect(minuteCyclePosition) {
        val shouldResyncMinuteProgress = shouldSnapElapsedMinuteProgress(
                needsForegroundResync = needsMinuteProgressResync,
                currentPosition = animatedMinutePosition.value,
                targetPosition = minuteCyclePosition
            )
        if (!isTimerUiActive || shouldResyncMinuteProgress) {
            // Time advanced while the UI was not visible.  Show the current phase immediately
            // instead of replaying the elapsed minutes as a fast-forward animation.
            animatedMinutePosition.snapTo(minuteCyclePosition)
            if (isTimerUiActive && shouldResyncMinuteProgress) {
                needsMinuteProgressResync = false
            }
        } else {
            // Keep an unbounded phase so a completed minute flows directly into the next one.
            // Only the drawing phase is wrapped; the animation itself never jumps backwards.
            animatedMinutePosition.animateTo(
                targetValue = minuteCyclePosition,
                animationSpec = tween(durationMillis = 1000, easing = LinearEasing)
            )
        }
    }
    val ringTrackColor = MaterialTheme.colorScheme.surfaceVariant
    val ringProgressColor = MaterialTheme.colorScheme.primary
    val minuteProgressColor = MaterialTheme.colorScheme.secondary
    FullscreenPage(
        onDismissRequest = onDismiss,
        backgroundColor = MaterialTheme.colorScheme.background
    ) {
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(24.dp)
            ) {
                IconButton(
                    onClick = onDismiss,
                    modifier = Modifier.align(Alignment.TopEnd)
                ) {
                    Icon(
                        imageVector = Icons.Default.FullscreenExit,
                        contentDescription = stringResource(
                            R.string.record_cd_close_elapsed_fullscreen
                        )
                    )
                }
                Column(
                    modifier = Modifier.fillMaxSize(),
                    horizontalAlignment = Alignment.CenterHorizontally,
                    verticalArrangement = Arrangement.Center
                ) {
                    if (activityName.isNotBlank()) {
                        Text(
                            text = activityName,
                            modifier = Modifier
                                .fillMaxWidth()
                                .padding(horizontal = 24.dp),
                            style = MaterialTheme.typography.headlineSmall,
                            color = MaterialTheme.colorScheme.onSurface,
                            maxLines = 2,
                            overflow = TextOverflow.Ellipsis,
                            textAlign = TextAlign.Center
                        )
                        Spacer(modifier = Modifier.height(24.dp))
                    }
                    Box(
                        modifier = Modifier
                            .size(300.dp)
                            .testTag("record_elapsed_fullscreen_timer"),
                        contentAlignment = Alignment.Center
                    ) {
                        Canvas(modifier = Modifier.fillMaxSize()) {
                            val stroke = Stroke(
                                width = 24.dp.toPx(),
                                cap = StrokeCap.Round
                            )
                            drawArc(
                                color = ringTrackColor,
                                startAngle = -90f,
                                sweepAngle = 360f,
                                useCenter = false,
                                style = stroke
                            )
                            drawArc(
                                color = ringProgressColor,
                                startAngle = -90f,
                                sweepAngle = 360f * hourCycleProgress,
                                useCenter = false,
                                style = stroke
                            )
                            val innerInset = 38.dp.toPx()
                            val innerSize = Size(
                                width = size.width - innerInset * 2f,
                                height = size.height - innerInset * 2f
                            )
                            val innerStroke = Stroke(
                                width = 12.dp.toPx(),
                                cap = StrokeCap.Round
                            )
                            drawArc(
                                color = ringTrackColor.copy(alpha = 0.55f),
                                startAngle = -90f,
                                sweepAngle = 360f,
                                useCenter = false,
                                topLeft = Offset(innerInset, innerInset),
                                size = innerSize,
                                style = innerStroke
                            )
                            drawArc(
                                color = minuteProgressColor,
                                startAngle = -90f,
                                sweepAngle = 360f * (animatedMinutePosition.value % 1f),
                                useCenter = false,
                                topLeft = Offset(innerInset, innerInset),
                                size = innerSize,
                                style = innerStroke
                            )
                        }
                        Column(horizontalAlignment = Alignment.CenterHorizontally) {
                            Text(
                                text = formatExactDuration(safeElapsedSeconds.toInt()),
                                style = MaterialTheme.typography.displaySmall,
                                color = MaterialTheme.colorScheme.onSurface
                            )
                            Text(
                                text = stringResource(
                                    R.string.record_interval_elapsed_fullscreen_title
                                ),
                                style = MaterialTheme.typography.bodyMedium,
                                color = MaterialTheme.colorScheme.onSurfaceVariant
                            )
                        }
                    }
                }
            }
    }
}

internal fun shouldSnapElapsedMinuteProgress(
    needsForegroundResync: Boolean,
    currentPosition: Float,
    targetPosition: Float
): Boolean = needsForegroundResync && currentPosition != targetPosition

internal fun hourCycleProgressForElapsedSeconds(elapsedSeconds: Long): Float {
    val safeElapsedSeconds = elapsedSeconds.coerceAtLeast(0L)
    val completedMinutesInHour = (safeElapsedSeconds % 3600L) / 60L
    return completedMinutesInHour.toFloat() / 60f
}

private fun formatExactDuration(totalSeconds: Int): String {
    val safeSeconds = totalSeconds.coerceAtLeast(0)
    val hours = safeSeconds / 3600
    val minutes = (safeSeconds % 3600) / 60
    val seconds = safeSeconds % 60
    return listOf(hours, minutes, seconds)
        .joinToString(":") { it.toString().padStart(2, '0') }
}

@Composable
@OptIn(ExperimentalMaterial3Api::class)
private fun RecordRemarkEditSheet(
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
            Text(
                text = stringResource(R.string.record_remark_edit_title),
                style = MaterialTheme.typography.headlineSmall
            )
            OutlinedTextField(
                value = remark,
                onValueChange = { remark = it },
                label = { Text(stringResource(R.string.record_label_remark_optional)) },
                modifier = Modifier.fillMaxWidth(),
                minLines = 4
            )
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.End
            ) {
                TextButton(onClick = onDismiss) {
                    Text(stringResource(R.string.record_action_cancel))
                }
                Button(onClick = { onApply(remark) }) {
                    Text(stringResource(R.string.record_action_save_remark))
                }
            }
        }
    }
}

internal fun syncActivityNameInputValue(
    currentValue: TextFieldValue,
    recordContent: String
): TextFieldValue = if (currentValue.text == recordContent) {
    currentValue
} else {
    TextFieldValue(
        text = recordContent,
        selection = TextRange(recordContent.length)
    )
}

internal const val RECORD_TXT_PREVIEW_BUTTON_TEST_TAG = "record_txt_preview_button"

internal const val RECORD_ACTIVITY_NAME_INPUT_TEST_TAG = "record_activity_name_input"

internal const val RECORD_REMARK_INPUT_TEST_TAG = "record_remark_input"
