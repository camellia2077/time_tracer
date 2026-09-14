package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.ArrowDropDown
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.MoreVert
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp


@Composable
internal fun ActivityHierarchyEditorCard(
    aliasFiles: List<ConfigTomlFileEntry>,
    selectedFileDisplayName: String,
    document: ActivityHierarchyDocument?,
    movePlan: AliasEntryMovePlan?,
    moveDestinations: List<AliasEntryMoveDestinationDocument>,
    moveDestinationsLoading: Boolean,
    searchQuery: String,
    errorMessage: String,
    onCreateAliasTomlFile: (String) -> Unit,
    onSelectAliasFile: (String) -> Unit,
    onDeleteAliasTomlFile: () -> Unit,
    onRenameCategory: (String) -> Unit,
    onSetParentColor: (String) -> Unit,
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
    onSearchQueryChange: (String) -> Unit
) {
    var dialogState by remember { mutableStateOf<AliasEditorDialogState?>(null) }
    var showCreateAliasTomlDialog by remember { mutableStateOf(false) }
    var showDeleteAliasTomlDialog by remember { mutableStateOf(false) }
    var showRenameCategoryDialog by remember { mutableStateOf(false) }
    var showAliasFileMenu by remember { mutableStateOf(false) }
    var showCategoryActionsMenu by remember { mutableStateOf(false) }
    var parentColorDraft by remember(document?.parent, document?.color) {
        mutableStateOf(document?.color.orEmpty())
    }
    val categoryName = document?.parent
        ?.takeIf { it.isNotBlank() }
        .orEmpty()
    var currentPathGroupIds by remember(selectedFileDisplayName) {
        mutableStateOf(emptyList<String>())
    }
    Column(
        modifier = Modifier
            .fillMaxWidth()
            .padding(start = 16.dp, end = 16.dp, top = 4.dp, bottom = 16.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                Text(
                    text = stringResource(R.string.config_title_editor_categories),
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.onSurface,
                    maxLines = 1
                )
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    Box(modifier = Modifier.weight(1f)) {
                        Surface(
                            onClick = { showAliasFileMenu = true },
                            modifier = Modifier.fillMaxWidth(),
                            shape = MaterialTheme.shapes.medium,
                            color = MaterialTheme.colorScheme.surfaceVariant
                        ) {
                            Row(
                                modifier = Modifier.padding(horizontal = 16.dp, vertical = 12.dp),
                                verticalAlignment = Alignment.CenterVertically,
                                horizontalArrangement = Arrangement.spacedBy(12.dp)
                            ) {
                                val parentColor = previewParentColor(document?.color.orEmpty())
                                Surface(
                                    modifier = Modifier.size(12.dp),
                                    shape = MaterialTheme.shapes.extraSmall,
                                    color = parentColor ?: MaterialTheme.colorScheme.surfaceVariant,
                                    border = androidx.compose.foundation.BorderStroke(
                                        1.dp,
                                        MaterialTheme.colorScheme.outline
                                    )
                                ) {}
                                Column(modifier = Modifier.weight(1f)) {
                                    Text(
                                        text = categoryName,
                                        style = MaterialTheme.typography.titleMedium,
                                        maxLines = 1
                                    )
                                }
                                Icon(Icons.Filled.ArrowDropDown, contentDescription = null)
                            }
                        }
                    DropdownMenu(
                        expanded = showAliasFileMenu,
                        onDismissRequest = { showAliasFileMenu = false }
                    ) {
                        aliasFiles.forEach { entry ->
                            DropdownMenuItem(
                                text = { Text(entry.displayName.removeSuffix(".toml")) },
                                onClick = {
                                    showAliasFileMenu = false
                                    onSelectAliasFile(entry.relativePath)
                                }
                            )
                        }
                        HorizontalDivider()
                        DropdownMenuItem(
                            text = { Text(stringResource(R.string.config_action_new_alias_mapping)) },
                            leadingIcon = { Icon(Icons.Filled.Add, contentDescription = null) },
                            onClick = {
                                showAliasFileMenu = false
                                showCreateAliasTomlDialog = true
                            }
                        )
                    }
                    }
                    Box {
                        IconButton(onClick = { showCategoryActionsMenu = true }) {
                            Icon(
                                imageVector = Icons.Filled.MoreVert,
                                contentDescription = stringResource(R.string.config_alias_action_category_options)
                            )
                        }
                        DropdownMenu(
                            expanded = showCategoryActionsMenu,
                            onDismissRequest = { showCategoryActionsMenu = false }
                        ) {
                            DropdownMenuItem(
                                text = {
                                    Text(stringResource(R.string.config_alias_action_rename_category))
                                },
                                onClick = {
                                    showCategoryActionsMenu = false
                                    showRenameCategoryDialog = true
                                }
                            )
                            HorizontalDivider()
                            DropdownMenuItem(
                                text = {
                                    Text(
                                        stringResource(R.string.config_action_delete_alias_mapping),
                                        color = MaterialTheme.colorScheme.error
                                    )
                                },
                                leadingIcon = {
                                    Icon(
                                        Icons.Filled.Delete,
                                        contentDescription = null,
                                        tint = MaterialTheme.colorScheme.error
                                    )
                                },
                                onClick = {
                                    showCategoryActionsMenu = false
                                    showDeleteAliasTomlDialog = true
                                }
                            )
                        }
                    }
                }
            }

            ActivityHierarchyParentColorEditor(
                draftValue = parentColorDraft,
                persistedValue = document?.color.orEmpty(),
                onDraftValueChange = { parentColorDraft = it },
                onSaveColor = onSetParentColor
            )

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

            HorizontalDivider(modifier = Modifier.padding(vertical = 8.dp))
            Text(
                text = stringResource(R.string.config_alias_activities_title),
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.onSurface
            )

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
                    searchQuery = searchQuery,
                    onSearchQueryChange = onSearchQueryChange,
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

    if (showCreateAliasTomlDialog) {
        ConfigTomlCreationDialog(
            onDismiss = { showCreateAliasTomlDialog = false },
            onConfirm = { fileName ->
                showCreateAliasTomlDialog = false
                onCreateAliasTomlFile(fileName)
            }
        )
    }
}
