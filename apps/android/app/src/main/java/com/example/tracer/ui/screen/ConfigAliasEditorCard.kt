package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.BorderStroke
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.FilledTonalButton
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
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
import com.example.tracer.ui.components.NativeMultilineTextEditor
import com.example.tracer.ui.components.TracerSegmentedButtonDefaults
import kotlinx.coroutines.delay

@Composable
internal fun ConfigAliasEditorCard(
    selectedFileDisplayName: String,
    selectedFileContent: String,
    mode: AliasEditorMode,
    document: AliasTomlDocument?,
    movePlan: AliasEntryMovePlan?,
    parentOptions: List<String>,
    advancedTomlDraft: String,
    errorMessage: String,
    onCreateAliasTomlFile: (String) -> Unit,
    onDeleteAliasTomlFile: () -> Unit,
    onSelectStructuredMode: () -> Unit,
    onSelectAdvancedMode: () -> Unit,
    onParentChange: (String) -> Unit,
    onAdvancedTomlChange: (String) -> Unit,
    onAddGroup: (parentGroupId: String?, name: String) -> Unit,
    onDeleteGroup: (groupId: String) -> Unit,
    onRenameGroup: (groupId: String, name: String) -> Unit,
    onAddEntry: (parentGroupId: String?, canonicalLeaf: String, aliases: List<String>) -> Unit,
    onUpdateEntry: (entryId: String, canonicalLeaf: String, aliases: List<String>) -> Unit,
    onPromoteEntry: (entryId: String) -> Unit,
    onRenameGroupAlias: (groupId: String, oldAlias: String, newAlias: String) -> Unit,
    onAddGroupAlias: (groupId: String, alias: String) -> Unit,
    onUpdateGroupAliases: (groupId: String, aliases: List<String>) -> Unit,
    onDeleteEntry: (entryId: String) -> Unit,
    onPreviewEntryMove: (entryId: String, targetGroupId: String) -> Unit,
    onConfirmMovePlan: () -> Unit,
    onDiscardMovePlan: () -> Unit,
    onSave: () -> Unit
) {
    var dialogState by remember { mutableStateOf<AliasEditorDialogState?>(null) }
    var showDeleteAliasTomlDialog by remember { mutableStateOf(false) }
    var currentPathGroupIds by remember(selectedFileDisplayName) {
        mutableStateOf(emptyList<String>())
    }
    LaunchedEffect(mode, selectedFileContent, advancedTomlDraft) {
        val currentDraft = advancedTomlDraft
        if (currentDraft.isBlank() || currentDraft == selectedFileContent) {
            return@LaunchedEffect
        }
        delay(CONFIG_ALIAS_EDITOR_AUTO_SAVE_DELAY_MS)
        val latestDraft = advancedTomlDraft
        if (latestDraft.isNotBlank() && latestDraft != selectedFileContent) {
            onSave()
        }
    }

    ElevatedCard(modifier = Modifier.fillMaxWidth()) {
        Column(
            modifier = Modifier.padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Text(
                text = stringResource(R.string.config_title_editor_file, selectedFileDisplayName),
                style = MaterialTheme.typography.headlineSmall,
                color = MaterialTheme.colorScheme.onSurface
            )

            if (mode == AliasEditorMode.STRUCTURED && document != null) {
                AliasParentSelector(
                    parent = document.parent,
                    parentOptions = parentOptions,
                    onParentChange = onParentChange
                )
            }

            ConfigEditorFileControls(
                onCreateAliasTomlFile = onCreateAliasTomlFile
            )

            OutlinedButton(
                onClick = { showDeleteAliasTomlDialog = true },
                modifier = Modifier.fillMaxWidth(),
                colors = ButtonDefaults.outlinedButtonColors(
                    contentColor = MaterialTheme.colorScheme.error
                )
            ) {
                Icon(Icons.Filled.Delete, contentDescription = null)
                Spacer(modifier = Modifier.width(8.dp))
                Text(stringResource(R.string.config_action_delete_alias_mapping))
            }

            SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
                val modes = listOf(
                    AliasEditorMode.STRUCTURED to stringResource(R.string.config_alias_mode_structured),
                    AliasEditorMode.ADVANCED to stringResource(R.string.config_alias_mode_advanced)
                )
                modes.forEachIndexed { index, (candidateMode, label) ->
                    val selected = mode == candidateMode
                    SegmentedButton(
                        shape = SegmentedButtonDefaults.itemShape(index = index, count = modes.size),
                        onClick = {
                            when (candidateMode) {
                                AliasEditorMode.STRUCTURED -> onSelectStructuredMode()
                                AliasEditorMode.ADVANCED -> onSelectAdvancedMode()
                            }
                        },
                        selected = selected,
                        modifier = Modifier.weight(1f),
                        colors = TracerSegmentedButtonDefaults.colors(),
                        label = { Text(label) }
                    )
                }
            }

            if (errorMessage.isNotBlank()) {
                Text(
                    text = errorMessage,
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.error
                )
            }

            if (movePlan != null) {
                AliasEntryMovePlanPreview(
                    plan = movePlan,
                    onConfirm = onConfirmMovePlan,
                    onDiscard = onDiscardMovePlan
                )
            }

            when (mode) {
                AliasEditorMode.STRUCTURED -> {
                    if (document == null) {
                        Text(
                            text = stringResource(R.string.config_alias_structured_unavailable),
                            style = MaterialTheme.typography.bodyMedium
                        )
                    } else {
                        val layer = resolveAliasStructuredLayer(
                            document = document,
                            pathGroupIds = currentPathGroupIds
                        )
                        LaunchedEffect(layer.normalizedPathGroupIds) {
                            // Keep local navigation path self-healing after tree mutations
                            // (for example group delete/rename) by snapping to the nearest
                            // still-valid ancestor path produced by layer resolution.
                            if (layer.normalizedPathGroupIds != currentPathGroupIds) {
                                currentPathGroupIds = layer.normalizedPathGroupIds
                            }
                        }
                        AliasStructuredEditorContent(
                            document = document,
                            layer = layer,
                            onNavigateToBreadcrumb = { groupId ->
                                currentPathGroupIds = if (groupId == null) {
                                    emptyList()
                                } else {
                                    val index = layer.breadcrumbs.indexOfFirst { it.groupId == groupId }
                                    if (index >= 0) {
                                        layer.normalizedPathGroupIds.take(index + 1)
                                    } else {
                                        currentPathGroupIds
                                    }
                                }
                            },
                            onNavigateToGroup = { groupId ->
                                currentPathGroupIds = layer.normalizedPathGroupIds + groupId
                            },
                            onRequestAddCurrentGroup = {
                                dialogState = AliasEditorDialogState.AddGroup(
                                    parentGroupId = layer.currentParentGroupId
                                )
                            },
                            onRequestAddCurrentEntry = {
                                dialogState = AliasEditorDialogState.AddEntry(
                                    parentGroupId = layer.currentParentGroupId
                                )
                            },
                            onRequestEditGroup = { group ->
                                dialogState = AliasEditorDialogState.GroupActions(group)
                            },
                            onRequestEditEntry = { entry ->
                                dialogState = AliasEditorDialogState.EntryActions(entry)
                            }
                        )
                    }
                }

                AliasEditorMode.ADVANCED -> {
                    Text(
                        text = stringResource(R.string.config_label_toml_content),
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                    NativeMultilineTextEditor(
                        value = advancedTomlDraft,
                        onValueChange = onAdvancedTomlChange,
                        modifier = Modifier.fillMaxWidth(),
                        minLines = 12,
                        monospace = true
                    )
                }
            }

        }
    }

    when (val activeDialog = dialogState) {
        is AliasEditorDialogState.AddGroup -> {
            AliasGroupNameDialog(
                title = stringResource(R.string.config_alias_dialog_add_group_title),
                initialName = "",
                onDismiss = { dialogState = null },
                onConfirm = { name ->
                    dialogState = null
                    onAddGroup(activeDialog.parentGroupId, name)
                }
            )
        }

        is AliasEditorDialogState.AddEntry -> {
            AliasEntryDialog(
                title = stringResource(R.string.config_alias_dialog_add_entry_title),
                initialCanonicalLeaf = "",
                initialAliases = listOf(""),
                onDismiss = { dialogState = null },
                onConfirm = { canonicalLeaf, aliases ->
                    dialogState = null
                    onAddEntry(activeDialog.parentGroupId, canonicalLeaf, aliases)
                }
            )
        }

        is AliasEditorDialogState.EditEntryAliases -> {
            AliasManagementDialog(
                title = stringResource(R.string.config_alias_dialog_edit_aliases_title),
                aliases = activeDialog.entry.aliases,
                minimumAliases = 1,
                onDismiss = { dialogState = null },
                onConfirm = { aliases ->
                    dialogState = null
                    onUpdateEntry(activeDialog.entry.id, activeDialog.entry.canonicalLeaf, aliases)
                }
            )
        }

        is AliasEditorDialogState.EditGroupName -> {
            AliasGroupNameDialog(
                title = stringResource(R.string.config_alias_dialog_edit_group_name_title),
                initialName = activeDialog.group.name,
                onDismiss = { dialogState = null },
                onConfirm = { name ->
                    dialogState = null
                    onRenameGroup(activeDialog.group.id, name)
                }
            )
        }

        is AliasEditorDialogState.EditEntryName -> {
            AliasEntryDialog(
                title = stringResource(R.string.config_alias_dialog_edit_entry_name_title),
                initialCanonicalLeaf = activeDialog.entry.canonicalLeaf,
                initialAliases = activeDialog.entry.aliases,
                onDismiss = { dialogState = null },
                onConfirm = { canonicalLeaf, aliases ->
                    dialogState = null
                    onUpdateEntry(activeDialog.entry.id, canonicalLeaf, aliases)
                }
            )
        }

        is AliasEditorDialogState.GroupActions -> {
            AliasGroupActionsDialog(
                group = activeDialog.group,
                onDismiss = { dialogState = null },
                onEditName = {
                    dialogState = AliasEditorDialogState.EditGroupName(activeDialog.group)
                },
                onEditAlias = {
                    dialogState = AliasEditorDialogState.EditGroupAliases(activeDialog.group)
                },
                onAddAlias = { dialogState = AliasEditorDialogState.AddGroupAlias(activeDialog.group.id) },
                onDelete = { dialogState = AliasEditorDialogState.ConfirmDeleteGroup(activeDialog.group) }
            )
        }

        is AliasEditorDialogState.EntryActions -> {
            AliasEntryActionsDialog(
                entry = activeDialog.entry,
                onDismiss = { dialogState = null },
                onEditName = {
                    dialogState = AliasEditorDialogState.EditEntryName(activeDialog.entry)
                },
                onEditAlias = {
                    dialogState = AliasEditorDialogState.EditEntryAliases(activeDialog.entry)
                },
                onPromote = { dialogState = AliasEditorDialogState.ConfirmPromote(activeDialog.entry) },
                onMove = { dialogState = AliasEditorDialogState.PlanEntryMove(activeDialog.entry) },
                onDelete = { dialogState = AliasEditorDialogState.ConfirmDeleteEntry(activeDialog.entry) }
            )
        }

        is AliasEditorDialogState.ConfirmDeleteGroup -> {
            AliasDeleteConfirmDialog(
                title = stringResource(R.string.config_alias_delete_group_title),
                message = stringResource(R.string.config_alias_delete_group_message, activeDialog.group.name),
                onDismiss = { dialogState = null },
                onConfirm = {
                    dialogState = null
                    onDeleteGroup(activeDialog.group.id)
                }
            )
        }

        is AliasEditorDialogState.ConfirmDeleteEntry -> {
            AliasDeleteConfirmDialog(
                title = stringResource(R.string.config_alias_delete_entry_title),
                message = stringResource(R.string.config_alias_delete_entry_message, activeDialog.entry.canonicalLeaf),
                onDismiss = { dialogState = null },
                onConfirm = {
                    dialogState = null
                    onDeleteEntry(activeDialog.entry.id)
                }
            )
        }

        is AliasEditorDialogState.PlanEntryMove -> {
            AliasEntryMoveTargetDialog(
                entry = activeDialog.entry,
                destinations = document?.moveDestinationsForEntry(activeDialog.entry.id).orEmpty(),
                onDismiss = { dialogState = null },
                onConfirm = { targetGroupId ->
                    dialogState = null
                    onPreviewEntryMove(activeDialog.entry.id, targetGroupId)
                }
            )
        }

        is AliasEditorDialogState.ConfirmPromote -> {
            AliasPromoteConfirmDialog(
                entry = activeDialog.entry,
                onDismiss = { dialogState = null },
                onConfirm = {
                    dialogState = null
                    onPromoteEntry(activeDialog.entry.id)
                }
            )
        }

        is AliasEditorDialogState.EditGroupAliases -> {
            AliasManagementDialog(
                title = stringResource(R.string.config_alias_dialog_edit_aliases_title),
                aliases = activeDialog.group.groupAliases,
                minimumAliases = 0,
                onDismiss = { dialogState = null },
                onConfirm = { aliases ->
                    dialogState = null
                    onUpdateGroupAliases(activeDialog.group.id, aliases)
                }
            )
        }

        is AliasEditorDialogState.AddGroupAlias -> {
            AliasGroupAliasDialog(
                title = stringResource(R.string.config_alias_dialog_add_alias_title),
                initialAlias = "",
                onDismiss = { dialogState = null },
                onConfirm = { alias ->
                    dialogState = null
                    onAddGroupAlias(activeDialog.groupId, alias)
                }
            )
        }

        null -> Unit
    }

    if (showDeleteAliasTomlDialog) {
        ConfigAliasTomlDeleteDialog(
            fileName = selectedFileDisplayName,
            onDismiss = { showDeleteAliasTomlDialog = false },
            onConfirm = {
                showDeleteAliasTomlDialog = false
                onDeleteAliasTomlFile()
            }
        )
    }
}

private const val CONFIG_ALIAS_EDITOR_AUTO_SAVE_DELAY_MS = 600L

@Composable
private fun AliasEntryMovePlanPreview(
    plan: AliasEntryMovePlan,
    onConfirm: () -> Unit,
    onDiscard: () -> Unit
) {
    AlertDialog(
        onDismissRequest = onDiscard,
        title = {
            Text(
                text = stringResource(R.string.config_alias_move_plan_title),
                style = MaterialTheme.typography.titleSmall
            )
        },
        text = {
            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                Text(
                    text = stringResource(
                        R.string.config_alias_move_plan_canonical,
                        plan.oldCanonical,
                        plan.newCanonical
                    )
                )
                Text(
                    text = stringResource(R.string.config_alias_move_plan_impact),
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
        },
        confirmButton = {
            TextButton(onClick = onConfirm) {
                Text(stringResource(R.string.config_alias_action_confirm_move_plan))
            }
        },
        dismissButton = {
            TextButton(onClick = onDiscard) {
                Text(stringResource(R.string.config_alias_action_discard_move_plan))
            }
        }
    )
}

@Composable
private fun AliasStructuredEditorContent(
    document: AliasTomlDocument,
    layer: AliasStructuredLayer,
    onNavigateToBreadcrumb: (String?) -> Unit,
    onNavigateToGroup: (String) -> Unit,
    onRequestAddCurrentGroup: () -> Unit,
    onRequestAddCurrentEntry: () -> Unit,
    onRequestEditGroup: (AliasTomlGroup) -> Unit,
    onRequestEditEntry: (AliasTomlEntry) -> Unit
) {
    AliasPathBar(
        breadcrumbs = layer.breadcrumbs,
        onNavigateToBreadcrumb = onNavigateToBreadcrumb
    )

    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        FilledTonalButton(
            onClick = onRequestAddCurrentGroup,
            modifier = Modifier.weight(1f),
            colors = ButtonDefaults.filledTonalButtonColors(
                containerColor = MaterialTheme.colorScheme.primaryContainer,
                contentColor = MaterialTheme.colorScheme.onPrimaryContainer
            ),
            border = BorderStroke(1.dp, MaterialTheme.colorScheme.primary)
        ) {
            Icon(Icons.Filled.Add, contentDescription = null)
            Spacer(modifier = Modifier.width(8.dp))
            Text(stringResource(R.string.config_alias_action_add_group))
        }
        FilledTonalButton(
            onClick = onRequestAddCurrentEntry,
            modifier = Modifier.weight(1f),
            colors = ButtonDefaults.filledTonalButtonColors(
                containerColor = MaterialTheme.colorScheme.primaryContainer,
                contentColor = MaterialTheme.colorScheme.onPrimaryContainer
            ),
            border = BorderStroke(1.dp, MaterialTheme.colorScheme.primary)
        ) {
            Icon(Icons.Filled.Add, contentDescription = null)
            Spacer(modifier = Modifier.width(8.dp))
            Text(stringResource(R.string.config_alias_action_add_alias))
        }
    }

    if (layer.currentGroups.isEmpty() && layer.currentEntries.isEmpty()) {
        AliasEmptyState()
    }

    for (group in layer.currentGroups) {
        AliasGroupRowCard(
            group = group,
            onEnterGroup = { onNavigateToGroup(group.id) },
            onEdit = { onRequestEditGroup(group) }
        )
    }

    for (entry in layer.currentEntries) {
        AliasEntryRow(
            entry = entry,
            modifier = Modifier.fillMaxWidth(),
            onEdit = { onRequestEditEntry(entry) }
        )
    }
}

internal data class AliasBreadcrumbSegment(
    val groupId: String,
    val name: String
)

internal data class AliasStructuredLayer(
    val normalizedPathGroupIds: List<String>,
    val breadcrumbs: List<AliasBreadcrumbSegment>,
    val currentNodes: List<AliasTomlNode>
) {
    val currentGroups: List<AliasTomlGroup>
        get() = currentNodes.filterIsInstance<AliasTomlGroup>()
    val currentEntries: List<AliasTomlEntry>
        get() = currentNodes.filterIsInstance<AliasTomlEntry>()
    val currentParentGroupId: String?
        get() = normalizedPathGroupIds.lastOrNull()
}

internal fun resolveAliasStructuredLayer(
    document: AliasTomlDocument,
    pathGroupIds: List<String>
): AliasStructuredLayer {
    // Drill-down design choice: render only one layer at a time and derive that
    // layer by walking the requested path until the first invalid segment.
    // This guarantees deterministic fallback to the nearest valid ancestor.
    val normalizedPath = mutableListOf<String>()
    val breadcrumbs = mutableListOf<AliasBreadcrumbSegment>()
    var currentNodes: List<AliasTomlNode> = document.nodes

    for (candidateId in pathGroupIds) {
        val nextGroup = currentNodes
            .filterIsInstance<AliasTomlGroup>()
            .firstOrNull { group -> group.id == candidateId }
            ?: break
        normalizedPath += nextGroup.id
        breadcrumbs += AliasBreadcrumbSegment(
            groupId = nextGroup.id,
            name = nextGroup.name
        )
        currentNodes = nextGroup.nodes
    }

    return AliasStructuredLayer(
        normalizedPathGroupIds = normalizedPath,
        breadcrumbs = breadcrumbs,
        currentNodes = currentNodes
    )
}

internal sealed interface AliasEditorDialogState {
    data class AddGroup(val parentGroupId: String?) : AliasEditorDialogState
    data class AddEntry(val parentGroupId: String?) : AliasEditorDialogState
    data class EditEntryAliases(val entry: AliasTomlEntry) : AliasEditorDialogState
    data class EditGroupName(val group: AliasTomlGroup) : AliasEditorDialogState
    data class EditEntryName(val entry: AliasTomlEntry) : AliasEditorDialogState
    data class GroupActions(val group: AliasTomlGroup) : AliasEditorDialogState
    data class EntryActions(val entry: AliasTomlEntry) : AliasEditorDialogState
    data class PlanEntryMove(val entry: AliasTomlEntry) : AliasEditorDialogState
    data class ConfirmPromote(val entry: AliasTomlEntry) : AliasEditorDialogState
    data class EditGroupAliases(val group: AliasTomlGroup) : AliasEditorDialogState
    data class AddGroupAlias(val groupId: String) : AliasEditorDialogState
    data class ConfirmDeleteGroup(val group: AliasTomlGroup) : AliasEditorDialogState
    data class ConfirmDeleteEntry(val entry: AliasTomlEntry) : AliasEditorDialogState
}
