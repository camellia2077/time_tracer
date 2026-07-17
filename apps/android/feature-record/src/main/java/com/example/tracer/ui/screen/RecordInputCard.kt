package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.Edit
import androidx.compose.material.icons.filled.Description
import androidx.compose.material.icons.filled.KeyboardArrowDown
import androidx.compose.material.icons.filled.KeyboardArrowUp
import androidx.compose.material3.Button
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.record.R
import com.example.tracer.ui.components.TracerSegmentedButtonDefaults

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
    lastRecordedActivityAlias: String,
    lastRecordedDuration: String,
    suggestionsVisible: Boolean,
    onToggleSuggestions: () -> Unit,
    onOpenTxtPreview: () -> Unit,
    onRecordNow: () -> Unit
) {
    ElevatedCard(
        modifier = Modifier.fillMaxWidth(),
        shape = MaterialTheme.shapes.extraLarge
    ) {
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
                    IconButton(
                        onClick = onOpenTxtPreview,
                        modifier = Modifier.testTag(recordTxtPreviewButtonTestTag())
                    ) {
                        Icon(
                            imageVector = Icons.Default.Description,
                            contentDescription = stringResource(R.string.record_cd_open_txt_preview)
                        )
                    }
                    TextButton(onClick = onToggleSuggestions) {
                        Text(stringResource(R.string.record_action_suggestions))
                        Spacer(modifier = Modifier.width(4.dp))
                        Icon(
                            imageVector = if (suggestionsVisible) {
                                Icons.Default.KeyboardArrowUp
                            } else {
                                Icons.Default.KeyboardArrowDown
                            },
                            contentDescription = null
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
                    RecordAuthoringMode.POINT to stringResource(R.string.record_mode_point),
                    RecordAuthoringMode.INTERVAL to stringResource(R.string.record_mode_interval)
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
                modifier = Modifier.fillMaxWidth()
            )

            if (authoringMode == RecordAuthoringMode.INTERVAL) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    OutlinedTextField(
                        value = intervalStart,
                        onValueChange = onIntervalStartChange,
                        label = { Text(stringResource(R.string.record_label_interval_start)) },
                        singleLine = true,
                        shape = MaterialTheme.shapes.large,
                        modifier = Modifier.weight(1f)
                    )
                    OutlinedTextField(
                        value = intervalEnd,
                        onValueChange = onIntervalEndChange,
                        label = { Text(stringResource(R.string.record_label_interval_end)) },
                        singleLine = true,
                        shape = MaterialTheme.shapes.large,
                        modifier = Modifier.weight(1f)
                    )
                }
            }

            OutlinedTextField(
                value = recordRemark,
                onValueChange = onRecordRemarkChange,
                label = { Text(stringResource(R.string.record_label_remark_optional)) },
                leadingIcon = { Icon(Icons.Default.Edit, contentDescription = null) },
                singleLine = true,
                shape = MaterialTheme.shapes.large,
                modifier = Modifier.fillMaxWidth()
            )

            Button(
                onClick = onRecordNow,
                modifier = Modifier
                    .fillMaxWidth()
                    .height(56.dp)
            ) {
                Icon(Icons.Default.Check, contentDescription = null)
                Spacer(modifier = Modifier.width(8.dp))
                Text(
                    stringResource(
                        if (authoringMode == RecordAuthoringMode.INTERVAL) {
                            R.string.record_action_record_interval
                        } else {
                            R.string.record_action_record_activity
                        }
                    )
                )
            }
        }
    }
}

internal fun recordTxtPreviewButtonTestTag(): String = "record_txt_preview_button"
