package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.ColumnScope
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.ChevronRight
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.ExpandMore
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.RadioButton
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp

@Composable
internal fun AliasPromoteConfirmDialog(
    entry: AliasTomlEntry,
    onDismiss: () -> Unit,
    onConfirm: () -> Unit
) {
    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(stringResource(R.string.config_alias_promote_confirm_title)) },
        text = { Text(stringResource(R.string.config_alias_promote_confirm_message, entry.aliasKey)) },
        confirmButton = {
            TextButton(onClick = onConfirm) {
                Text(stringResource(R.string.config_alias_action_promote_to_group))
            }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) { Text(stringResource(android.R.string.cancel)) }
        }
    )
}

@Composable
internal fun AliasGroupAliasDialog(
    title: String,
    initialAlias: String,
    onDismiss: () -> Unit,
    onConfirm: (String) -> Unit
) {
    var alias by remember(initialAlias) { mutableStateOf(initialAlias) }
    var showError by remember { mutableStateOf(false) }
    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(title) },
        text = {
            OutlinedTextField(
                value = alias,
                onValueChange = { alias = it; showError = false },
                label = { Text(stringResource(R.string.config_alias_alias_label)) },
                modifier = Modifier.fillMaxWidth(),
                isError = showError
            )
        },
        confirmButton = {
                TextButton(onClick = {
                if (alias.trim().isEmpty()) showError = true else onConfirm(alias)
            }) { Text(stringResource(android.R.string.ok)) }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) { Text(stringResource(android.R.string.cancel)) }
        }
    )
}

@Composable
internal fun AliasManagementDialog(
    title: String,
    aliases: List<String>,
    minimumAliases: Int,
    onDismiss: () -> Unit,
    onConfirm: (List<String>) -> Unit
) {
    var draftAliases by remember(aliases) { mutableStateOf(aliases) }
    var showError by remember { mutableStateOf(false) }

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(title) },
        text = {
            Column(
                modifier = Modifier
                    .heightIn(max = 360.dp)
                    .verticalScroll(rememberScrollState()),
                verticalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                draftAliases.forEachIndexed { index, alias ->
                    Row(verticalAlignment = androidx.compose.ui.Alignment.CenterVertically) {
                        OutlinedTextField(
                            value = alias,
                            onValueChange = { value ->
                                draftAliases = draftAliases.toMutableList().also { it[index] = value }
                                showError = false
                            },
                            label = { Text(stringResource(R.string.config_alias_alias_item_label, index + 1)) },
                            modifier = Modifier.weight(1f),
                            isError = showError
                        )
                        IconButton(
                            onClick = {
                                draftAliases = draftAliases.toMutableList().also { it.removeAt(index) }
                                showError = false
                            },
                            enabled = draftAliases.size > minimumAliases
                        ) {
                            Icon(
                                Icons.Filled.Delete,
                                contentDescription = stringResource(R.string.config_alias_action_delete)
                            )
                        }
                    }
                }
                TextButton(
                    onClick = { draftAliases = draftAliases + "" },
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Icon(Icons.Filled.Add, contentDescription = null)
                    Text(stringResource(R.string.config_alias_action_add_alias_item))
                }
                if (showError) {
                    Text(
                        text = stringResource(R.string.config_alias_alias_required),
                        color = MaterialTheme.colorScheme.error,
                        style = MaterialTheme.typography.bodySmall
                    )
                }
            }
        },
        confirmButton = {
            TextButton(onClick = {
                val normalizedAliases = draftAliases.map(String::trim)
                if (
                    normalizedAliases.size < minimumAliases ||
                    normalizedAliases.any(String::isEmpty) ||
                    normalizedAliases.distinct().size != normalizedAliases.size
                ) {
                    showError = true
                } else {
                    onConfirm(normalizedAliases)
                }
            }) {
                Text(stringResource(R.string.config_alias_action_save_aliases))
            }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) { Text(stringResource(android.R.string.cancel)) }
        }
    )
}

@Composable
internal fun AliasGroupActionsDialog(
    group: AliasTomlGroup,
    onDismiss: () -> Unit,
    onEditName: () -> Unit,
    onEditAlias: () -> Unit,
    onAddAlias: () -> Unit,
    onMove: () -> Unit,
    onDelete: () -> Unit
) {
    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(group.name) },
        text = {
            Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                TextButton(onClick = onEditName, modifier = Modifier.fillMaxWidth()) {
                    Text(stringResource(R.string.config_alias_action_edit_group_name))
                }
                if (group.groupAliases.isEmpty()) {
                    TextButton(onClick = onAddAlias, modifier = Modifier.fillMaxWidth()) {
                        Text(stringResource(R.string.config_alias_action_add_alias_item))
                    }
                } else {
                    TextButton(onClick = onEditAlias, modifier = Modifier.fillMaxWidth()) {
                        Text(stringResource(R.string.config_alias_action_edit_alias))
                    }
                }
                TextButton(onClick = onMove, modifier = Modifier.fillMaxWidth()) {
                    Text(stringResource(R.string.config_alias_action_move))
                }
                TextButton(onClick = onDelete, modifier = Modifier.fillMaxWidth()) {
                    Text(
                        text = stringResource(R.string.config_alias_action_delete),
                        color = MaterialTheme.colorScheme.error
                    )
                }
            }
        },
        confirmButton = {
            TextButton(onClick = onDismiss) { Text(stringResource(android.R.string.cancel)) }
        }
    )
}

@Composable
internal fun AliasEntryActionsDialog(
    entry: AliasTomlEntry,
    onDismiss: () -> Unit,
    onEditName: () -> Unit,
    onEditAlias: () -> Unit,
    onMerge: () -> Unit,
    onPromote: () -> Unit,
    onMove: () -> Unit,
    onDelete: () -> Unit
) {
    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(entry.aliasKey) },
        text = {
            Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                TextButton(onClick = onEditName, modifier = Modifier.fillMaxWidth()) {
                    Text(stringResource(R.string.config_alias_action_edit_entry_name))
                }
                TextButton(onClick = onEditAlias, modifier = Modifier.fillMaxWidth()) {
                    Text(stringResource(R.string.config_alias_action_edit_alias))
                }
                TextButton(onClick = onMerge, modifier = Modifier.fillMaxWidth()) {
                    Text(stringResource(R.string.config_alias_action_merge))
                }
                TextButton(onClick = onPromote, modifier = Modifier.fillMaxWidth()) {
                    Text(stringResource(R.string.config_alias_action_promote_to_group))
                }
                TextButton(onClick = onMove, modifier = Modifier.fillMaxWidth()) {
                    Text(stringResource(R.string.config_alias_action_move))
                }
                TextButton(onClick = onDelete, modifier = Modifier.fillMaxWidth()) {
                    Text(
                        text = stringResource(R.string.config_alias_action_delete),
                        color = MaterialTheme.colorScheme.error
                    )
                }
            }
        },
        confirmButton = {
            TextButton(onClick = onDismiss) { Text(stringResource(android.R.string.cancel)) }
        }
    )
}

@Composable
internal fun AliasEntryMergeTargetDialog(
    source: AliasTomlEntry,
    tomlDisplayName: String,
    document: AliasTomlDocument?,
    onDismiss: () -> Unit,
    onConfirm: (String) -> Unit
) {
    val targets = document?.allAliasEntries()
        ?.filter { it.id != source.id }
        .orEmpty()
    var selected by remember(targets) { mutableStateOf(targets.firstOrNull()) }
    ActivityHierarchyTargetSelectionDialog(
        subjectDescription = stringResource(
            R.string.config_alias_merge_message,
            source.canonicalLeaf
        ),
        title = stringResource(R.string.config_alias_merge_title),
        loading = false,
        emptyMessage = stringResource(R.string.config_alias_merge_no_target),
        hasContent = targets.isNotEmpty(),
        confirmLabel = stringResource(R.string.config_alias_action_confirm_merge),
        confirmEnabled = selected != null,
        onDismiss = onDismiss,
        onConfirm = { selected?.let { onConfirm(it.id) } }
    ) {
        Text(
            text = tomlDisplayName,
            style = MaterialTheme.typography.titleSmall
        )
        targets.forEach { target ->
            AliasMoveTargetRow(
                label = target.aliasKey,
                selected = selected?.id == target.id,
                depth = 0,
                expandable = false,
                expanded = false,
                onExpand = {},
                onSelect = { selected = target }
            )
        }
    }
}

@Composable
internal fun AliasDeleteConfirmDialog(
    title: String,
    message: String,
    onDismiss: () -> Unit,
    onConfirm: () -> Unit
) {
    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(title) },
        text = { Text(message) },
        confirmButton = {
            TextButton(onClick = onConfirm) {
                Text(
                    text = stringResource(R.string.config_alias_action_delete),
                    color = MaterialTheme.colorScheme.error
                )
            }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) { Text(stringResource(android.R.string.cancel)) }
        }
    )
}

