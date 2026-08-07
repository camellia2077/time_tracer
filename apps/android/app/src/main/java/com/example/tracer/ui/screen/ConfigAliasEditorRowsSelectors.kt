package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.rememberScrollState
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ChevronRight
import androidx.compose.material.icons.filled.Edit
import androidx.compose.material.icons.filled.Folder
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ExposedDropdownMenuBox
import androidx.compose.material3.ExposedDropdownMenuAnchorType
import androidx.compose.material3.ExposedDropdownMenuDefaults
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedCard
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.AssistChip
import androidx.compose.material3.AssistChipDefaults
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp

@Composable
internal fun AliasPathBar(
    rootLabel: String,
    breadcrumbs: List<AliasBreadcrumbSegment>,
    onNavigateToBreadcrumb: (String?) -> Unit
) {
    OutlinedCard(modifier = Modifier.fillMaxWidth()) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .horizontalScroll(rememberScrollState())
                .padding(horizontal = 12.dp, vertical = 4.dp)
                .heightIn(min = 48.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            TextButton(
                onClick = { onNavigateToBreadcrumb(null) },
                contentPadding = androidx.compose.foundation.layout.PaddingValues(horizontal = 4.dp)
            ) {
                Text(
                    text = rootLabel,
                    style = MaterialTheme.typography.labelLarge
                )
            }

            breadcrumbs.forEach { breadcrumb ->
                Icon(
                    imageVector = Icons.Filled.ChevronRight,
                    contentDescription = stringResource(R.string.config_alias_path_separator),
                    tint = MaterialTheme.colorScheme.onSurfaceVariant
                )
                TextButton(
                    onClick = { onNavigateToBreadcrumb(breadcrumb.groupId) },
                    contentPadding = androidx.compose.foundation.layout.PaddingValues(horizontal = 4.dp)
                ) {
                    Text(
                        text = breadcrumb.name,
                        style = if (breadcrumb == breadcrumbs.last()) {
                            MaterialTheme.typography.labelLarge
                        } else {
                            MaterialTheme.typography.bodyMedium
                        },
                        color = if (breadcrumb == breadcrumbs.last()) {
                            MaterialTheme.colorScheme.onSurface
                        } else {
                            MaterialTheme.colorScheme.primary
                        },
                        maxLines = 1,
                        overflow = TextOverflow.Ellipsis
                    )
                }
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
internal fun AliasParentSelector(
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
                .menuAnchor(ExposedDropdownMenuAnchorType.PrimaryNotEditable),
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
internal fun AliasGroupRowCard(
    group: ActivityHierarchyGroup,
    onEnterGroup: () -> Unit,
    onEdit: () -> Unit
) {
    OutlinedCard(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.outlinedCardColors(
            containerColor = MaterialTheme.colorScheme.surfaceContainerLow
        )
    ) {
        Column(
            modifier = Modifier.padding(start = 16.dp, top = 8.dp, end = 8.dp, bottom = 8.dp),
            verticalArrangement = Arrangement.spacedBy(4.dp)
        ) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Icon(
                    imageVector = Icons.Filled.Folder,
                    contentDescription = null,
                    tint = MaterialTheme.colorScheme.primary
                )
                Spacer(modifier = Modifier.width(8.dp))
                Text(
                    text = group.name,
                    style = MaterialTheme.typography.titleSmall,
                    modifier = Modifier.weight(1f),
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis
                )
                IconButton(onClick = onEdit) {
                    Icon(
                        Icons.Filled.Edit,
                        contentDescription = stringResource(R.string.config_alias_action_edit_alias)
                    )
                }
                IconButton(onClick = onEnterGroup) {
                    Icon(
                        Icons.Filled.ChevronRight,
                        contentDescription = stringResource(R.string.config_alias_action_open_category)
                    )
                }
            }

            Text(
                text = stringResource(R.string.config_alias_category_child_count, group.nodes.size),
                style = MaterialTheme.typography.bodySmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            if (group.groupAliases.isNotEmpty()) {
                Text(
                    text = stringResource(R.string.config_alias_group_recordable_aliases_label),
                    style = MaterialTheme.typography.labelMedium,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
                Row(
                    horizontalArrangement = Arrangement.spacedBy(4.dp),
                    modifier = Modifier.horizontalScroll(rememberScrollState())
                ) {
                    group.groupAliases.forEach { alias ->
                        AssistChip(
                            onClick = onEdit,
                            label = { Text(alias, maxLines = 1, overflow = TextOverflow.Ellipsis) },
                            colors = AssistChipDefaults.assistChipColors()
                        )
                    }
                }
            }
        }
    }
}

@Composable
internal fun AliasEntryRow(
    entry: ActivityHierarchyLeaf,
    modifier: Modifier = Modifier,
    onEdit: () -> Unit
) {
    OutlinedCard(modifier = modifier) {
        Column(
            modifier = Modifier.padding(start = 16.dp, top = 8.dp, end = 8.dp, bottom = 8.dp),
            verticalArrangement = Arrangement.spacedBy(2.dp)
        ) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Text(
                    text = entry.canonicalLeaf,
                    style = MaterialTheme.typography.titleSmall,
                    modifier = Modifier.weight(1f),
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis
                )
                IconButton(onClick = onEdit) {
                    Icon(
                        Icons.Filled.Edit,
                        contentDescription = stringResource(R.string.config_alias_action_rename)
                    )
                }
            }
            Text(
                text = stringResource(R.string.config_alias_aliases_label),
                style = MaterialTheme.typography.labelMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .horizontalScroll(rememberScrollState()),
                horizontalArrangement = Arrangement.spacedBy(4.dp)
            ) {
                entry.aliases.forEach { alias ->
                    AssistChip(
                        onClick = onEdit,
                        label = { Text(alias, maxLines = 1, overflow = TextOverflow.Ellipsis) },
                        colors = AssistChipDefaults.assistChipColors()
                    )
                }
            }
        }
    }
}

@Composable
internal fun AliasEmptyState() {
    OutlinedCard(modifier = Modifier.fillMaxWidth()) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 20.dp, vertical = 24.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.spacedBy(4.dp)
        ) {
            Text(
                text = stringResource(R.string.config_alias_empty_title),
                style = MaterialTheme.typography.titleSmall
            )
            Text(
                text = stringResource(R.string.config_alias_empty_message),
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
        }
    }
}
