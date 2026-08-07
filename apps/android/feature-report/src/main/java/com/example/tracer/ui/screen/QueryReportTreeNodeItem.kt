
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
import androidx.compose.foundation.layout.widthIn
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
import androidx.compose.ui.unit.dp
import androidx.compose.ui.res.stringResource
import com.example.tracer.feature.report.R
import java.util.Locale

private val TreeNodeFixedIndent = 16.dp

@Composable
internal fun TreeResultNodeItem(
    node: TreeNode,
    depth: Int,
    nodeKey: String,
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
    Surface(
        modifier = Modifier
            .fillMaxWidth()
            .padding(start = TreeNodeFixedIndent),
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
                    Spacer(modifier = Modifier.width(10.dp))
                }
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
                }
            }

            val durationPercent = treeNodeDurationPercent(
                node = node,
                depth = depth,
                totalTreeDurationSeconds = totalTreeDurationSeconds
            )
            if (node.durationSeconds != null || durationPercent != null) {
                Column(
                    modifier = Modifier
                        .fillMaxWidth()
                        .widthIn(max = 280.dp)
                        .padding(start = 14.dp, end = 14.dp, bottom = 12.dp),
                    verticalArrangement = Arrangement.spacedBy(4.dp)
                ) {
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.SpaceBetween
                    ) {
                        node.durationSeconds?.let { duration ->
                            Text(
                                text = formatTreeDuration(duration, period),
                                style = MaterialTheme.typography.labelMedium,
                                color = MaterialTheme.colorScheme.onSurfaceVariant
                            )
                        }
                        durationPercent?.let { percent ->
                            Text(
                                text = formatTreeParentDurationPercent(percent),
                                style = MaterialTheme.typography.labelSmall,
                                color = colors.treeProgressAccent
                            )
                        }
                    }
                    durationPercent?.let { percent ->
                        LinearProgressIndicator(
                            color = colors.treeProgressAccent,
                            progress = { percent.coerceIn(0f, 100f) / 100f },
                            modifier = Modifier.fillMaxWidth()
                        )
                    }
                }
            }

            if (hasChildren && expanded) {
                Column(
                    modifier = Modifier.padding(end = 10.dp, bottom = 10.dp),
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


