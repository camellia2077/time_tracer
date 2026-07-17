package com.example.tracer

import androidx.compose.material3.AlertDialog
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.res.stringResource

@Composable
internal fun ConfigTomlCreationDialog(
    onDismiss: () -> Unit,
    onConfirm: (String) -> Unit
) {
    var fileName by remember { mutableStateOf("") }
    val normalizedFileName = fileName.trim()
    val isValid = normalizedFileName.isNotEmpty() &&
        normalizedFileName.none { character ->
            character == '/' || character == '\\' || character.isISOControl()
        }

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(stringResource(R.string.config_title_new_alias_mapping)) },
        text = {
            OutlinedTextField(
                value = fileName,
                onValueChange = { fileName = it },
                label = { Text(stringResource(R.string.config_label_new_alias_file_name)) },
                supportingText = {
                    Text(stringResource(R.string.config_new_alias_file_location))
                },
                isError = fileName.isNotBlank() && !isValid,
                singleLine = true
            )
        },
        confirmButton = {
            TextButton(
                onClick = { onConfirm(normalizedFileName) },
                enabled = isValid
            ) {
                Text(stringResource(R.string.config_action_create))
            }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) {
                Text(stringResource(R.string.config_action_cancel))
            }
        }
    )
}
