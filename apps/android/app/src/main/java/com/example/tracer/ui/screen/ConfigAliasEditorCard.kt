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
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material3.ElevatedCard
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
internal fun ConfigAliasEditorCard(
    selectedFileDisplayName: String,
    selectedFileContent: String,
    mode: AliasEditorMode,
    document: AliasTomlDocument?,
    parentOptions: List<String>,
    advancedTomlDraft: String,
    errorMessage: String,
    autoSaveStatus: ConfigAutoSaveStatus,
    onCreateAliasTomlFile: (String) -> Unit,
    onDeleteAliasTomlFile: () -> Unit,
    onSelectStructuredMode: () -> Unit,
    onSelectAdvancedMode: () -> Unit,
    onParentChange: (String) -> Unit,
    onAdvancedTomlChange: (String) -> Unit,
    onAddGroup: (parentGroupId: String?, name: String) -> Unit,
    onDeleteGroup: (groupId: String) -> Unit,
    onAddEntry: (parentGroupId: String?, aliasKey: String, canonicalLeaf: String) -> Unit,
    onUpdateEntry: (entryId: String, aliasKey: String, canonicalLeaf: String) -> Unit,
    onDeleteEntry: (entryId: String) -> Unit,
    onSave: () -> Unit
) {
    var dialogState by remember { mutableStateOf<AliasEditorDialogState?>(null) }
    var showDeleteAliasTomlDialog by remember { mutableStateOf(false) }
    var currentPathGroupIds by remember(selectedFileDisplayName) {
        mutableStateOf(emptyList<String>())
    }
    val renderedStructuredDraft = remember(document) {
        document?.let(AliasTomlEditorCodec::serialize).orEmpty()
    }

    LaunchedEffect(mode, selectedFileContent, advancedTomlDraft, renderedStructuredDraft) {
        val currentDraft = when (mode) {
            AliasEditorMode.STRUCTURED -> renderedStructuredDraft
            AliasEditorMode.ADVANCED -> advancedTomlDraft
        }
        if (currentDraft.isBlank() || currentDraft == selectedFileContent) {
            return@LaunchedEffect
        }
        delay(CONFIG_ALIAS_EDITOR_AUTO_SAVE_DELAY_MS)
        val latestDraft = when (mode) {
            AliasEditorMode.STRUCTURED -> renderedStructuredDraft
            AliasEditorMode.ADVANCED -> advancedTomlDraft
        }
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
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary
            )

            ConfigEditorFileControls(
                onCreateAliasTomlFile = onCreateAliasTomlFile
            )

            if (mode == AliasEditorMode.STRUCTURED && document != null) {
                AliasParentSelector(
                    parent = document.parent,
                    parentOptions = parentOptions,
                    onParentChange = onParentChange
                )
            }

            OutlinedButton(
                onClick = { showDeleteAliasTomlDialog = true },
                modifier = Modifier.fillMaxWidth()
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

            ConfigAutoSaveStatusText(
                autoSaveStatus = autoSaveStatus,
                modifier = Modifier.fillMaxWidth()
            )

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
                title = stringResource(R.string.config_alias_dialog_rename_alias_title),
                initialAliasKey = activeDialog.initialAliasKey,
                initialCanonicalLeaf = activeDialog.initialCanonicalLeaf,
                showCanonicalLeafField = false,
                onDismiss = { dialogState = null },
                onConfirm = { aliasKey, canonicalLeaf ->
                    dialogState = null
                    onUpdateEntry(activeDialog.entryId, aliasKey, canonicalLeaf)
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
private fun AliasStructuredEditorContent(
    document: AliasTomlDocument,
    layer: AliasStructuredLayer,
    onNavigateBack: () -> Unit,
    onNavigateToGroup: (String) -> Unit,
    onRequestAddCurrentGroup: () -> Unit,
    onRequestAddCurrentEntry: () -> Unit,
    onDeleteGroup: (String) -> Unit,
    onRequestAddChildGroup: (String) -> Unit,
    onRequestAddChildEntry: (String) -> Unit,
    onRequestEditEntry: (AliasTomlEntry) -> Unit,
    onDeleteEntry: (String) -> Unit
) {
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
    data class AddEntry(val parentGroupId: String?) : AliasEditorDialogState
    data class EditEntry(
        val entryId: String,
        val initialAliasKey: String,
        val initialCanonicalLeaf: String
    ) : AliasEditorDialogState
}
