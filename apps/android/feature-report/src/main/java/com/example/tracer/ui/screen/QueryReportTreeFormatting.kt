
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

internal fun List<TreeNode>.sortedByDuration(descending: Boolean): List<TreeNode> =
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

internal fun buildNodeKey(parentKey: String, node: TreeNode): String {
    val identity = if (node.path.isNotBlank()) node.path else node.name
    return "$parentKey:$identity"
}

internal fun formatTreeDuration(
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

internal fun treeNodePath(node: TreeNode): String =
    node.path.ifBlank { node.name }

internal fun treeNodeDurationPercent(
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

internal fun formatTreeCanonical(node: TreeNode, treeRootPath: String): String? {
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

internal fun formatTreeParentDurationPercent(percent: Float): String =
    String.format(Locale.ROOT, "%.1f%%", percent.coerceAtLeast(0f))

