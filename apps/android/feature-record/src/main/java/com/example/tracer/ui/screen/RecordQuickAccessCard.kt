package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.text.KeyboardActions
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.TrendingUp
import androidx.compose.material.icons.filled.AccountTree
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.Done
import androidx.compose.material.icons.filled.Edit
import androidx.compose.material.icons.filled.ExpandLess
import androidx.compose.material.icons.filled.ExpandMore
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.InputChip
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalSoftwareKeyboardController
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.record.R

@OptIn(ExperimentalMaterial3Api::class)
@Composable
internal fun RecordQuickAccessCard(
    recordContent: String,
    onRecordContentChange: (String) -> Unit,
    quickActivities: List<String>,
    availableActivityNames: List<String>,
    onQuickActivitiesUpdate: (List<String>) -> Boolean,
    quickAccessCardExpanded: Boolean = true,
    onToggleQuickAccessCard: () -> Unit = {},
    quickAccessAddSheetVisible: Boolean,
    onToggleQuickAccessAddSheet: () -> Unit,
    quickAccessManageMode: Boolean,
    onToggleQuickAccessManageMode: () -> Unit,
    onToggleFrequentActivities: () -> Unit = {},
    onOpenQuickAccessCanonicalCatalog: () -> Unit,
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
                    Row(
                        horizontalArrangement = Arrangement.spacedBy(4.dp),
                        verticalAlignment = androidx.compose.ui.Alignment.CenterVertically
                    ) {
                        if (quickActivities.isNotEmpty()) {
                            IconButton(onClick = onToggleQuickAccessManageMode) {
                                Icon(
                                    imageVector = if (quickAccessManageMode) {
                                        Icons.Default.Done
                                    } else {
                                        Icons.Default.Edit
                                    },
                                    contentDescription = stringResource(
                                        if (quickAccessManageMode) {
                                            R.string.record_cd_finish_manage_quick_access
                                        } else {
                                            R.string.record_cd_manage_quick_access
                                        }
                                    ),
                                    tint = if (quickAccessManageMode) {
                                        MaterialTheme.colorScheme.primary
                                    } else {
                                        MaterialTheme.colorScheme.onSurfaceVariant
                                    }
                                )
                            }
                        }
                        IconButton(onClick = onToggleQuickAccessAddSheet) {
                            Icon(
                                imageVector = Icons.Default.Add,
                                contentDescription = stringResource(
                                    R.string.record_cd_add_quick_access
                                ),
                                tint = MaterialTheme.colorScheme.onSurfaceVariant
                            )
                        }
                        IconButton(onClick = onToggleQuickAccessCard) {
                            Icon(
                                imageVector = if (quickAccessCardExpanded) {
                                    Icons.Default.ExpandLess
                                } else {
                                    Icons.Default.ExpandMore
                                },
                                contentDescription = if (quickAccessCardExpanded) {
                                    stringResource(R.string.record_cd_collapse)
                                } else {
                                    stringResource(R.string.record_cd_expand)
                                },
                                tint = MaterialTheme.colorScheme.primary
                            )
                        }
                    }
                }
            }

            if (quickAccessCardExpanded) {
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
                        isDeleteMode = quickAccessManageMode,
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

    if (quickAccessAddSheetVisible) {
        ModalBottomSheet(onDismissRequest = onToggleQuickAccessAddSheet) {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 24.dp)
                    .padding(bottom = 24.dp),
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                Text(
                    text = stringResource(R.string.record_title_add_quick_access),
                    style = MaterialTheme.typography.titleLarge,
                    color = MaterialTheme.colorScheme.primary
                )
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

                Column(
                    modifier = Modifier.fillMaxWidth(),
                    verticalArrangement = Arrangement.spacedBy(4.dp)
                ) {
                    TextButton(
                        onClick = onToggleFrequentActivities,
                        modifier = Modifier
                            .fillMaxWidth()
                            .heightIn(min = 56.dp),
                        contentPadding = PaddingValues(horizontal = 16.dp, vertical = 8.dp)
                    ) {
                        Row(
                            modifier = Modifier.fillMaxWidth(),
                            verticalAlignment = androidx.compose.ui.Alignment.CenterVertically
                        ) {
                            Icon(
                                imageVector = Icons.AutoMirrored.Filled.TrendingUp,
                                contentDescription = null,
                                modifier = Modifier.size(24.dp)
                            )
                            Spacer(modifier = Modifier.width(12.dp))
                            Text(
                                text = stringResource(R.string.record_action_add_from_frequent),
                                style = MaterialTheme.typography.bodyLarge
                            )
                        }
                    }

                    TextButton(
                        onClick = onOpenQuickAccessCanonicalCatalog,
                        modifier = Modifier
                            .fillMaxWidth()
                            .heightIn(min = 56.dp),
                        contentPadding = PaddingValues(horizontal = 16.dp, vertical = 8.dp)
                    ) {
                        Row(
                            modifier = Modifier.fillMaxWidth(),
                            verticalAlignment = androidx.compose.ui.Alignment.CenterVertically
                        ) {
                            Icon(
                                imageVector = Icons.Default.AccountTree,
                                contentDescription = stringResource(
                                    R.string.record_cd_open_quick_access_catalog
                                ),
                                modifier = Modifier.size(24.dp)
                            )
                            Spacer(modifier = Modifier.width(12.dp))
                            Text(
                                text = stringResource(
                                    R.string.record_action_browse_quick_access_catalog
                                ),
                                style = MaterialTheme.typography.bodyLarge
                            )
                        }
                    }
                }
            }
        }
    }
}
}
