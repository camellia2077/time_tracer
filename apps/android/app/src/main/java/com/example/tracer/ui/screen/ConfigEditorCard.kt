package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.ui.components.NativeMultilineTextEditor
import kotlinx.coroutines.delay

@Composable
internal fun ConfigEditorCard(
    selectedFileDisplayName: String,
    selectedFileContent: String,
    editableContent: String,
    autoSaveStatus: ConfigAutoSaveStatus,
    onEditableContentChange: (String) -> Unit,
    onSaveCurrentFile: () -> Unit,
    readOnly: Boolean = false
) {
    if (!readOnly) {
        LaunchedEffect(editableContent, selectedFileContent) {
            if (editableContent == selectedFileContent) {
                return@LaunchedEffect
            }
            delay(CONFIG_EDITOR_AUTO_SAVE_DELAY_MS)
            if (editableContent != selectedFileContent) {
                onSaveCurrentFile()
            }
        }
    }

    Column(
        modifier = Modifier
            .fillMaxWidth()
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
            Text(
                text = stringResource(R.string.config_title_editor_file, selectedFileDisplayName),
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary
            )

            Text(
                text = stringResource(R.string.config_label_toml_content),
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )

            if (readOnly) {
                Text(
                    text = stringResource(R.string.config_program_resource_read_only),
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }

            ConfigAutoSaveStatusText(
                autoSaveStatus = autoSaveStatus,
                modifier = Modifier.fillMaxWidth()
            )

            NativeMultilineTextEditor(
                value = editableContent,
                onValueChange = onEditableContentChange,
                modifier = Modifier.fillMaxWidth(),
                minLines = 12,
                monospace = true,
                readOnly = readOnly
            )
    }
}

private const val CONFIG_EDITOR_AUTO_SAVE_DELAY_MS = 600L
