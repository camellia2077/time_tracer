package com.example.tracer

import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.unit.dp
import java.util.Locale

@Composable
internal fun QueryReportTreeResultContent(
    result: QueryResult.Tree,
    modifier: Modifier = Modifier
) {
    if (!result.found || result.nodes.isEmpty()) {
        Column(modifier = modifier) {
            Text(
                text = result.message,
                style = MaterialTheme.typography.bodyMedium,
                modifier = Modifier.fillMaxWidth()
            )
            if (result.roots.isNotEmpty()) {
                for (root in result.roots) {
                    Text(
                        text = "• $root",
                        style = MaterialTheme.typography.bodySmall,
                        fontFamily = FontFamily.Monospace,
                        modifier = Modifier.fillMaxWidth()
                    )
                }
            }
        }
        return
    }

    Column(modifier = modifier) {
        for ((index, node) in result.nodes.withIndex()) {
            TreeResultNodeItem(
                node = node,
                depth = 0,
                nodeKey = buildNodeKey(parentKey = "root_$index", node = node)
            )
        }
    }
}

@Composable
private fun TreeResultNodeItem(
    node: TreeNode,
    depth: Int,
    nodeKey: String
) {
    val hasChildren = node.children.isNotEmpty()
    var expanded by remember(nodeKey) { mutableStateOf(true) }
    val indent = (depth * 16).dp
    val details = buildList {
        node.durationSeconds?.let { add(formatTreeDurationHoursMinutes(it)) }
        node.parentDurationPercent?.let { add(formatTreeParentDurationPercent(it)) }
    }
    val detailSuffix = details.takeIf { it.isNotEmpty() }
        ?.joinToString(separator = " · ", prefix = " [", postfix = "]")
        .orEmpty()
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(start = indent)
            .clickable(enabled = hasChildren) { expanded = !expanded },
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(
            text = if (hasChildren) {
                if (expanded) "▼" else "▶"
            } else {
                "•"
            },
            style = MaterialTheme.typography.bodySmall
        )
        Spacer(modifier = Modifier.width(6.dp))
        Text(
            text = node.name + detailSuffix,
            style = MaterialTheme.typography.bodyMedium,
            modifier = Modifier.fillMaxWidth()
        )
    }

    if (hasChildren && expanded) {
        for ((index, child) in node.children.withIndex()) {
            TreeResultNodeItem(
                node = child,
                depth = depth + 1,
                nodeKey = buildNodeKey(
                    parentKey = "$nodeKey/$index",
                    node = child
                )
            )
        }
    }
}

private fun buildNodeKey(parentKey: String, node: TreeNode): String {
    val identity = if (node.path.isNotBlank()) node.path else node.name
    return "$parentKey:$identity"
}

private fun formatTreeDurationHoursMinutes(durationSeconds: Long): String {
    val totalMinutes = durationSeconds.coerceAtLeast(0L) / 60L
    return String.format(
        Locale.ROOT,
        "%02d:%02d",
        totalMinutes / 60L,
        totalMinutes % 60L
    )
}

private fun formatTreeParentDurationPercent(percent: Float): String =
    String.format(Locale.ROOT, "%.1f%%", percent.coerceAtLeast(0f))
