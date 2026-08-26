package com.example.tracer

import androidx.activity.compose.BackHandler
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ChevronRight
import androidx.compose.material.icons.filled.Close
import androidx.compose.material.icons.filled.Home
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.ListItem
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.RadioButton
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
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.insights.R

@Composable
internal fun InsightsChartRootPickerPage(
    rootNodes: List<TreeNode>,
    selectedPath: String,
    onPathSelected: (String) -> Unit,
    onDismiss: () -> Unit
) {
    FullscreenPage(
        onDismissRequest = onDismiss,
        backgroundColor = MaterialTheme.colorScheme.background
    ) {
        InsightsChartRootPickerScreen(
            rootNodes = rootNodes,
            selectedPath = selectedPath,
            onPathSelected = onPathSelected,
            onDismiss = onDismiss
        )
    }
}

@Composable
internal fun InsightsChartRootPickerScreen(
    rootNodes: List<TreeNode>,
    selectedPath: String,
    onPathSelected: (String) -> Unit,
    onDismiss: () -> Unit
) {
    var currentPath by remember { mutableStateOf("") }
    val currentNode = findInsightsChartTreeNode(rootNodes, currentPath)
    val currentNodes = if (currentPath.isBlank()) {
        rootNodes
    } else {
        currentNode?.children.orEmpty()
    }
    val currentPathNodes = findInsightsChartTreePath(rootNodes, currentPath)
    BackHandler(enabled = currentPath.isNotBlank()) {
        currentPath = currentPathNodes
            ?.dropLast(1)
            ?.lastOrNull()
            ?.path
            .orEmpty()
    }

    Column(modifier = Modifier.fillMaxSize()) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 8.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            IconButton(onClick = onDismiss) {
                Icon(
                    imageVector = Icons.Default.Close,
                    contentDescription = stringResource(R.string.insights_chart_root_close)
                )
            }
            Text(
                text = stringResource(R.string.insights_chart_root_picker_title),
                style = MaterialTheme.typography.titleLarge,
                modifier = Modifier.weight(1f)
            )
            TextButton(onClick = onDismiss) {
                Text(stringResource(R.string.insights_chart_root_done))
            }
        }

        Text(
            text = stringResource(
                R.string.insights_chart_root_current_selection,
                formatInsightsChartScopeLabel(selectedPath, rootNodes)
            ),
            style = MaterialTheme.typography.bodyMedium,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
            modifier = Modifier.padding(horizontal = 24.dp, vertical = 8.dp)
        )

        if (currentPath.isBlank()) {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .clickable { currentPath = "" }
                    .padding(horizontal = 16.dp, vertical = 4.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Icon(
                    imageVector = Icons.Default.Home,
                    contentDescription = null,
                    tint = MaterialTheme.colorScheme.onSurfaceVariant,
                    modifier = Modifier.padding(end = 12.dp)
                )
                Text(
                    text = stringResource(R.string.insights_chart_root_all),
                    modifier = Modifier.weight(1f)
                )
                RadioButton(
                    selected = selectedPath.isBlank(),
                    onClick = { onPathSelected("") }
                )
            }

            HorizontalDivider()
        }

        if (currentPathNodes != null) {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .horizontalScroll(rememberScrollState())
                    .padding(horizontal = 16.dp, vertical = 4.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = stringResource(R.string.insights_chart_root_all),
                    color = MaterialTheme.colorScheme.primary,
                    modifier = Modifier.clickable { currentPath = "" }
                )
                currentPathNodes.forEach { node ->
                    Text(
                        text = " › ",
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                    Text(
                        text = node.name,
                        color = MaterialTheme.colorScheme.primary,
                        modifier = Modifier.clickable { currentPath = node.path }
                    )
                }
            }
        }

        LazyColumn(modifier = Modifier.fillMaxWidth()) {
            items(currentNodes, key = { it.path }) { node ->
                ListItem(
                    headlineContent = {
                        Text(
                            text = node.name,
                            modifier = Modifier.clickable {
                                if (node.children.isEmpty()) {
                                    onPathSelected(node.path)
                                } else {
                                    currentPath = node.path
                                }
                            }
                        )
                    },
                    supportingContent = if (node.children.isNotEmpty()) {
                        { Text(stringResource(R.string.insights_chart_root_has_children)) }
                    } else {
                        null
                    },
                    leadingContent = {
                        RadioButton(
                            selected = selectedPath == node.path,
                            onClick = { onPathSelected(node.path) }
                        )
                    },
                    trailingContent = if (node.children.isNotEmpty()) {
                        {
                            IconButton(onClick = { currentPath = node.path }) {
                                Icon(
                                    imageVector = Icons.Default.ChevronRight,
                                    contentDescription = stringResource(
                                        R.string.insights_chart_root_open_children
                                    )
                                )
                            }
                        }
                    } else {
                        null
                    }
                )
            }
        }
    }
}

internal fun findInsightsChartTreeNode(
    nodes: List<TreeNode>,
    path: String
): TreeNode? {
    if (path.isBlank()) {
        return null
    }
    nodes.forEach { node ->
        if (node.path == path) {
            return node
        }
        findInsightsChartTreeNode(node.children, path)?.let { return it }
    }
    return null
}

internal fun formatInsightsChartScopeLabel(
    selectedPath: String,
    rootNodes: List<TreeNode>
): String {
    if (selectedPath.isBlank()) {
        return ""
    }
    return findInsightsChartTreePathLabels(rootNodes, selectedPath)
        ?.joinToString(" › ")
        ?: selectedPath.replace("_", " › ")
}

private fun findInsightsChartTreePathLabels(
    nodes: List<TreeNode>,
    targetPath: String,
    ancestors: List<String> = emptyList()
): List<String>? {
    nodes.forEach { node ->
        val path = ancestors + node.name
        if (node.path == targetPath) {
            return path
        }
        findInsightsChartTreePathLabels(node.children, targetPath, path)?.let { return it }
    }
    return null
}

private fun findInsightsChartTreePath(
    nodes: List<TreeNode>,
    targetPath: String,
    ancestors: List<TreeNode> = emptyList()
): List<TreeNode>? {
    if (targetPath.isBlank()) {
        return null
    }
    nodes.forEach { node ->
        val path = ancestors + node
        if (node.path == targetPath) {
            return path
        }
        findInsightsChartTreePath(node.children, targetPath, path)?.let { return it }
    }
    return null
}
