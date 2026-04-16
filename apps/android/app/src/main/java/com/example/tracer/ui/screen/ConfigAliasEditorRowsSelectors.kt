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
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.ChevronRight
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.Edit
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ExposedDropdownMenuBox
import androidx.compose.material3.ExposedDropdownMenuDefaults
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedCard
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.semantics.heading
import androidx.compose.ui.semantics.semantics
import androidx.compose.ui.text.SpanStyle
import androidx.compose.ui.text.buildAnnotatedString
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.text.withStyle
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

@Composable
internal fun AliasPathBar(
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
                Spacer(modifier = Modifier.width(48.dp))
            }
            Text(
                text = pathText,
                style = MaterialTheme.typography.bodyMedium,
                modifier = Modifier
                    .weight(1f)
                    .semantics { heading() },
                maxLines = 3,
                overflow = TextOverflow.Clip
            )
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
internal fun AliasGroupRowCard(
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

            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.End,
                verticalAlignment = Alignment.CenterVertically
            ) {
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
internal fun AliasEntryRow(
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
                text = buildAnnotatedString {
                    withStyle(SpanStyle(fontSize = 24.sp)) {
                        append("\u2192 ")
                    }
                    append(entry.canonicalLeaf)
                },
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
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
