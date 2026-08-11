package com.example.tracer

import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.ui.res.stringResource

@Composable
internal fun ActivityHierarchyTomlDeleteDialog(
    fileName: String,
    onDismiss: () -> Unit,
    onConfirm: () -> Unit
) {
    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(stringResource(R.string.config_title_delete_alias_mapping, fileName)) },
        text = { Text(stringResource(R.string.config_delete_alias_mapping_warning)) },
        confirmButton = {
            TextButton(onClick = onConfirm) {
                Text(stringResource(R.string.config_action_delete))
            }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) {
                Text(stringResource(R.string.config_action_cancel))
            }
        }
    )
}
