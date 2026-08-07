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
    val treeContentMinWidth = remember(result.nodes) {
        treeContentMinWidthForDepth(result.nodes.maxTreeDepth())
    }

    ElevatedCard(
        modifier = modifier.fillMaxWidth(),
        colors = CardDefaults.elevatedCardColors(
            containerColor = MaterialTheme.colorScheme.surfaceContainerLow
        )
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(start = 8.dp, top = 8.dp, end = 8.dp),
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
                        .padding(
                            end = 8.dp,
                            bottom = 8.dp
                        ),
                    verticalArrangement = Arrangement.spacedBy(6.dp)
                ) {
                    for ((index, node) in sortedRoots.withIndex()) {
                        TreeResultNodeItem(
                            node = node,
                            depth = 0,
                            nodeKey = buildNodeKey(parentKey = "root_$index", node = node),
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
