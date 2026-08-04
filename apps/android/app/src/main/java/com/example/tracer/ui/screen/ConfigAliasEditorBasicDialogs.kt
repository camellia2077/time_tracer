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
    initialCanonicalLeaf: String,
    initialAliases: List<String>,
    showCanonicalLeafField: Boolean = true,
    onDismiss: () -> Unit,
    onConfirm: (String, List<String>) -> Unit
) {
    var canonicalLeaf by remember(initialCanonicalLeaf) { mutableStateOf(initialCanonicalLeaf) }
    var aliases by remember(initialAliases) { mutableStateOf(initialAliases) }
    var showError by remember { mutableStateOf(false) }

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(title) },
        text = {
            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
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
                Text(
                    text = stringResource(R.string.config_alias_aliases_label),
                    style = MaterialTheme.typography.labelLarge
                )
                aliases.forEachIndexed { index, alias ->
                    Row(verticalAlignment = androidx.compose.ui.Alignment.CenterVertically) {
                        OutlinedTextField(
                            value = alias,
                            onValueChange = { value ->
                                aliases = aliases.toMutableList().also { it[index] = value }
                                showError = false
                            },
                            label = { Text(stringResource(R.string.config_alias_alias_item_label, index + 1)) },
                            modifier = Modifier.weight(1f),
                            isError = showError
                        )
                        IconButton(
                            onClick = {
                                aliases = aliases.toMutableList().also { it.removeAt(index) }
                                showError = false
                            },
                            enabled = aliases.size > 1
                        ) {
                            Icon(Icons.Filled.Delete, contentDescription = stringResource(R.string.config_alias_action_delete))
                        }
                    }
                }
                TextButton(
                    onClick = { aliases = aliases + "" },
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Icon(Icons.Filled.Add, contentDescription = null)
                    Text(stringResource(R.string.config_alias_action_add_alias_item))
                }
                if (showError) {
                    Text(
                        text = stringResource(
                            if (showCanonicalLeafField) {
                                R.string.config_alias_entry_required
                            } else {
                                R.string.config_alias_alias_required
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
                    val normalizedAliases = aliases.map(String::trim)
                    val isInvalid = canonicalLeaf.trim().isEmpty() ||
                        normalizedAliases.isEmpty() ||
                        normalizedAliases.any(String::isEmpty) ||
                        normalizedAliases.distinct().size != normalizedAliases.size
                    if (isInvalid) {
                        showError = true
                    } else {
                        onConfirm(canonicalLeaf, normalizedAliases)
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

