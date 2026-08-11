package com.example.tracer

import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.width
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Add
import androidx.compose.material3.Button
import androidx.compose.material3.Icon
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp

@Composable
internal fun ActivityHierarchyFileControls(
    onCreateAliasTomlFile: (String) -> Unit
) {
    var showCreateTomlDialog by remember { mutableStateOf(false) }

    Button(
        onClick = { showCreateTomlDialog = true },
        modifier = Modifier.fillMaxWidth()
    ) {
        Icon(Icons.Filled.Add, contentDescription = null)
        Spacer(modifier = Modifier.width(8.dp))
        Text(stringResource(R.string.config_action_new_alias_mapping))
    }

    if (showCreateTomlDialog) {
        ConfigTomlCreationDialog(
            onDismiss = { showCreateTomlDialog = false },
            onConfirm = { fileName ->
                showCreateTomlDialog = false
                onCreateAliasTomlFile(fileName)
            }
        )
    }
}
