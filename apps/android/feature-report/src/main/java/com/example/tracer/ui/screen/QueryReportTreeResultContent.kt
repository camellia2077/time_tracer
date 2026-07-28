package com.example.tracer

import androidx.compose.foundation.background
import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.rememberScrollState
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowDownward
import androidx.compose.material.icons.filled.ArrowUpward
import androidx.compose.material.icons.filled.ChevronRight
import androidx.compose.material.icons.filled.ExpandMore
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.Icon
import androidx.compose.material3.LinearProgressIndicator
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.AssistChip
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.unit.dp
import androidx.compose.ui.res.stringResource
import com.example.tracer.feature.report.R
import java.util.Locale

@Composable
internal fun QueryReportTreeResultContent(
    result: QueryResult.Tree,
    modifier: Modifier = Modifier
) {
    val colors = reportSemanticColors()
    if (!result.found || result.nodes.isEmpty()) {
        ElevatedCard(modifier = modifier.fillMaxWidth()) {
            Column(
                modifier = Modifier.padding(16.dp),
                verticalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                Text(
                    text = result.message,
                    style = MaterialTheme.typography.bodyMedium,
                    modifier = Modifier.fillMaxWidth()
                )
                if (result.roots.isNotEmpty()) {
                    for (root in result.roots) {
                        Text(
                            text = root,
                            style = MaterialTheme.typography.bodySmall,
                            modifier = Modifier.fillMaxWidth()
                        )
                    }
                }
            }
        }
        return
    }

    var sortDescending by remember(result) { mutableStateOf(true) }
    val sortedRoots = remember(result.nodes, sortDescending) {
        result.nodes.sortedByDuration(descending = sortDescending)
    }
    val totalTreeDurationSeconds = remember(result.nodes) {
        result.nodes.sumOf { it.durationSeconds ?: 0L }
    }
    val horizontalScrollState = rememberScrollState()
    val treeContentMinWidth = 320.dp +
        (result.maxAvailableDepth.coerceAtLeast(0) * 28).dp

    ElevatedCard(
        modifier = modifier.fillMaxWidth(),
        colors = CardDefaults.elevatedCardColors(
            containerColor = MaterialTheme.colorScheme.surfaceContainerLow
        )
    ) {
        BoxWithConstraints(
            modifier = Modifier.fillMaxWidth()
        ) {
            val treeContentWidth = maxOf(maxWidth, treeContentMinWidth)
            Box(
                modifier = Modifier
                    .fillMaxWidth()
                    .horizontalScroll(horizontalScrollState)
            ) {
                Column(
                    modifier = Modifier
                        .width(treeContentWidth)
                        .padding(8.dp),
                    verticalArrangement = Arrangement.spacedBy(6.dp)
                ) {
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.End
                    ) {
                        AssistChip(
                            onClick = { sortDescending = !sortDescending },
                            label = {
                                Text(
                                    text = stringResource(
                                        if (sortDescending) {
                                            R.string.report_tree_sort_duration_desc
                                        } else {
                                            R.string.report_tree_sort_duration_asc
                                        }
                                    )
                                )
                            },
                            leadingIcon = {
                                Icon(
                                    imageVector = if (sortDescending) {
                                        Icons.Default.ArrowDownward
                                    } else {
                                        Icons.Default.ArrowUpward
                                    },
                                    contentDescription = null
                                )
                            }
                        )
                    }
                    for ((index, node) in sortedRoots.withIndex()) {
                        TreeResultNodeItem(
                            node = node,
                            depth = 0,
                            nodeKey = buildNodeKey(parentKey = "root_$index", node = node),
                            treeRootPath = treeNodePath(node),
                            totalTreeDurationSeconds = totalTreeDurationSeconds,
                            period = result.period,
                            sortDescending = sortDescending,
                            colors = colors
                        )
                    }
                }
            }
        }
    }
}

@Composable
private fun TreeResultNodeItem(
    node: TreeNode,
    depth: Int,
    nodeKey: String,
    treeRootPath: String,
    totalTreeDurationSeconds: Long,
    period: DataTreePeriod,
    sortDescending: Boolean,
    colors: ReportSemanticColors
) {
    val hasChildren = node.children.isNotEmpty()
    val isTopLevelActivity = depth == 0
    var expanded by remember(nodeKey) { mutableStateOf(true) }
    val sortedChildren = remember(node.children, sortDescending) {
        node.children.sortedByDuration(descending = sortDescending)
    }
    val indent = (depth * 14).dp
    Surface(
        modifier = Modifier
            .fillMaxWidth()
            .padding(start = indent),
        shape = MaterialTheme.shapes.medium,
        border = BorderStroke(
            width = 1.dp,
            color = MaterialTheme.colorScheme.outlineVariant
        ),
        color = if (depth == 0) {
            MaterialTheme.colorScheme.surfaceContainerHigh
        } else {
            MaterialTheme.colorScheme.surfaceContainer
        },
        tonalElevation = if (depth == 0) 1.dp else 0.dp
    ) {
        Column {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .clickable(enabled = hasChildren) { expanded = !expanded }
                    .padding(horizontal = 14.dp, vertical = 12.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                if (hasChildren) {
                    Icon(
                        imageVector = if (expanded) {
                            Icons.Default.ExpandMore
                        } else {
                            Icons.Default.ChevronRight
                        },
                        contentDescription = null,
                        tint = if (isTopLevelActivity) colors.root else colors.child,
                        modifier = Modifier.size(22.dp)
                    )
                } else {
                    Spacer(modifier = Modifier.width(22.dp))
                }
                Spacer(modifier = Modifier.width(10.dp))
                Column(modifier = Modifier.weight(1f)) {
                    Text(
                        text = node.name,
                        style = if (isTopLevelActivity) {
                            MaterialTheme.typography.titleSmall
                        } else {
                            MaterialTheme.typography.bodyMedium
                        },
                        color = if (isTopLevelActivity) {
                            colors.root
                        } else {
                            MaterialTheme.colorScheme.onSurface
                        },
                        maxLines = 1
                    )
                    formatTreeCanonical(node, treeRootPath)?.let { path ->
                        Text(
                            text = path,
                            style = MaterialTheme.typography.labelSmall,
                            color = MaterialTheme.colorScheme.onSurfaceVariant,
                            maxLines = 1,
                            modifier = Modifier.alpha(0.8f)
                        )
                    }
                }
                node.durationSeconds?.let { duration ->
                    Spacer(modifier = Modifier.width(8.dp))
                    Text(
                        text = formatTreeDuration(duration, period),
                        style = MaterialTheme.typography.labelMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }

            treeNodeDurationPercent(
                node = node,
                depth = depth,
                totalTreeDurationSeconds = totalTreeDurationSeconds
            )?.let { percent ->
                Column(
                    modifier = Modifier.padding(start = 66.dp, end = 14.dp, bottom = 12.dp),
                    verticalArrangement = Arrangement.spacedBy(4.dp)
                ) {
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.SpaceBetween
                    ) {
                        Text(
                            text = formatTreeParentDurationPercent(percent),
                            style = MaterialTheme.typography.labelSmall,
                            color = colors.treeProgressAccent
                        )
                    }
                    LinearProgressIndicator(
                        color = colors.treeProgressAccent,
                        progress = { percent.coerceIn(0f, 100f) / 100f },
                        modifier = Modifier.fillMaxWidth()
                    )
                }
            }

            if (hasChildren && expanded) {
                Row(
                    modifier = Modifier.padding(start = 10.dp, end = 10.dp, bottom = 10.dp),
                    verticalAlignment = Alignment.Top
                ) {
                    Box(
                        modifier = Modifier
                            .width(2.dp)
                            .fillMaxHeight()
                            .background(MaterialTheme.colorScheme.outlineVariant)
                    )
                    Column(
                        modifier = Modifier
                            .weight(1f)
                            .padding(start = 8.dp),
                        verticalArrangement = Arrangement.spacedBy(8.dp)
                    ) {
                        for ((index, child) in sortedChildren.withIndex()) {
                            TreeResultNodeItem(
                                node = child,
                                depth = depth + 1,
                                nodeKey = buildNodeKey(
                                    parentKey = "$nodeKey/$index",
                                    node = child
                                ),
                                treeRootPath = treeRootPath,
                                totalTreeDurationSeconds = totalTreeDurationSeconds,
                                period = period,
                                sortDescending = sortDescending,
                                colors = colors
                            )
                        }
                    }
                }
            }
        }
    }
}

private fun List<TreeNode>.sortedByDuration(descending: Boolean): List<TreeNode> =
    if (descending) {
        sortedWith(
            compareByDescending<TreeNode> { it.durationSeconds ?: 0L }
                .thenBy { it.name }
        )
    } else {
        sortedWith(
            compareBy<TreeNode> { it.durationSeconds ?: 0L }
                .thenBy { it.name }
        )
    }

private fun buildNodeKey(parentKey: String, node: TreeNode): String {
    val identity = if (node.path.isNotBlank()) node.path else node.name
    return "$parentKey:$identity"
}

private fun formatTreeDuration(
    durationSeconds: Long,
    period: DataTreePeriod
): String {
    val totalSeconds = durationSeconds.coerceAtLeast(0L)
    val secondsPerDay = 24L * 60L * 60L
    val secondsPerHour = 60L * 60L
    val secondsPerMinute = 60L

    if (period == DataTreePeriod.DAY) {
        return String.format(
            Locale.ROOT,
            "%02d:%02d:%02d",
            totalSeconds / secondsPerHour,
            (totalSeconds % secondsPerHour) / secondsPerMinute,
            totalSeconds % secondsPerMinute
        )
    }

    val days = totalSeconds / secondsPerDay
    val hours = (totalSeconds % secondsPerDay) / secondsPerHour
    val minutes = (totalSeconds % secondsPerHour) / secondsPerMinute
    return if (days > 0L) {
        String.format(Locale.ROOT, "%dd %02d:%02d", days, hours, minutes)
    } else {
        String.format(Locale.ROOT, "%02d:%02d", hours, minutes)
    }
}

private fun treeNodePath(node: TreeNode): String =
    node.path.ifBlank { node.name }

private fun treeNodeDurationPercent(
    node: TreeNode,
    depth: Int,
    totalTreeDurationSeconds: Long
): Float? = if (depth == 0) {
    val durationSeconds = node.durationSeconds ?: return null
    if (totalTreeDurationSeconds <= 0L) {
        null
    } else {
        (durationSeconds.toDouble() * 100.0 / totalTreeDurationSeconds)
            .toFloat()
            .coerceIn(0f, 100f)
    }
} else {
    node.parentDurationPercent
}

private fun formatTreeCanonical(node: TreeNode, treeRootPath: String): String? {
    val path = treeNodePath(node)
    val relativePath = when {
        path == treeRootPath -> ""
        path.startsWith("${treeRootPath}_") -> path.removePrefix("${treeRootPath}_")
        else -> path.substringAfter('_', missingDelimiterValue = "")
    }
    return relativePath
        .takeIf { it.isNotBlank() }
        ?.replace("_", " > ")
}

private fun formatTreeParentDurationPercent(percent: Float): String =
    String.format(Locale.ROOT, "%.1f%%", percent.coerceAtLeast(0f))
