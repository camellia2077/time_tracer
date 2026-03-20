package com.example.tracer

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ExpandLess
import androidx.compose.material.icons.filled.ExpandMore
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.Icon
import androidx.compose.material3.InputChip
import androidx.compose.material3.InputChipDefaults
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.SuggestionChip
import androidx.compose.material3.SuggestionChipDefaults
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.record.R

@Composable
internal fun RecordQuickAccessCard(
    recordContent: String,
    onRecordContentChange: (String) -> Unit,
    quickActivities: List<String>,
    availableActivityNames: List<String>,
    onQuickActivitiesUpdate: (List<String>) -> Unit,
    assistSettingsExpanded: Boolean,
    onToggleAssistSettings: () -> Unit,
    suggestionLookbackDays: Int,
    suggestionTopN: Int,
    onSuggestionLookbackDaysChange: (String) -> Unit,
    onSuggestionTopNChange: (String) -> Unit,
    quickActivitySearch: String,
    onQuickActivitySearchChange: (String) -> Unit,
    maxQuickActivityCount: Int
) {
    ElevatedCard(
        modifier = Modifier.fillMaxWidth(),
        shape = MaterialTheme.shapes.extraLarge
    ) {
        Column(modifier = Modifier.fillMaxWidth()) {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .clickable { onToggleAssistSettings() }
                    .padding(16.dp),
                verticalAlignment = androidx.compose.ui.Alignment.CenterVertically,
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                Text(
                    text = stringResource(R.string.record_title_quick_access),
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.primary
                )
                Icon(
                    imageVector = if (assistSettingsExpanded) {
                        Icons.Default.ExpandLess
                    } else {
                        Icons.Default.ExpandMore
                    },
                    contentDescription = if (assistSettingsExpanded) {
                        stringResource(R.string.record_cd_collapse)
                    } else {
                        stringResource(R.string.record_cd_expand)
                    },
                    tint = MaterialTheme.colorScheme.primary
                )
            }

            AnimatedVisibility(visible = assistSettingsExpanded) {
                Column(
                    modifier = Modifier
                        .padding(horizontal = 16.dp)
                        .padding(bottom = 12.dp),
                    verticalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    Text(
                        stringResource(R.string.record_title_assist_settings),
                        style = MaterialTheme.typography.titleSmall
                    )
                    Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                        OutlinedTextField(
                            value = suggestionLookbackDays.toString(),
                            onValueChange = onSuggestionLookbackDaysChange,
                            label = { Text(stringResource(R.string.record_label_days)) },
                            modifier = Modifier.weight(1f),
                            keyboardOptions = androidx.compose.foundation.text.KeyboardOptions(
                                keyboardType = androidx.compose.ui.text.input.KeyboardType.Number
                            )
                        )
                        OutlinedTextField(
                            value = suggestionTopN.toString(),
                            onValueChange = onSuggestionTopNChange,
                            label = { Text(stringResource(R.string.record_label_top_n)) },
                            modifier = Modifier.weight(1f),
                            keyboardOptions = androidx.compose.foundation.text.KeyboardOptions(
                                keyboardType = androidx.compose.ui.text.input.KeyboardType.Number
                            )
                        )
                    }

                    OutlinedTextField(
                        value = quickActivitySearch,
                        onValueChange = onQuickActivitySearchChange,
                        label = {
                            Text(stringResource(R.string.record_label_search_add_quick_activity))
                        },
                        modifier = Modifier.fillMaxWidth()
                    )

                    val searchToken = quickActivitySearch.trim()
                    if (searchToken.isNotEmpty()) {
                        val candidates = availableActivityNames.filter {
                            !quickActivities.contains(it) && it.contains(searchToken, true)
                        }.take(5)
                        com.example.tracer.ui.components.SimpleFlowRow(
                            horizontalGap = 8.dp,
                            verticalGap = 8.dp
                        ) {
                            candidates.forEach { candidate ->
                                InputChip(
                                    selected = false,
                                    onClick = {
                                        val updated = (quickActivities + candidate)
                                            .distinct()
                                            .take(maxQuickActivityCount)
                                        onQuickActivitiesUpdate(updated)
                                    },
                                    label = {
                                        Text(
                                            stringResource(
                                                R.string.record_chip_add_activity,
                                                candidate
                                            )
                                        )
                                    }
                                )
                            }
                        }
                    }
                }
            }

            Column(
                modifier = Modifier
                    .padding(horizontal = 16.dp)
                    .padding(bottom = 16.dp)
            ) {
                if (quickActivities.isNotEmpty()) {
                    com.example.tracer.ui.components.SimpleFlowRow(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalGap = 8.dp,
                        verticalGap = 8.dp
                    ) {
                        quickActivities.forEach { activity ->
                            val isSelected = recordContent.trim() == activity
                            val isDeleting = assistSettingsExpanded

                            if (isDeleting) {
                                InputChip(
                                    selected = isSelected,
                                    onClick = {
                                        onQuickActivitiesUpdate(
                                            quickActivities.filter { it != activity }
                                        )
                                    },
                                    label = {
                                        Text(
                                            stringResource(
                                                R.string.record_chip_remove_activity,
                                                activity
                                            )
                                        )
                                    },
                                    colors = InputChipDefaults.inputChipColors(
                                        containerColor = MaterialTheme.colorScheme.errorContainer,
                                        labelColor = MaterialTheme.colorScheme.onErrorContainer
                                    )
                                )
                            } else {
                                SuggestionChip(
                                    onClick = { onRecordContentChange(activity) },
                                    label = {
                                        Text(
                                            text = activity,
                                            style = MaterialTheme.typography.bodyMedium,
                                            modifier = Modifier.padding(vertical = 4.dp),
                                            maxLines = 1,
                                            overflow = TextOverflow.Ellipsis
                                        )
                                    },
                                    colors = SuggestionChipDefaults.suggestionChipColors(
                                        containerColor = if (isSelected) {
                                            MaterialTheme.colorScheme.primaryContainer
                                        } else {
                                            MaterialTheme.colorScheme.surfaceContainerHigh
                                        },
                                        labelColor = if (isSelected) {
                                            MaterialTheme.colorScheme.onPrimaryContainer
                                        } else {
                                            MaterialTheme.colorScheme.onSurface
                                        }
                                    ),
                                    border = SuggestionChipDefaults.suggestionChipBorder(
                                        enabled = true,
                                        borderColor = if (isSelected) {
                                            MaterialTheme.colorScheme.primary
                                        } else {
                                            MaterialTheme.colorScheme.outline
                                        }
                                    )
                                )
                            }
                        }
                    }
                } else {
                    Text(
                        stringResource(R.string.record_hint_no_quick_activities),
                        style = MaterialTheme.typography.bodySmall
                    )
                }
            }
        }
    }
}
