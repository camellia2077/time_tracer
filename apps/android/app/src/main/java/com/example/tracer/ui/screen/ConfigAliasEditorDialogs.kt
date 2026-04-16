package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
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
                if (showError) {
                    Text(
                        text = stringResource(R.string.config_alias_entry_required),
                        color = MaterialTheme.colorScheme.error,
                        style = MaterialTheme.typography.bodySmall
                    )
                }
            }
        },
        confirmButton = {
            TextButton(
                onClick = {
                    if (aliasKey.trim().isEmpty() || canonicalLeaf.trim().isEmpty()) {
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
