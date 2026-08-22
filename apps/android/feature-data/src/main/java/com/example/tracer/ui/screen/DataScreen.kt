package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.clickable
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.BorderStroke
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowForward
import androidx.compose.material.icons.filled.Archive
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.ExpandLess
import androidx.compose.material.icons.filled.ExpandMore
import androidx.compose.material.icons.filled.FolderOpen
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.LinearProgressIndicator
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.Alignment
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.data.R

private enum class DestructiveAction {
    ClearTxt,
    ClearDatabase,
    RebuildDatabase,
    ClearAllData
}

@Composable
fun DataManagementSection(
    modifier: Modifier = Modifier,
    onImportDataFolder: () -> Unit,
    onImportSingleTracer: () -> Unit,
    canExportAllMonthsTracer: Boolean,
    canExportCurrentTxtTracer: Boolean,
    onExportAllMonthsTracer: () -> Unit,
    onExportCurrentTxtTracer: () -> Unit,
    isTracerExportInProgress: Boolean,
    selectedTracerSecurityLevel: TracerExchangeSecurityLevel,
    onTracerSecurityLevelChange: (TracerExchangeSecurityLevel) -> Unit,
    showCryptoProgress: Boolean,
    cryptoProgressTitle: String,
    cryptoProgressPhase: String,
    cryptoOverallProgress: Float,
    cryptoOverallText: String,
    cryptoDetailsText: String,
    cryptoAdvancedDetailsText: String,
    expanded: Boolean = true,
    onToggleExpanded: () -> Unit = {},
    onClearTxt: () -> Unit,
    onClearDatabase: () -> Unit,
    onRebuildDatabase: () -> Unit,
    onClearData: () -> Unit
) {
    var pendingAction by remember { mutableStateOf<DestructiveAction?>(null) }
    var showAdvancedCryptoDetails by remember { mutableStateOf(false) }
    var isSecurityMenuExpanded by remember { mutableStateOf(false) }
    var isArchiveExportExpanded by rememberSaveable { mutableStateOf(false) }
    var isDangerZoneExpanded by rememberSaveable { mutableStateOf(false) }

    LaunchedEffect(showCryptoProgress) {
        if (!showCryptoProgress) {
            showAdvancedCryptoDetails = false
        }
    }

    ElevatedCard(modifier = modifier.fillMaxWidth()) {
        Column(
            modifier = Modifier.padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = androidx.compose.ui.Alignment.CenterVertically
            ) {
                Text(
                    text = stringResource(R.string.data_title_data_management),
                    style = MaterialTheme.typography.titleLarge,
                    color = MaterialTheme.colorScheme.onSurface
                )
                IconButton(onClick = onToggleExpanded) {
                    Icon(
                        imageVector = if (expanded) Icons.Default.ExpandLess else Icons.Default.ExpandMore,
                        contentDescription = stringResource(
                            if (expanded) R.string.data_cd_collapse_card else R.string.data_cd_expand_card
                        )
                    )
                }
            }
            if (expanded) {
                HorizontalDivider()
                DataSectionTitle(R.string.data_title_ingestion)
                DataActionRow(
                    title = stringResource(R.string.data_action_import_data_folder),
                    subtitle = stringResource(R.string.data_description_import_data_folder),
                    icon = Icons.Filled.FolderOpen,
                    onClick = onImportDataFolder
                )
                DataActionRow(
                    title = stringResource(R.string.data_action_import_single_tracer),
                    subtitle = stringResource(R.string.data_description_import_single_tracer),
                    icon = Icons.Filled.Refresh,
                    onClick = onImportSingleTracer
                )

                HorizontalDivider()
                DataSectionTitle(R.string.data_title_export)
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

                    DataActionRow(
                        title = stringResource(R.string.data_action_export_current_txt_tracer),
                        subtitle = stringResource(R.string.data_description_export_current_txt_tracer),
                        icon = Icons.Filled.FolderOpen,
                        onClick = onExportCurrentTxtTracer,
                        enabled = canExportCurrentTxtTracer && !isTracerExportInProgress,
                        trailingText = if (isTracerExportInProgress) {
                            stringResource(R.string.data_action_exporting)
                        } else {
                            null
                        }
                    )
                    DataDisclosureRow(
                        title = stringResource(R.string.data_title_compressed_archive),
                        subtitle = stringResource(
                            R.string.data_summary_compressed_archive,
                            tracerSecurityLevelLabel(selectedTracerSecurityLevel)
                        ),
                        icon = Icons.Filled.Archive,
                        expanded = isArchiveExportExpanded,
                        onClick = { isArchiveExportExpanded = !isArchiveExportExpanded }
                    )
                    if (isArchiveExportExpanded) {
                        Box(modifier = Modifier.fillMaxWidth()) {
                            DataActionRow(
                                title = stringResource(R.string.data_label_tracer_security_level),
                                subtitle = tracerSecurityLevelLabel(selectedTracerSecurityLevel),
                                icon = Icons.Filled.Refresh,
                                onClick = { isSecurityMenuExpanded = true },
                                enabled = !isTracerExportInProgress
                            )
                            DropdownMenu(
                                expanded = isSecurityMenuExpanded,
                                onDismissRequest = { isSecurityMenuExpanded = false }
                            ) {
                                val levelOptions = listOf(
                                    TracerExchangeSecurityLevel.INTERACTIVE,
                                    TracerExchangeSecurityLevel.MODERATE,
                                    TracerExchangeSecurityLevel.HIGH
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
                        DataActionRow(
                            title = stringResource(R.string.data_action_create_compressed_archive),
                            subtitle = stringResource(R.string.data_description_export_all_months_tracer),
                            icon = Icons.Filled.Refresh,
                            onClick = onExportAllMonthsTracer,
                            enabled = canExportAllMonthsTracer && !isTracerExportInProgress,
                            trailingText = if (isTracerExportInProgress) {
                                stringResource(R.string.data_action_exporting)
                            } else {
                                null
                            },
                            emphasized = true
                        )
                    }
                HorizontalDivider()
                DataSectionTitle(R.string.data_title_maintenance_debug)
                DataActionRow(
                    title = stringResource(R.string.data_action_rebuild_database),
                    subtitle = stringResource(R.string.data_description_rebuild_database),
                    icon = Icons.Filled.Refresh,
                    onClick = { pendingAction = DestructiveAction.RebuildDatabase }
                )
                DataDisclosureRow(
                    title = stringResource(R.string.data_title_danger_zone),
                    subtitle = null,
                    icon = Icons.Filled.Delete,
                    expanded = isDangerZoneExpanded,
                    onClick = { isDangerZoneExpanded = !isDangerZoneExpanded },
                    destructive = true
                )
                if (isDangerZoneExpanded) {
                    Text(
                        text = stringResource(R.string.data_description_danger_zone),
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                    DataActionRow(
                        title = stringResource(R.string.data_action_clear_txt),
                        icon = Icons.Filled.Delete,
                        onClick = { pendingAction = DestructiveAction.ClearTxt },
                        destructive = true
                    )
                    DataActionRow(
                        title = stringResource(R.string.data_action_clear_database),
                        icon = Icons.Filled.Delete,
                        onClick = { pendingAction = DestructiveAction.ClearDatabase },
                        destructive = true
                    )
                    DataActionRow(
                        title = stringResource(R.string.data_action_clear_all_app_data),
                        subtitle = stringResource(R.string.data_description_clear_all_app_data),
                        icon = Icons.Filled.Delete,
                        onClick = { pendingAction = DestructiveAction.ClearAllData },
                        destructive = true,
                        emphasizedDestructive = true
                    )
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
            DestructiveAction.ClearDatabase -> Pair(
                R.string.data_dialog_clear_database_title,
                R.string.data_dialog_clear_database_message
            )
            DestructiveAction.RebuildDatabase -> Pair(
                R.string.data_dialog_rebuild_database_title,
                R.string.data_dialog_rebuild_database_message
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
                            DestructiveAction.ClearDatabase -> onClearDatabase()
                            DestructiveAction.RebuildDatabase -> onRebuildDatabase()
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
private fun DataSectionTitle(@androidx.annotation.StringRes titleRes: Int) {
    Text(
        text = stringResource(titleRes),
        style = MaterialTheme.typography.titleMedium,
        color = MaterialTheme.colorScheme.onSurface
    )
}

@Composable
private fun DataActionRow(
    title: String,
    icon: ImageVector,
    onClick: () -> Unit,
    subtitle: String? = null,
    trailingText: String? = null,
    enabled: Boolean = true,
    destructive: Boolean = false,
    emphasized: Boolean = false,
    emphasizedDestructive: Boolean = false
) {
    val shape = RoundedCornerShape(16.dp)
    val contentColor = when {
        emphasizedDestructive -> MaterialTheme.colorScheme.onError
        emphasized -> MaterialTheme.colorScheme.onPrimary
        destructive -> MaterialTheme.colorScheme.error
        else -> MaterialTheme.colorScheme.onSurface
    }
    val containerColor = when {
        emphasizedDestructive -> MaterialTheme.colorScheme.error
        emphasized -> MaterialTheme.colorScheme.primary
        else -> MaterialTheme.colorScheme.surface
    }
    val outlineColor = when {
        emphasizedDestructive -> MaterialTheme.colorScheme.error
        emphasized -> MaterialTheme.colorScheme.primary
        destructive -> MaterialTheme.colorScheme.error.copy(alpha = 0.45f)
        else -> MaterialTheme.colorScheme.outlineVariant
    }
    Surface(
        modifier = Modifier
            .fillMaxWidth()
            .clickable(enabled = enabled, onClick = onClick),
        shape = shape,
        color = containerColor,
        contentColor = if (enabled) contentColor else MaterialTheme.colorScheme.onSurface.copy(alpha = 0.38f),
        border = BorderStroke(1.dp, outlineColor.copy(alpha = if (enabled) 1f else 0.38f))
    ) {
        Row(
            modifier = Modifier.padding(horizontal = 16.dp, vertical = 14.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(14.dp)
        ) {
            Icon(imageVector = icon, contentDescription = null)
            Column(modifier = Modifier.weight(1f), verticalArrangement = Arrangement.spacedBy(2.dp)) {
                Text(text = title, style = MaterialTheme.typography.bodyLarge)
                if (subtitle != null) {
                    Text(
                        text = subtitle,
                        style = MaterialTheme.typography.bodySmall,
                        color = if (emphasizedDestructive || emphasized) {
                            (if (emphasizedDestructive) MaterialTheme.colorScheme.onError
                            else MaterialTheme.colorScheme.onPrimary).copy(alpha = 0.82f)
                        } else {
                            MaterialTheme.colorScheme.onSurfaceVariant
                        }
                    )
                }
            }
            if (trailingText != null) {
                Text(text = trailingText, style = MaterialTheme.typography.labelMedium)
            } else {
                Icon(imageVector = Icons.AutoMirrored.Filled.ArrowForward, contentDescription = null)
            }
        }
    }
}

@Composable
private fun DataDisclosureRow(
    title: String,
    subtitle: String?,
    icon: ImageVector,
    expanded: Boolean,
    onClick: () -> Unit,
    destructive: Boolean = false
) {
    val shape = RoundedCornerShape(16.dp)
    val contentColor = if (destructive) MaterialTheme.colorScheme.error else MaterialTheme.colorScheme.onSurface
    val containerColor = if (destructive) {
        MaterialTheme.colorScheme.errorContainer.copy(alpha = 0.32f)
    } else {
        MaterialTheme.colorScheme.surface
    }
    val outlineColor = if (destructive) {
        MaterialTheme.colorScheme.error.copy(alpha = 0.38f)
    } else {
        MaterialTheme.colorScheme.outlineVariant
    }
    Surface(
        modifier = Modifier
            .fillMaxWidth()
            .clickable(onClick = onClick),
        shape = shape,
        color = containerColor,
        contentColor = contentColor,
        border = BorderStroke(1.dp, outlineColor)
    ) {
        Row(
            modifier = Modifier.padding(horizontal = 16.dp, vertical = 14.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Icon(imageVector = icon, contentDescription = null)
            Spacer(modifier = Modifier.width(14.dp))
            Column(modifier = Modifier.weight(1f), verticalArrangement = Arrangement.spacedBy(2.dp)) {
                Text(text = title, style = MaterialTheme.typography.bodyLarge)
                if (subtitle != null) {
                    Text(
                        text = subtitle,
                        style = MaterialTheme.typography.bodySmall,
                        color = if (destructive) {
                            MaterialTheme.colorScheme.error.copy(alpha = 0.82f)
                        } else {
                            MaterialTheme.colorScheme.onSurfaceVariant
                        }
                    )
                }
            }
            Icon(
                imageVector = if (expanded) Icons.Filled.ExpandLess else Icons.Filled.ExpandMore,
                contentDescription = null
            )
        }
    }
}

@Composable
private fun tracerSecurityLevelLabel(level: TracerExchangeSecurityLevel): String =
    when (level) {
        TracerExchangeSecurityLevel.INTERACTIVE -> stringResource(
            R.string.data_security_level_interactive
        )
        TracerExchangeSecurityLevel.MODERATE -> stringResource(
            R.string.data_security_level_moderate
        )
        TracerExchangeSecurityLevel.HIGH -> stringResource(
            R.string.data_security_level_high
        )
    }
