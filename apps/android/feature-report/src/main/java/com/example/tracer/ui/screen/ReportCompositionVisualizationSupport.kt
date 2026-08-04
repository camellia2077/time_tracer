package com.example.tracer

import android.util.Log
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ChevronRight
import androidx.compose.material3.Icon
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
@Composable
internal fun CompositionSliceLegend(
    slices: List<ReportCompositionSlice>,
    colors: List<androidx.compose.ui.graphics.Color>,
    parentPath: List<String>,
    nodes: List<TreeNode>,
    valueLabel: (Long) -> String,
    showAverage: Boolean,
    showAverageRecords: Boolean,
    showFrequency: Boolean,
    onSliceSelected: (Int) -> Unit
) {
    Column(
        modifier = Modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(6.dp)
    ) {
        slices.forEachIndexed { index, slice ->
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .heightIn(min = 48.dp)
                    .clickable { onSliceSelected(index) }
                    .padding(horizontal = 4.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Box(
                    modifier = Modifier
                        .size(10.dp)
                        .background(
                            color = colors.getOrElse(index) { androidx.compose.ui.graphics.Color.Gray },
                            shape = CircleShape
                        )
                )
                CompositionLegendRow(
                    slice = slice,
                    showAverage = showAverage,
                    showFrequency = showFrequency,
                    label = (parentPath + slice.root).joinToString(" › "),
                    modifier = Modifier.weight(1f).padding(start = 8.dp)
                )
                if (nodes.firstOrNull { it.name == slice.root }?.children?.isNotEmpty() == true) {
                    Icon(
                        imageVector = Icons.Filled.ChevronRight,
                        contentDescription = null,
                        modifier = Modifier.padding(start = 4.dp),
                        tint = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }
        }
    }
}

internal fun resolveCompositionDrilldownNodes(
    tree: List<TreeNode>,
    path: List<String>
): List<TreeNode> {
    var currentLevel = tree
    for (segment in path) {
        val selected = currentLevel.firstOrNull { it.name == segment } ?: return tree
        currentLevel = selected.children
    }
    return currentLevel
}

internal fun List<TreeNode>.toReportCompositionSlices(
    compositionMeasure: ReportCompositionMeasure = ReportCompositionMeasure.DURATION,
    averageDenominatorDays: Int = 0
): List<ReportCompositionSlice> {
    val totalValue = sumOf { node ->
        when (compositionMeasure) {
            ReportCompositionMeasure.DURATION -> node.durationSeconds ?: 0L
            ReportCompositionMeasure.FREQUENCY -> node.occurrenceCount ?: 0L
        }
    }.coerceAtLeast(0L)
    return mapNotNull { node ->
        val value = when (compositionMeasure) {
            ReportCompositionMeasure.DURATION -> node.durationSeconds
            ReportCompositionMeasure.FREQUENCY -> node.occurrenceCount
        }?.coerceAtLeast(0L) ?: return@mapNotNull null
        if (node.name.isBlank() || value <= 0L) {
            return@mapNotNull null
        }
        ReportCompositionSlice(
            root = node.name,
            durationSeconds = value,
            percent = if (totalValue > 0L) {
                value.toFloat() * 100f / totalValue.toFloat()
            } else {
                0f
            },
            totalDurationSeconds = node.durationSeconds,
            occurrenceCount = node.occurrenceCount,
            averageDurationSeconds = node.averageDurationSeconds ?:
                if (averageDenominatorDays > 0) {
                    (node.durationSeconds ?: 0L) / averageDenominatorDays
                } else {
                    null
                },
            averageOccurrenceCount = node.averageOccurrenceCount,
            averageOccurrenceRatio = node.averageOccurrenceRatio
        )
    }.sortedWith(
        compareByDescending<ReportCompositionSlice> { it.durationSeconds }
            .thenBy { it.root }
    )
}

