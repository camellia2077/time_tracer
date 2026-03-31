package com.example.tracer

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.text.KeyboardActions
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ExpandLess
import androidx.compose.material.icons.filled.ExpandMore
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.Icon
import androidx.compose.material3.InputChip
import androidx.compose.material3.InputChipDefaults
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalSoftwareKeyboardController
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.record.R

@Composable
internal fun RecordQuickAccessCard(
    recordContent: String,
    onRecordContentChange: (String) -> Unit,
    quickActivities: List<String>,
    availableActivityNames: List<String>,
    onQuickActivitiesUpdate: (List<String>) -> Boolean,
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
    val keyboardController = LocalSoftwareKeyboardController.current
    val searchToken = quickActivitySearch.trim()
    val candidates = if (searchToken.isEmpty()) {
        emptyList()
    } else {
        availableActivityNames.filter {
            !quickActivities.contains(it) && it.contains(searchToken, true)
        }.take(5)
    }

    fun tryAddSearchToken(): Boolean {
        if (searchToken.isEmpty()) {
            return false
        }
        val updated = (quickActivities + searchToken)
            .distinct()
            .take(maxQuickActivityCount)
        return onQuickActivitiesUpdate(updated)
    }

    ElevatedCard(
        modifier = Modifier.fillMaxWidth(),
        shape = MaterialTheme.shapes.extraLarge
    ) {
        Column(modifier = Modifier.fillMaxWidth()) {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .clickable { onToggleAssistSettings() }
                    .padding(16.dp),
                verticalArrangement = Arrangement.spacedBy(4.dp)
            ) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
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
                Text(
                    text = stringResource(R.string.record_hint_long_press_drag_sort),
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
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
                            keyboardOptions = KeyboardOptions(
                                keyboardType = KeyboardType.Number
                            )
                        )
                        OutlinedTextField(
                            value = suggestionTopN.toString(),
                            onValueChange = onSuggestionTopNChange,
                            label = { Text(stringResource(R.string.record_label_top_n)) },
                            modifier = Modifier.weight(1f),
                            keyboardOptions = KeyboardOptions(
                                keyboardType = KeyboardType.Number
                            )
                        )
                    }

                    OutlinedTextField(
                        value = quickActivitySearch,
                        onValueChange = onQuickActivitySearchChange,
                        label = {
                            Text(stringResource(R.string.record_label_search_add_quick_activity))
                        },
                        modifier = Modifier.fillMaxWidth(),
                        keyboardOptions = KeyboardOptions(imeAction = ImeAction.Done),
                        keyboardActions = KeyboardActions(
                            onDone = {
                                if (tryAddSearchToken()) {
                                    onQuickActivitySearchChange("")
                                    keyboardController?.hide()
                                }
                            }
                        ),
                        trailingIcon = {
                            TextButton(
                                onClick = {
                                    if (tryAddSearchToken()) {
                                        onQuickActivitySearchChange("")
                                        keyboardController?.hide()
                                    }
                                },
                                enabled = searchToken.isNotEmpty()
                            ) {
                                Text(stringResource(R.string.record_action_add_quick_activity))
                            }
                        }
                    )

                    if (searchToken.isNotEmpty()) {
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
                                        if (onQuickActivitiesUpdate(updated)) {
                                            onQuickActivitySearchChange("")
                                            keyboardController?.hide()
                                        }
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
                        if (candidates.isEmpty()) {
                            Text(
                                text = stringResource(R.string.record_hint_no_matching_alias_key),
                                style = MaterialTheme.typography.bodySmall,
                                color = MaterialTheme.colorScheme.onSurfaceVariant
                            )
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
                    QuickAccessActivityGrid(
                        modifier = Modifier.fillMaxWidth(),
                        quickActivities = quickActivities,
                        recordContent = recordContent,
                        isDeleteMode = assistSettingsExpanded,
                        onRecordContentChange = onRecordContentChange,
                        onQuickActivitiesUpdate = onQuickActivitiesUpdate
                    )
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
