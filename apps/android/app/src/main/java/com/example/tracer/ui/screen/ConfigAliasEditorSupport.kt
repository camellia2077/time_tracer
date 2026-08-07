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
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
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


internal const val CONFIG_ALIAS_EDITOR_AUTO_SAVE_DELAY_MS = 600L

@Composable
internal fun AliasEntryMovePlanPreview(
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
                if (plan.sourceFilePath.isNotBlank() && plan.destinationFilePath.isNotBlank()) {
                    Text(
                        text = stringResource(
                            R.string.config_alias_move_plan_location,
                            plan.sourceFilePath.removePrefix("user/activity_hierarchy/"),
                            plan.sourceGroupPath.joinToString(" / ").ifBlank {
                                stringResource(R.string.config_alias_move_target_root)
                            },
                            plan.destinationFilePath.removePrefix("user/activity_hierarchy/"),
                            plan.destinationGroupPath.joinToString(" / ").ifBlank {
                                stringResource(R.string.config_alias_move_target_root)
                            }
                        ),
                        style = MaterialTheme.typography.bodySmall
                    )
                }
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
internal fun AliasStructuredEditorContent(
    document: ActivityHierarchyDocument,
    layer: AliasStructuredLayer,
    onNavigateToBreadcrumb: (String?) -> Unit,
    onNavigateToGroup: (String) -> Unit,
    onRequestAddCurrentGroup: () -> Unit,
    onRequestAddCurrentEntry: () -> Unit,
    onRequestEditGroup: (ActivityHierarchyGroup) -> Unit,
    onRequestEditEntry: (ActivityHierarchyLeaf) -> Unit
) {
    AliasPathBar(
        rootLabel = document.parent.ifBlank {
            stringResource(R.string.config_alias_path_root)
        },
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
    val currentNodes: List<ActivityHierarchyDocumentNode>
) {
    val currentGroups: List<ActivityHierarchyGroup>
        get() = currentNodes.filterIsInstance<ActivityHierarchyGroup>()
    val currentEntries: List<ActivityHierarchyLeaf>
        get() = currentNodes.filterIsInstance<ActivityHierarchyLeaf>()
    val currentParentGroupId: String?
        get() = normalizedPathGroupIds.lastOrNull()
}

internal fun resolveAliasStructuredLayer(
    document: ActivityHierarchyDocument,
    pathGroupIds: List<String>
): AliasStructuredLayer {
    // Drill-down design choice: render only one layer at a time and derive that
    // layer by walking the requested path until the first invalid segment.
    // This guarantees deterministic fallback to the nearest valid ancestor.
    val normalizedPath = mutableListOf<String>()
    val breadcrumbs = mutableListOf<AliasBreadcrumbSegment>()
    var currentNodes: List<ActivityHierarchyDocumentNode> = document.nodes

    for (candidateId in pathGroupIds) {
        val nextGroup = currentNodes
            .filterIsInstance<ActivityHierarchyGroup>()
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
    data class EditEntryAliases(val entry: ActivityHierarchyLeaf) : AliasEditorDialogState
    data class EditGroupName(val group: ActivityHierarchyGroup) : AliasEditorDialogState
    data class EditEntryName(val entry: ActivityHierarchyLeaf) : AliasEditorDialogState
    data class GroupActions(val group: ActivityHierarchyGroup) : AliasEditorDialogState
    data class EntryActions(val entry: ActivityHierarchyLeaf) : AliasEditorDialogState
    data class MergeEntry(val entry: ActivityHierarchyLeaf) : AliasEditorDialogState
    data class PlanEntryMove(val entry: ActivityHierarchyLeaf) : AliasEditorDialogState
    data class PlanGroupMove(val group: ActivityHierarchyGroup) : AliasEditorDialogState
    data class ConfirmPromote(val entry: ActivityHierarchyLeaf) : AliasEditorDialogState
    data class EditGroupAliases(val group: ActivityHierarchyGroup) : AliasEditorDialogState
    data class AddGroupAlias(val groupId: String) : AliasEditorDialogState
    data class ConfirmDeleteGroup(val group: ActivityHierarchyGroup) : AliasEditorDialogState
    data class ConfirmDeleteEntry(val entry: ActivityHierarchyLeaf) : AliasEditorDialogState
}

