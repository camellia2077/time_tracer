package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
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
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.Icon
import androidx.compose.material3.LinearProgressIndicator
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
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
    onIngestFull: () -> Unit,
    onImportFolderTracer: () -> Unit,
    onImportSingleTxt: () -> Unit,
    onImportSingleTracer: () -> Unit,
    canExportAllMonthsTxt: Boolean,
    onExportAllMonthsTxt: () -> Unit,
    isTxtExportInProgress: Boolean,
    canExportAllMonthsTracer: Boolean,
    onExportAllMonthsTracer: () -> Unit,
    isTracerExportInProgress: Boolean,
    selectedTracerSecurityLevel: FileCryptoSecurityLevel,
    onTracerSecurityLevelChange: (FileCryptoSecurityLevel) -> Unit,
    showCryptoProgress: Boolean,
    cryptoProgressTitle: String,
    cryptoProgressPhase: String,
    cryptoOverallProgress: Float,
    cryptoOverallText: String,
    cryptoCurrentProgress: Float,
    cryptoCurrentText: String,
    cryptoDetailsText: String,
    cryptoAdvancedDetailsText: String,
    onClearTxt: () -> Unit,
    onClearData: () -> Unit
) {
    var pendingAction by remember { mutableStateOf<DestructiveAction?>(null) }
    var showAdvancedCryptoDetails by remember { mutableStateOf(false) }
    var isSecurityMenuExpanded by remember { mutableStateOf(false) }

    LaunchedEffect(showCryptoProgress) {
        if (!showCryptoProgress) {
            showAdvancedCryptoDetails = false
        }
    }

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
                        onClick = onIngestFull,
                        modifier = Modifier.fillMaxWidth()
                    ) {
                        Icon(Icons.Filled.Refresh, contentDescription = null)
                        Spacer(modifier = Modifier.width(8.dp))
                        Text(stringResource(R.string.data_action_ingest_full))
                    }

                    Button(
                        onClick = onImportFolderTracer,
                        modifier = Modifier.fillMaxWidth()
                    ) {
                        Icon(Icons.Filled.Refresh, contentDescription = null)
                        Spacer(modifier = Modifier.width(8.dp))
                        Text(stringResource(R.string.data_action_import_folder_tracer))
                    }

                    Button(
                        onClick = onImportSingleTxt,
                        modifier = Modifier.fillMaxWidth()
                    ) {
                        Icon(Icons.Filled.Refresh, contentDescription = null)
                        Spacer(modifier = Modifier.width(8.dp))
                        Text(stringResource(R.string.data_action_import_single_txt))
                    }

                    Button(
                        onClick = onImportSingleTracer,
                        modifier = Modifier.fillMaxWidth()
                    ) {
                        Icon(Icons.Filled.Refresh, contentDescription = null)
                        Spacer(modifier = Modifier.width(8.dp))
                        Text(stringResource(R.string.data_action_import_single_tracer))
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
                    if (showCryptoProgress) {
                        Text(
                            text = cryptoProgressTitle,
                            style = MaterialTheme.typography.bodyMedium,
                            color = MaterialTheme.colorScheme.onSurface
                        )
                        Text(
                            text = cryptoProgressPhase,
                            style = MaterialTheme.typography.bodySmall,
                            color = MaterialTheme.colorScheme.primary
                        )
                        Text(
                            text = stringResource(R.string.data_progress_overall),
                            style = MaterialTheme.typography.bodySmall
                        )
                        LinearProgressIndicator(
                            progress = { cryptoOverallProgress.coerceIn(0f, 1f) },
                            modifier = Modifier.fillMaxWidth()
                        )
                        Text(
                            text = cryptoOverallText,
                            style = MaterialTheme.typography.bodySmall
                        )
                        Text(
                            text = stringResource(R.string.data_progress_current_file),
                            style = MaterialTheme.typography.bodySmall
                        )
                        LinearProgressIndicator(
                            progress = { cryptoCurrentProgress.coerceIn(0f, 1f) },
                            modifier = Modifier.fillMaxWidth()
                        )
                        Text(
                            text = cryptoCurrentText,
                            style = MaterialTheme.typography.bodySmall
                        )
                        if (cryptoDetailsText.isNotBlank()) {
                            Text(
                                text = cryptoDetailsText,
                                style = MaterialTheme.typography.bodySmall
                            )
                        }
                        if (cryptoAdvancedDetailsText.isNotBlank()) {
                            TextButton(
                                onClick = {
                                    showAdvancedCryptoDetails = !showAdvancedCryptoDetails
                                }
                            ) {
                                Text(
                                    text = if (showAdvancedCryptoDetails) {
                                        stringResource(R.string.data_action_hide_progress_details)
                                    } else {
                                        stringResource(R.string.data_action_show_progress_details)
                                    }
                                )
                            }
                            if (showAdvancedCryptoDetails) {
                                Text(
                                    text = cryptoAdvancedDetailsText,
                                    style = MaterialTheme.typography.bodySmall
                                )
                            }
                        }
                    }

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

                    Text(
                        text = stringResource(R.string.data_label_tracer_security_level),
                        style = MaterialTheme.typography.bodySmall
                    )
                    Box(modifier = Modifier.fillMaxWidth()) {
                        OutlinedButton(
                            onClick = { isSecurityMenuExpanded = true },
                            enabled = !isTracerExportInProgress,
                            modifier = Modifier.fillMaxWidth()
                        ) {
                            Text(
                                text = "${tracerSecurityLevelLabel(selectedTracerSecurityLevel)} ▼"
                            )
                        }
                        DropdownMenu(
                            expanded = isSecurityMenuExpanded,
                            onDismissRequest = { isSecurityMenuExpanded = false }
                        ) {
                            val levelOptions = listOf(
                                FileCryptoSecurityLevel.INTERACTIVE,
                                FileCryptoSecurityLevel.MODERATE,
                                FileCryptoSecurityLevel.HIGH
                            )
                            for (levelOption in levelOptions) {
                                DropdownMenuItem(
                                    text = { Text(text = tracerSecurityLevelLabel(levelOption)) },
                                    onClick = {
                                        onTracerSecurityLevelChange(levelOption)
                                        isSecurityMenuExpanded = false
                                    }
                                )
                            }
                        }
                    }

                    Button(
                        onClick = onExportAllMonthsTracer,
                        enabled = canExportAllMonthsTracer && !isTracerExportInProgress,
                        modifier = Modifier.fillMaxWidth()
                    ) {
                        Text(
                            if (isTracerExportInProgress) {
                                stringResource(R.string.data_action_exporting)
                            } else {
                                stringResource(R.string.data_action_export_all_months_tracer)
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

@Composable
private fun tracerSecurityLevelLabel(level: FileCryptoSecurityLevel): String =
    when (level) {
        FileCryptoSecurityLevel.INTERACTIVE -> stringResource(
            R.string.data_security_level_interactive
        )
        FileCryptoSecurityLevel.MODERATE -> stringResource(
            R.string.data_security_level_moderate
        )
        FileCryptoSecurityLevel.HIGH -> stringResource(
            R.string.data_security_level_high
        )
    }
