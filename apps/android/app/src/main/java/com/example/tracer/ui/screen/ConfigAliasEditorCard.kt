package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.ChevronRight
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.Edit
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.ExposedDropdownMenuBox
import androidx.compose.material3.ExposedDropdownMenuDefaults
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.OutlinedCard
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.semantics.semantics
import androidx.compose.ui.semantics.heading
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.SpanStyle
import androidx.compose.ui.text.buildAnnotatedString
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.text.withStyle
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
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
                                // In drill-down mode, add actions always target current layer.
                                dialogState = AliasEditorDialogState.AddGroup(
                                    parentGroupId = layer.currentParentGroupId
                                )
                            },
                            onRequestAddCurrentEntry = {
                                // In drill-down mode, add actions always target current layer.
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

@Composable
private fun AliasPathBar(
    breadcrumbs: List<AliasBreadcrumbSegment>,
    onNavigateBack: () -> Unit
) {
    val pathText = remember(breadcrumbs) {
        if (breadcrumbs.isEmpty()) {
            "Current path: aliases"
        } else {
            "Current path: aliases / " + breadcrumbs.joinToString(" / ") { it.name }
        }
    }

    OutlinedCard(modifier = Modifier.fillMaxWidth()) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 8.dp, vertical = 6.dp)
                // Keep baseline bar height consistent between root/non-root.
                .heightIn(min = 48.dp),
            verticalAlignment = Alignment.Top
        ) {
            if (breadcrumbs.isNotEmpty()) {
                IconButton(onClick = onNavigateBack) {
                    Icon(
                        Icons.AutoMirrored.Filled.ArrowBack,
                        contentDescription = "Back to parent group"
                    )
                }
            } else {
                // Reserve the back-button slot even at root so path bar height and
                // text alignment remain visually stable across navigation depth.
                Spacer(modifier = Modifier.width(48.dp))
            }
            Text(
                text = pathText,
                style = MaterialTheme.typography.bodyMedium,
                modifier = Modifier
                    .weight(1f)
                    .semantics { heading() },
                // Let deep paths wrap on narrow devices instead of hiding the
                // navigation context behind aggressive single-line truncation.
                maxLines = 3,
                overflow = TextOverflow.Clip
            )
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun AliasParentSelector(
    parent: String,
    parentOptions: List<String>,
    onParentChange: (String) -> Unit
) {
    var expanded by remember { mutableStateOf(false) }
    val options = remember(parent, parentOptions) {
        val merged = if (parent.isBlank()) {
            parentOptions
        } else {
            parentOptions + parent
        }
        merged
            .map { it.trim() }
            .filter { it.isNotEmpty() }
            .distinct()
            .sortedWith(
                compareBy<String> { it.lowercase() }
                    .thenBy { it }
            )
    }

    ExposedDropdownMenuBox(
        expanded = expanded,
        onExpandedChange = { expanded = options.isNotEmpty() && !expanded }
    ) {
        OutlinedTextField(
            value = parent,
            onValueChange = {},
            readOnly = true,
                label = { Text(stringResource(R.string.config_alias_parent_label)) },
                trailingIcon = { ExposedDropdownMenuDefaults.TrailingIcon(expanded = expanded) },
                modifier = Modifier
                    .fillMaxWidth()
                    .menuAnchor(),
        )
        DropdownMenu(
            expanded = expanded,
            onDismissRequest = { expanded = false }
        ) {
            options.forEach { option ->
                DropdownMenuItem(
                    text = { Text(option) },
                    onClick = {
                        expanded = false
                        if (option != parent) {
                            onParentChange(option)
                        }
                    }
                )
            }
        }
    }
}

@Composable
private fun AliasGroupRowCard(
    group: AliasTomlGroup,
    onEnterGroup: () -> Unit,
    onRequestRenameGroup: (AliasTomlGroup) -> Unit,
    onDeleteGroup: (String) -> Unit,
    onRequestAddChildGroup: (String) -> Unit,
    onRequestAddChildEntry: (String) -> Unit
) {
    var showAddMenu by remember(group.id) { mutableStateOf(false) }

    OutlinedCard(modifier = Modifier.fillMaxWidth()) {
        Column(
            modifier = Modifier.padding(12.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = group.name,
                    style = MaterialTheme.typography.titleSmall,
                    // Keep the title row text-focused: actions are moved to a
                    // dedicated second row so long group names can use width.
                    modifier = Modifier.weight(1f),
                    maxLines = 2,
                    overflow = TextOverflow.Ellipsis
                )
                IconButton(onClick = onEnterGroup) {
                    Icon(
                        Icons.Filled.ChevronRight,
                        contentDescription = "Open child group"
                    )
                }
            }

            // Dedicated action row prevents long group titles from being forced
            // into aggressive truncation by inline action buttons.
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.End,
                verticalAlignment = Alignment.CenterVertically
            ) {
                // Keep icon-only actions to preserve narrow-screen stability.
                IconButton(onClick = { onRequestRenameGroup(group) }) {
                    Icon(
                        Icons.Filled.Edit,
                        contentDescription = stringResource(R.string.config_alias_action_rename),
                        tint = Color(0xFF4F46E5)
                    )
                }
                IconButton(onClick = { onDeleteGroup(group.id) }) {
                    Icon(
                        Icons.Filled.Delete,
                        contentDescription = stringResource(R.string.config_alias_action_delete),
                        tint = MaterialTheme.colorScheme.error
                    )
                }
                IconButton(onClick = { showAddMenu = true }) {
                    Icon(
                        Icons.Filled.Add,
                        contentDescription = stringResource(R.string.config_alias_action_add_group)
                    )
                }
                DropdownMenu(
                    expanded = showAddMenu,
                    onDismissRequest = { showAddMenu = false }
                ) {
                    DropdownMenuItem(
                        text = { Text(stringResource(R.string.config_alias_action_add_group)) },
                        onClick = {
                            showAddMenu = false
                            onRequestAddChildGroup(group.id)
                        }
                    )
                    DropdownMenuItem(
                        text = { Text(stringResource(R.string.config_alias_action_add_alias)) },
                        onClick = {
                            showAddMenu = false
                            onRequestAddChildEntry(group.id)
                        }
                    )
                }
            }
        }
    }
}

@Composable
private fun AliasEntryRow(
    entry: AliasTomlEntry,
    modifier: Modifier = Modifier,
    onEdit: () -> Unit,
    onDelete: () -> Unit
) {
    OutlinedCard(modifier = modifier) {
        Column(
            modifier = Modifier.padding(12.dp),
            verticalArrangement = Arrangement.spacedBy(4.dp)
        ) {
            Text(
                text = entry.aliasKey,
                style = MaterialTheme.typography.titleSmall,
                modifier = Modifier.fillMaxWidth(),
                maxLines = 1,
                overflow = TextOverflow.Ellipsis
            )
            Text(
                // Slightly enlarge only the arrow marker while keeping body text
                // typography unchanged to reinforce mapping intent.
                text = buildAnnotatedString {
                    withStyle(SpanStyle(fontSize = 24.sp)) {
                        append("\u2192 ")
                    }
                    append(entry.canonicalLeaf)
                },
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                // Long canonical leaves can be unbroken tokens (no spaces), so
                // cap lines and ellipsize to prevent the "vertical strip" effect.
                modifier = Modifier.fillMaxWidth(),
                maxLines = 2,
                overflow = TextOverflow.Ellipsis
            )
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.End
            ) {
                TextButton(onClick = onEdit) {
                    Icon(Icons.Filled.Edit, contentDescription = null)
                    Spacer(modifier = Modifier.width(4.dp))
                    Text(stringResource(R.string.config_alias_action_rename))
                }
                TextButton(onClick = onDelete) {
                    Icon(Icons.Filled.Delete, contentDescription = null, tint = MaterialTheme.colorScheme.error)
                    Spacer(modifier = Modifier.width(4.dp))
                    Text(
                        text = stringResource(R.string.config_alias_action_delete),
                        color = MaterialTheme.colorScheme.error
                    )
                }
            }
        }
    }
}

@Composable
private fun AliasGroupNameDialog(
    title: String,
    initialName: String,
    onDismiss: () -> Unit,
    onConfirm: (String) -> Unit
) {
    var value by remember(initialName) { mutableStateOf(initialName) }
    var showError by remember { mutableStateOf(false) }

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(title) },
        text = {
            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                OutlinedTextField(
                    value = value,
                    onValueChange = {
                        value = it
                        showError = false
                    },
                    label = { Text(stringResource(R.string.config_alias_group_name_label)) },
                    modifier = Modifier.fillMaxWidth(),
                    isError = showError
                )
                if (showError) {
                    Text(
                        text = stringResource(R.string.config_alias_group_name_required),
                        color = MaterialTheme.colorScheme.error,
                        style = MaterialTheme.typography.bodySmall
                    )
                }
            }
        },
        confirmButton = {
            TextButton(
                onClick = {
                    if (value.trim().isEmpty()) {
                        showError = true
                    } else {
                        onConfirm(value)
                    }
                }
            ) {
                Text(stringResource(android.R.string.ok))
            }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) {
                Text(stringResource(android.R.string.cancel))
            }
        }
    )
}

@Composable
private fun AliasEntryDialog(
    title: String,
    initialAliasKey: String,
    initialCanonicalLeaf: String,
    onDismiss: () -> Unit,
    onConfirm: (String, String) -> Unit
) {
    var aliasKey by remember(initialAliasKey) { mutableStateOf(initialAliasKey) }
    var canonicalLeaf by remember(initialCanonicalLeaf) { mutableStateOf(initialCanonicalLeaf) }
    var showError by remember { mutableStateOf(false) }

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(title) },
        text = {
            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                OutlinedTextField(
                    value = aliasKey,
                    onValueChange = {
                        aliasKey = it
                        showError = false
                    },
                    label = { Text(stringResource(R.string.config_alias_alias_key_label)) },
                    modifier = Modifier.fillMaxWidth(),
                    isError = showError
                )
                OutlinedTextField(
                    value = canonicalLeaf,
                    onValueChange = {
                        canonicalLeaf = it
                        showError = false
                    },
                    label = { Text(stringResource(R.string.config_alias_canonical_leaf_label)) },
                    modifier = Modifier.fillMaxWidth(),
                    isError = showError
                )
                if (showError) {
                    Text(
                        text = stringResource(R.string.config_alias_entry_required),
                        color = MaterialTheme.colorScheme.error,
                        style = MaterialTheme.typography.bodySmall
                    )
                }
            }
        },
        confirmButton = {
            TextButton(
                onClick = {
                    if (aliasKey.trim().isEmpty() || canonicalLeaf.trim().isEmpty()) {
                        showError = true
                    } else {
                        onConfirm(aliasKey, canonicalLeaf)
                    }
                }
            ) {
                Text(stringResource(android.R.string.ok))
            }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) {
                Text(stringResource(android.R.string.cancel))
            }
        }
    )
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

private sealed interface AliasEditorDialogState {
    data class AddGroup(val parentGroupId: String?) : AliasEditorDialogState
    data class RenameGroup(val groupId: String, val initialName: String) : AliasEditorDialogState
    data class AddEntry(val parentGroupId: String?) : AliasEditorDialogState
    data class EditEntry(
        val entryId: String,
        val initialAliasKey: String,
        val initialCanonicalLeaf: String
    ) : AliasEditorDialogState
}
