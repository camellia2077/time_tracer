package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.ColumnScope
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.ChevronRight
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.ExpandMore
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.RadioButton
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

@Composable
internal fun AliasEntryMoveTargetDialog(
    entry: AliasTomlEntry,
    destinations: List<AliasEntryMoveDestinationDocument>,
    loading: Boolean,
    onDismiss: () -> Unit,
    onConfirm: (AliasEntryMoveTarget) -> Unit
) = ActivityHierarchyMoveTargetDialog(
    subjectDescription = stringResource(R.string.config_alias_move_entry_label, entry.aliasKey),
    title = stringResource(R.string.config_alias_dialog_move_entry_title),
    destinations = destinations,
    loading = loading,
    onDismiss = onDismiss,
    onConfirm = onConfirm
)

@Composable
internal fun AliasGroupMoveTargetDialog(
    group: AliasTomlGroup,
    destinations: List<AliasEntryMoveDestinationDocument>,
    loading: Boolean,
    onDismiss: () -> Unit,
    onConfirm: (AliasEntryMoveTarget) -> Unit
) = ActivityHierarchyMoveTargetDialog(
    subjectDescription = stringResource(R.string.config_alias_move_group_label, group.name),
    title = stringResource(R.string.config_alias_dialog_move_group_title),
    destinations = destinations,
    loading = loading,
    onDismiss = onDismiss,
    onConfirm = onConfirm
)

@Composable
private fun ActivityHierarchyMoveTargetDialog(
    subjectDescription: String,
    title: String,
    destinations: List<AliasEntryMoveDestinationDocument>,
    loading: Boolean,
    onDismiss: () -> Unit,
    onConfirm: (AliasEntryMoveTarget) -> Unit
) {
    var selectedTarget by remember(destinations) {
        mutableStateOf(destinations.firstOrNull { it.rootSelectable }?.let {
            AliasEntryMoveTarget(it.sourceName, emptyList())
        })
    }
    var expandedPaths by remember(destinations) { mutableStateOf(emptySet<String>()) }

    ActivityHierarchyTargetSelectionDialog(
        subjectDescription = subjectDescription,
        title = title,
        loading = loading,
        emptyMessage = stringResource(R.string.config_alias_move_no_destination),
        hasContent = destinations.isNotEmpty(),
        confirmLabel = stringResource(R.string.config_alias_action_preview_move),
        confirmEnabled = selectedTarget != null,
        onDismiss = onDismiss,
        onConfirm = { selectedTarget?.let(onConfirm) }
    ) {
        destinations.forEach { destination ->
            Text(
                text = destination.displayName,
                style = MaterialTheme.typography.titleSmall
            )
            if (destination.rootSelectable) {
                val target = AliasEntryMoveTarget(destination.sourceName, emptyList())
                AliasMoveTargetRow(
                    label = stringResource(R.string.config_alias_move_target_root),
                    selected = selectedTarget == target,
                    depth = 0,
                    expandable = false,
                    expanded = false,
                    onExpand = {},
                    onSelect = { selectedTarget = target }
                )
            }
            destination.document.nodes
                .filterIsInstance<AliasTomlGroup>()
                .forEach { group ->
                    AliasMoveTargetGroupTree(
                        sourceName = destination.sourceName,
                        group = group,
                        groupPath = emptyList(),
                        excludedGroupPath = destination.excludedGroupPath,
                        excludeDescendants = destination.excludeDescendants,
                        expandedPaths = expandedPaths,
                        selectedTarget = selectedTarget,
                        onExpandedPathsChange = { expandedPaths = it },
                        onSelect = { selectedTarget = it }
                    )
                }
        }
    }
}

@Composable
internal fun ActivityHierarchyTargetSelectionDialog(
    subjectDescription: String,
    title: String,
    loading: Boolean,
    emptyMessage: String,
    hasContent: Boolean,
    confirmLabel: String,
    confirmEnabled: Boolean,
    onDismiss: () -> Unit,
    onConfirm: () -> Unit,
    content: @Composable ColumnScope.() -> Unit
) {
    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(title) },
        text = {
            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                Text(
                    text = subjectDescription,
                    style = MaterialTheme.typography.bodyMedium
                )
                Text(
                    text = stringResource(R.string.config_alias_move_target_toml),
                    style = MaterialTheme.typography.labelLarge
                )
                if (loading) {
                    Text(
                        text = stringResource(R.string.config_alias_move_loading),
                        color = MaterialTheme.colorScheme.onSurfaceVariant,
                        style = MaterialTheme.typography.bodySmall
                    )
                } else if (!hasContent) {
                    Text(
                        text = emptyMessage,
                        color = MaterialTheme.colorScheme.onSurfaceVariant,
                        style = MaterialTheme.typography.bodySmall
                    )
                } else {
                    Column(
                        modifier = Modifier
                            .heightIn(max = 360.dp)
                            .verticalScroll(rememberScrollState()),
                        verticalArrangement = Arrangement.spacedBy(4.dp),
                        content = content
                    )
                }
            }
        },
        confirmButton = {
            TextButton(
                enabled = confirmEnabled && !loading,
                onClick = onConfirm
            ) {
                Text(confirmLabel)
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
private fun AliasMoveTargetGroupTree(
    sourceName: String,
    group: AliasTomlGroup,
    groupPath: List<String>,
    excludedGroupPath: List<String>,
    excludeDescendants: Boolean,
    expandedPaths: Set<String>,
    selectedTarget: AliasEntryMoveTarget?,
    onExpandedPathsChange: (Set<String>) -> Unit,
    onSelect: (AliasEntryMoveTarget) -> Unit
) {
    val path = groupPath + group.name
    val pathKey = "$sourceName:${path.joinToString(".")}"
    val expanded = pathKey in expandedPaths
    val selectable = if (excludeDescendants) {
        path.size < excludedGroupPath.size ||
            path.take(excludedGroupPath.size) != excludedGroupPath
    } else {
        path != excludedGroupPath
    }
    val target = AliasEntryMoveTarget(sourceName, path, group.id)
    AliasMoveTargetRow(
        label = group.name,
        selected = selectable && selectedTarget == target,
        depth = path.size,
        expandable = group.nodes.any { it is AliasTomlGroup },
        expanded = expanded,
        onExpand = {
            onExpandedPathsChange(
                if (expanded) expandedPaths - pathKey else expandedPaths + pathKey
            )
        },
        onSelect = { if (selectable) onSelect(target) }
    )
    if (expanded) {
        group.nodes.filterIsInstance<AliasTomlGroup>().forEach { child ->
            AliasMoveTargetGroupTree(
                sourceName = sourceName,
                group = child,
                groupPath = path,
                excludedGroupPath = excludedGroupPath,
                excludeDescendants = excludeDescendants,
                expandedPaths = expandedPaths,
                selectedTarget = selectedTarget,
                onExpandedPathsChange = onExpandedPathsChange,
                onSelect = onSelect
            )
        }
    }
}

@Composable
internal fun AliasMoveTargetRow(
    label: String,
    selected: Boolean,
    depth: Int,
    expandable: Boolean,
    expanded: Boolean,
    onExpand: () -> Unit,
    onSelect: () -> Unit
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(start = (depth * 16).dp),
        verticalAlignment = androidx.compose.ui.Alignment.CenterVertically
    ) {
        RadioButton(selected = selected, onClick = onSelect)
        TextButton(
            onClick = onSelect,
            modifier = Modifier.weight(1f)
        ) {
            Text(label)
        }
        if (expandable) {
            IconButton(onClick = onExpand) {
                Icon(
                    imageVector = if (expanded) Icons.Filled.ExpandMore else Icons.Filled.ChevronRight,
                    contentDescription = null
                )
            }
        }
    }
}

