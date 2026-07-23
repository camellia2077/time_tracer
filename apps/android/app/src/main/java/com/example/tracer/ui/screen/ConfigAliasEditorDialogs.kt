package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material3.AlertDialog
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
internal fun AliasGroupNameDialog(
    title: String,
    initialName: String,
    onDismiss: () -> Unit,
    onConfirm: (String) -> Unit
) {
    var value by remember(initialName) { mutableStateOf(initialName) }
    var showError by remember { mutableStateOf(false) }

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(title) },
        text = {
            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                OutlinedTextField(
                    value = value,
                    onValueChange = {
                        value = it
                        showError = false
                    },
                    label = { Text(stringResource(R.string.config_alias_group_name_label)) },
                    modifier = Modifier.fillMaxWidth(),
                    isError = showError
                )
                if (showError) {
                    Text(
                        text = stringResource(R.string.config_alias_group_name_required),
                        color = MaterialTheme.colorScheme.error,
                        style = MaterialTheme.typography.bodySmall
                    )
                }
            }
        },
        confirmButton = {
            TextButton(
                onClick = {
                    if (value.trim().isEmpty()) {
                        showError = true
                    } else {
                        onConfirm(value)
                    }
                }
            ) {
                Text(stringResource(android.R.string.ok))
            }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) {
                Text(stringResource(android.R.string.cancel))
            }
        }
    )
}

@Composable
internal fun AliasEntryDialog(
    title: String,
    initialAliasKey: String,
    initialCanonicalLeaf: String,
    showCanonicalLeafField: Boolean = true,
    onDismiss: () -> Unit,
    onConfirm: (String, String) -> Unit
) {
    var aliasKey by remember(initialAliasKey) { mutableStateOf(initialAliasKey) }
    var canonicalLeaf by remember(initialCanonicalLeaf) { mutableStateOf(initialCanonicalLeaf) }
    var showError by remember { mutableStateOf(false) }

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(title) },
        text = {
            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                OutlinedTextField(
                    value = aliasKey,
                    onValueChange = {
                        aliasKey = it
                        showError = false
                    },
                    label = { Text(stringResource(R.string.config_alias_alias_key_label)) },
                    modifier = Modifier.fillMaxWidth(),
                    isError = showError
                )
                if (showCanonicalLeafField) {
                    OutlinedTextField(
                        value = canonicalLeaf,
                        onValueChange = {
                            canonicalLeaf = it
                            showError = false
                        },
                        label = { Text(stringResource(R.string.config_alias_canonical_leaf_label)) },
                        modifier = Modifier.fillMaxWidth(),
                        isError = showError
                    )
                } else {
                    Text(
                        text = stringResource(
                            R.string.config_alias_canonical_leaf_readonly,
                            initialCanonicalLeaf
                        ),
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
                if (showError) {
                    Text(
                        text = stringResource(
                            if (showCanonicalLeafField) {
                                R.string.config_alias_entry_required
                            } else {
                                R.string.config_alias_alias_key_required
                            }
                        ),
                        color = MaterialTheme.colorScheme.error,
                        style = MaterialTheme.typography.bodySmall
                    )
                }
            }
        },
        confirmButton = {
            TextButton(
                onClick = {
                    val isInvalid = if (showCanonicalLeafField) {
                        aliasKey.trim().isEmpty() || canonicalLeaf.trim().isEmpty()
                    } else {
                        aliasKey.trim().isEmpty()
                    }
                    if (isInvalid) {
                        showError = true
                    } else {
                        onConfirm(aliasKey, canonicalLeaf)
                    }
                }
            ) {
                Text(stringResource(android.R.string.ok))
            }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) {
                Text(stringResource(android.R.string.cancel))
            }
        }
    )
}

@Composable
internal fun AliasEntryMoveTargetDialog(
    entry: AliasTomlEntry,
    destinations: List<AliasEntryMoveDestination>,
    onDismiss: () -> Unit,
    onConfirm: (String) -> Unit
) {
    var selectedGroupId by remember(destinations) {
        mutableStateOf(destinations.firstOrNull()?.groupId)
    }

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(stringResource(R.string.config_alias_dialog_move_entry_title)) },
        text = {
            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                Text(
                    text = stringResource(R.string.config_alias_move_entry_label, entry.aliasKey),
                    style = MaterialTheme.typography.bodyMedium
                )
                if (destinations.isEmpty()) {
                    Text(
                        text = stringResource(R.string.config_alias_move_no_destination),
                        color = MaterialTheme.colorScheme.onSurfaceVariant,
                        style = MaterialTheme.typography.bodySmall
                    )
                } else {
                    destinations.forEach { destination ->
                        val selected = selectedGroupId == destination.groupId
                        TextButton(
                            onClick = { selectedGroupId = destination.groupId },
                            modifier = Modifier.fillMaxWidth()
                        ) {
                            RadioButton(
                                selected = selected,
                                onClick = { selectedGroupId = destination.groupId }
                            )
                            Text(destination.groupPath.joinToString(" / "))
                        }
                    }
                }
            }
        },
        confirmButton = {
            TextButton(
                enabled = selectedGroupId != null,
                onClick = { selectedGroupId?.let(onConfirm) }
            ) {
                Text(stringResource(R.string.config_alias_action_preview_move))
            }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) {
                Text(stringResource(android.R.string.cancel))
            }
        }
    )
}

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
                label = { Text(stringResource(R.string.config_alias_record_name_label)) },
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
internal fun AliasGroupActionsDialog(
    group: AliasTomlGroup,
    onDismiss: () -> Unit,
    onEditAlias: (String) -> Unit,
    onAddAlias: () -> Unit,
    onDelete: () -> Unit
) {
    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(group.name) },
        text = {
            Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                group.groupAliases.forEach { alias ->
                    TextButton(
                        onClick = { onEditAlias(alias) },
                        modifier = Modifier.fillMaxWidth()
                    ) {
                        Text(stringResource(R.string.config_alias_edit_record_name_item, alias))
                    }
                }
                TextButton(onClick = onAddAlias, modifier = Modifier.fillMaxWidth()) {
                    Text(stringResource(R.string.config_alias_action_add_record_name))
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
    onEdit: () -> Unit,
    onPromote: () -> Unit,
    onMove: () -> Unit,
    onDelete: () -> Unit
) {
    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(entry.aliasKey) },
        text = {
            Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                TextButton(onClick = onEdit, modifier = Modifier.fillMaxWidth()) {
                    Text(stringResource(R.string.config_alias_action_rename))
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
