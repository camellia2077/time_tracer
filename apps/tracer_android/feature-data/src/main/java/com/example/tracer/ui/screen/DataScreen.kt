package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
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
import com.example.tracer.feature.data.R

private enum class DestructiveAction {
    ClearTxt,
    ClearAllData
}

@Composable
fun DataManagementSection(
    modifier: Modifier = Modifier,
    onIngestSmoke: () -> Unit,
    onIngestFull: () -> Unit,
    onImportSingleTxt: () -> Unit,
    canExportAllMonthsTxt: Boolean,
    onExportAllMonthsTxt: () -> Unit,
    isTxtExportInProgress: Boolean,
    onClearTxt: () -> Unit,
    onClearData: () -> Unit
) {
    var pendingAction by remember { mutableStateOf<DestructiveAction?>(null) }

    androidx.compose.foundation.lazy.LazyColumn(
        modifier = modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(16.dp, androidx.compose.ui.Alignment.Bottom)
    ) {
        item {
            // Data Ingestion Card
            ElevatedCard(modifier = Modifier.fillMaxWidth()) {
                Column(
                    modifier = Modifier.padding(16.dp),
                    verticalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    Text(
                        text = stringResource(R.string.data_title_ingestion),
                        style = MaterialTheme.typography.titleMedium,
                        color = MaterialTheme.colorScheme.primary
                    )
                    
                    Button(
                        onClick = onIngestSmoke,
                        modifier = Modifier.fillMaxWidth()
                    ) {
                        Icon(Icons.Filled.Refresh, contentDescription = null)
                        Spacer(modifier = Modifier.width(8.dp))
                        Text(stringResource(R.string.data_action_ingest_smoke))
                    }

                    Button(
                        onClick = onIngestFull,
                        modifier = Modifier.fillMaxWidth()
                    ) {
                        Icon(Icons.Filled.Refresh, contentDescription = null)
                        Spacer(modifier = Modifier.width(8.dp))
                        Text(stringResource(R.string.data_action_ingest_full))
                    }

                    Button(
                        onClick = onImportSingleTxt,
                        modifier = Modifier.fillMaxWidth()
                    ) {
                        Icon(Icons.Filled.Refresh, contentDescription = null)
                        Spacer(modifier = Modifier.width(8.dp))
                        Text(stringResource(R.string.data_action_import_single_txt))
                    }
                }
            }
        }

        item {
            // TXT Export Card
            ElevatedCard(modifier = Modifier.fillMaxWidth()) {
                Column(
                    modifier = Modifier.padding(16.dp),
                    verticalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    Text(
                        text = stringResource(R.string.data_title_txt_export),
                        style = MaterialTheme.typography.titleMedium,
                        color = MaterialTheme.colorScheme.primary
                    )

                    Button(
                        onClick = onExportAllMonthsTxt,
                        enabled = canExportAllMonthsTxt && !isTxtExportInProgress,
                        modifier = Modifier.fillMaxWidth()
                    ) {
                        Text(
                            if (isTxtExportInProgress) {
                                stringResource(R.string.data_action_exporting)
                            } else {
                                stringResource(R.string.data_action_export_all_months_txt)
                            }
                        )
                    }
                }
            }
        }

        item {
            // Maintenance & Debug Card
            ElevatedCard(modifier = Modifier.fillMaxWidth()) {
                Column(
                    modifier = Modifier.padding(16.dp),
                    verticalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    Text(
                        text = stringResource(R.string.data_title_maintenance_debug),
                        style = MaterialTheme.typography.titleMedium,
                        color = MaterialTheme.colorScheme.primary
                    )

                    OutlinedButton(
                        onClick = { pendingAction = DestructiveAction.ClearTxt },
                        modifier = Modifier.fillMaxWidth(),
                        colors = ButtonDefaults.outlinedButtonColors(
                            contentColor = MaterialTheme.colorScheme.error
                        )
                    ) {
                        Icon(Icons.Filled.Delete, contentDescription = null)
                        Spacer(modifier = Modifier.width(8.dp))
                        Text(stringResource(R.string.data_action_clear_txt))
                    }

                    Button(
                        onClick = { pendingAction = DestructiveAction.ClearAllData },
                        modifier = Modifier.fillMaxWidth(),
                        colors = ButtonDefaults.buttonColors(
                            containerColor = MaterialTheme.colorScheme.error
                        )
                    ) {
                        Icon(Icons.Filled.Delete, contentDescription = null)
                        Spacer(modifier = Modifier.width(8.dp))
                        Text(stringResource(R.string.data_action_clear_all_app_data))
                    }
                }
            }
        }
    }

    val currentAction = pendingAction
    if (currentAction != null) {
        val (titleRes, messageRes) = when (currentAction) {
            DestructiveAction.ClearTxt -> Pair(
                R.string.data_dialog_clear_txt_title,
                R.string.data_dialog_clear_txt_message
            )
            DestructiveAction.ClearAllData -> Pair(
                R.string.data_dialog_clear_all_data_title,
                R.string.data_dialog_clear_all_data_message
            )
        }

        AlertDialog(
            onDismissRequest = { pendingAction = null },
            title = { Text(stringResource(titleRes)) },
            text = { Text(stringResource(messageRes)) },
            confirmButton = {
                TextButton(
                    onClick = {
                        when (currentAction) {
                            DestructiveAction.ClearTxt -> onClearTxt()
                            DestructiveAction.ClearAllData -> onClearData()
                        }
                        pendingAction = null
                    },
                    colors = ButtonDefaults.textButtonColors(
                        contentColor = MaterialTheme.colorScheme.error
                    )
                ) {
                    Text(stringResource(R.string.data_action_confirm))
                }
            },
            dismissButton = {
                TextButton(onClick = { pendingAction = null }) {
                    Text(stringResource(R.string.data_action_cancel))
                }
            }
        )
    }
}
