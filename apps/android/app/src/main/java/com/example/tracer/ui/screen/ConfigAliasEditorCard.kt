package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.Check
import androidx.compose.material3.Button
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.OutlinedTextField
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
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.unit.dp
import com.example.tracer.ui.components.TracerSegmentedButtonDefaults

@Composable
internal fun ConfigAliasEditorCard(
    selectedFileDisplayName: String,
    mode: AliasEditorMode,
    document: AliasTomlDocument?,
    parentOptions: List<String>,
    advancedTomlDraft: String,
    errorMessage: String,
    onSelectStructuredMode: () -> Unit,
    onSelectAdvancedMode: () -> Unit,
    onParentChange: (String) -> Unit,
    onAdvancedTomlChange: (String) -> Unit,
    onAddGroup: (parentGroupId: String?, name: String) -> Unit,
    onRenameGroup: (groupId: String, name: String) -> Unit,
    onDeleteGroup: (groupId: String) -> Unit,
    onAddEntry: (parentGroupId: String?, aliasKey: String, canonicalLeaf: String) -> Unit,
    onUpdateEntry: (entryId: String, aliasKey: String, canonicalLeaf: String) -> Unit,
    onDeleteEntry: (entryId: String) -> Unit,
    onSave: () -> Unit
) {
    var dialogState by remember { mutableStateOf<AliasEditorDialogState?>(null) }
    var currentPathGroupIds by remember(selectedFileDisplayName) {
        mutableStateOf(emptyList<String>())
    }

    ElevatedCard(modifier = Modifier.fillMaxWidth()) {
        Column(
            modifier = Modifier.padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Text(
                text = stringResource(R.string.config_title_editor_file, selectedFileDisplayName),
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary
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
                            parentOptions = parentOptions,
                            layer = layer,
                            onParentChange = onParentChange,
                            onNavigateBack = {
                                currentPathGroupIds = layer.normalizedPathGroupIds.dropLast(1)
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
                            onRequestRenameGroup = { group ->
                                dialogState = AliasEditorDialogState.RenameGroup(
                                    groupId = group.id,
                                    initialName = group.name
                                )
                            },
                            onDeleteGroup = onDeleteGroup,
                            onRequestAddChildGroup = { groupId ->
                                dialogState = AliasEditorDialogState.AddGroup(parentGroupId = groupId)
                            },
                            onRequestAddChildEntry = { groupId ->
                                dialogState = AliasEditorDialogState.AddEntry(parentGroupId = groupId)
                            },
                            onRequestEditEntry = { entry ->
                                dialogState = AliasEditorDialogState.EditEntry(
                                    entryId = entry.id,
                                    initialAliasKey = entry.aliasKey,
                                    initialCanonicalLeaf = entry.canonicalLeaf
                                )
                            },
                            onDeleteEntry = onDeleteEntry
                        )
                    }
                }

                AliasEditorMode.ADVANCED -> {
                    OutlinedTextField(
                        value = advancedTomlDraft,
                        onValueChange = onAdvancedTomlChange,
                        label = { Text(stringResource(R.string.config_label_toml_content)) },
                        modifier = Modifier.fillMaxWidth(),
                        minLines = 12,
                        textStyle = MaterialTheme.typography.bodyMedium.copy(
                            fontFamily = FontFamily.Monospace
                        )
                    )
                }
            }

            Button(
                onClick = onSave,
                modifier = Modifier.fillMaxWidth()
            ) {
                Icon(Icons.Filled.Check, contentDescription = null)
                Spacer(modifier = Modifier.width(8.dp))
                Text(stringResource(R.string.config_action_save_changes))
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

        is AliasEditorDialogState.RenameGroup -> {
            AliasGroupNameDialog(
                title = stringResource(R.string.config_alias_dialog_rename_group_title),
                initialName = activeDialog.initialName,
                onDismiss = { dialogState = null },
                onConfirm = { name ->
                    dialogState = null
                    onRenameGroup(activeDialog.groupId, name)
                }
            )
        }

        is AliasEditorDialogState.AddEntry -> {
            AliasEntryDialog(
                title = stringResource(R.string.config_alias_dialog_add_entry_title),
                initialAliasKey = "",
                initialCanonicalLeaf = "",
                onDismiss = { dialogState = null },
                onConfirm = { aliasKey, canonicalLeaf ->
                    dialogState = null
                    onAddEntry(activeDialog.parentGroupId, aliasKey, canonicalLeaf)
                }
            )
        }

        is AliasEditorDialogState.EditEntry -> {
            AliasEntryDialog(
                title = stringResource(R.string.config_alias_dialog_edit_entry_title),
                initialAliasKey = activeDialog.initialAliasKey,
                initialCanonicalLeaf = activeDialog.initialCanonicalLeaf,
                onDismiss = { dialogState = null },
                onConfirm = { aliasKey, canonicalLeaf ->
                    dialogState = null
                    onUpdateEntry(activeDialog.entryId, aliasKey, canonicalLeaf)
                }
            )
        }

        null -> Unit
    }
}

@Composable
private fun AliasStructuredEditorContent(
    document: AliasTomlDocument,
    parentOptions: List<String>,
    layer: AliasStructuredLayer,
    onParentChange: (String) -> Unit,
    onNavigateBack: () -> Unit,
    onNavigateToGroup: (String) -> Unit,
    onRequestAddCurrentGroup: () -> Unit,
    onRequestAddCurrentEntry: () -> Unit,
    onRequestRenameGroup: (AliasTomlGroup) -> Unit,
    onDeleteGroup: (String) -> Unit,
    onRequestAddChildGroup: (String) -> Unit,
    onRequestAddChildEntry: (String) -> Unit,
    onRequestEditEntry: (AliasTomlEntry) -> Unit,
    onDeleteEntry: (String) -> Unit
) {
    AliasParentSelector(
        parent = document.parent,
        parentOptions = parentOptions,
        onParentChange = onParentChange
    )

    AliasPathBar(
        breadcrumbs = layer.breadcrumbs,
        onNavigateBack = onNavigateBack
    )

    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        OutlinedButton(
            onClick = onRequestAddCurrentGroup,
            modifier = Modifier.weight(1f)
        ) {
            Icon(Icons.Filled.Add, contentDescription = null)
            Spacer(modifier = Modifier.width(8.dp))
            Text(stringResource(R.string.config_alias_action_add_group))
        }
        OutlinedButton(
            onClick = onRequestAddCurrentEntry,
            modifier = Modifier.weight(1f)
        ) {
            Icon(Icons.Filled.Add, contentDescription = null)
            Spacer(modifier = Modifier.width(8.dp))
            Text(stringResource(R.string.config_alias_action_add_alias))
        }
    }

    for (group in layer.currentGroups) {
        AliasGroupRowCard(
            group = group,
            onEnterGroup = { onNavigateToGroup(group.id) },
            onRequestRenameGroup = onRequestRenameGroup,
            onDeleteGroup = onDeleteGroup,
            onRequestAddChildGroup = onRequestAddChildGroup,
            onRequestAddChildEntry = onRequestAddChildEntry
        )
    }

    for (entry in layer.currentEntries) {
        AliasEntryRow(
            entry = entry,
            modifier = Modifier.fillMaxWidth(),
            onEdit = { onRequestEditEntry(entry) },
            onDelete = { onDeleteEntry(entry.id) }
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
    data class RenameGroup(val groupId: String, val initialName: String) : AliasEditorDialogState
    data class AddEntry(val parentGroupId: String?) : AliasEditorDialogState
    data class EditEntry(
        val entryId: String,
        val initialAliasKey: String,
        val initialCanonicalLeaf: String
    ) : AliasEditorDialogState
}
