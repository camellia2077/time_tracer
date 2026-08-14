package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
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
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.FilledTonalButton
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.Text
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
internal fun ActivityHierarchyEditorCard(
    aliasFiles: List<ConfigTomlFileEntry>,
    selectedFileDisplayName: String,
    selectedFileContent: String,
    mode: AliasEditorMode,
    document: ActivityHierarchyDocument?,
    movePlan: AliasEntryMovePlan?,
    moveDestinations: List<AliasEntryMoveDestinationDocument>,
    moveDestinationsLoading: Boolean,
    advancedTomlDraft: String,
    errorMessage: String,
    onCreateAliasTomlFile: (String) -> Unit,
    onSelectAliasFile: (String) -> Unit,
    onDeleteAliasTomlFile: () -> Unit,
    onRenameCategory: (String) -> Unit,
    onSetParentColor: (String) -> Unit,
    onSelectStructuredMode: () -> Unit,
    onSelectAdvancedMode: () -> Unit,
    onAdvancedTomlChange: (String) -> Unit,
    onAddGroup: (parentGroupId: String?, name: String) -> Unit,
    onDeleteGroup: (groupId: String) -> Unit,
    onRenameGroup: (groupId: String, name: String) -> Unit,
    onAddEntry: (parentGroupId: String?, canonicalLeaf: String, aliases: List<String>) -> Unit,
    onUpdateEntry: (entryId: String, canonicalLeaf: String, aliases: List<String>) -> Unit,
    onMergeEntry: (sourceEntryId: String, destinationEntryId: String) -> Unit,
    onPromoteEntry: (entryId: String) -> Unit,
    onRenameGroupAlias: (groupId: String, oldAlias: String, newAlias: String) -> Unit,
    onAddGroupAlias: (groupId: String, alias: String) -> Unit,
    onUpdateGroupAliases: (groupId: String, aliases: List<String>) -> Unit,
    onDeleteEntry: (entryId: String) -> Unit,
    onPrepareEntryMove: (entryId: String) -> Unit,
    onPrepareGroupMove: (groupId: String) -> Unit,
    onPreviewEntryMove: (entryId: String, target: AliasEntryMoveTarget) -> Unit,
    onPreviewGroupMove: (groupId: String, target: AliasEntryMoveTarget) -> Unit,
    onConfirmMovePlan: () -> Unit,
    onDiscardMovePlan: () -> Unit,
    onSave: () -> Unit
) {
    var dialogState by remember { mutableStateOf<AliasEditorDialogState?>(null) }
    var showDeleteAliasTomlDialog by remember { mutableStateOf(false) }
    var showRenameCategoryDialog by remember { mutableStateOf(false) }
    var showAliasFileMenu by remember { mutableStateOf(false) }
    var parentColorDraft by remember(document?.parent, document?.color) {
        mutableStateOf(document?.color.orEmpty())
    }
    val categoryName = document?.parent
        ?.takeIf { it.isNotBlank() }
        ?: selectedFileDisplayName.removeSuffix(".toml")
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

    Column(
        modifier = Modifier
            .fillMaxWidth()
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
            Column(
                modifier = Modifier.fillMaxWidth(),
                verticalArrangement = Arrangement.spacedBy(2.dp)
            ) {
                Text(
                    text = stringResource(R.string.config_title_editor_categories),
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.onSurface,
                    maxLines = 1
                )
                Text(
                    text = categoryName,
                    style = MaterialTheme.typography.headlineSmall,
                    color = MaterialTheme.colorScheme.primary,
                    maxLines = 1
                )
                Box(modifier = Modifier.fillMaxWidth()) {
                    OutlinedButton(
                        onClick = { showAliasFileMenu = true },
                        modifier = Modifier.fillMaxWidth()
                    ) {
                        Text(
                            text = selectedFileDisplayName,
                            maxLines = 1
                        )
                    }
                    DropdownMenu(
                        expanded = showAliasFileMenu,
                        onDismissRequest = { showAliasFileMenu = false }
                    ) {
                        aliasFiles.forEach { entry ->
                            DropdownMenuItem(
                                text = { Text(entry.displayName) },
                                onClick = {
                                    showAliasFileMenu = false
                                    onSelectAliasFile(entry.relativePath)
                                }
                            )
    }
}
            }

            ActivityHierarchyFileControls(
                onCreateAliasTomlFile = onCreateAliasTomlFile
            )

            Button(
                onClick = { showRenameCategoryDialog = true },
                modifier = Modifier.fillMaxWidth()
            ) {
                Text(stringResource(R.string.config_alias_action_rename_category))
            }

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

            ActivityHierarchyParentColorEditor(
                draftValue = parentColorDraft,
                persistedValue = document?.color.orEmpty(),
                onDraftValueChange = { parentColorDraft = it },
                onSaveColor = onSetParentColor
            )

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
                        text = stringResource(R.string.config_label_advanced_content),
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
                onMove = {
                    onPrepareGroupMove(activeDialog.group.id)
                    dialogState = AliasEditorDialogState.PlanGroupMove(activeDialog.group)
                },
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
                onMerge = {
                    dialogState = AliasEditorDialogState.MergeEntry(activeDialog.entry)
                },
                onPromote = { dialogState = AliasEditorDialogState.ConfirmPromote(activeDialog.entry) },
                onMove = {
                    onPrepareEntryMove(activeDialog.entry.id)
                    dialogState = AliasEditorDialogState.PlanEntryMove(activeDialog.entry)
                },
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
                destinations = moveDestinations,
                loading = moveDestinationsLoading,
                onDismiss = { dialogState = null },
                onConfirm = { target ->
                    dialogState = null
                    onPreviewEntryMove(activeDialog.entry.id, target)
                }
            )
        }

        is AliasEditorDialogState.PlanGroupMove -> {
            AliasGroupMoveTargetDialog(
                group = activeDialog.group,
                destinations = moveDestinations,
                loading = moveDestinationsLoading,
                onDismiss = { dialogState = null },
                onConfirm = { target ->
                    dialogState = null
                    onPreviewGroupMove(activeDialog.group.id, target)
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

        is AliasEditorDialogState.MergeEntry -> {
            AliasEntryMergeTargetDialog(
                source = activeDialog.entry,
                tomlDisplayName = selectedFileDisplayName,
                document = document,
                onDismiss = { dialogState = null },
                onConfirm = { targetId ->
                    dialogState = null
                    onMergeEntry(activeDialog.entry.id, targetId)
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
        ActivityHierarchyTomlDeleteDialog(
            fileName = selectedFileDisplayName,
            onDismiss = { showDeleteAliasTomlDialog = false },
            onConfirm = {
                showDeleteAliasTomlDialog = false
                onDeleteAliasTomlFile()
            }
        )
    }

    if (showRenameCategoryDialog) {
        AliasGroupNameDialog(
            title = stringResource(R.string.config_alias_dialog_rename_category_title),
            initialName = selectedFileDisplayName.removeSuffix(".toml"),
            onDismiss = { showRenameCategoryDialog = false },
            onConfirm = { name ->
                showRenameCategoryDialog = false
                onRenameCategory(name)
            }
        )
    }
}
