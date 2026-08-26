package com.example.tracer

import android.util.Log
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.HorizontalDivider
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
    slices: List<InsightsCompositionSlice>,
    colors: List<androidx.compose.ui.graphics.Color>,
    parentPath: List<String>,
    nodes: List<TreeNode>,
    showAverage: Boolean,
    showFrequency: Boolean,
    onSliceSelected: (Int) -> Unit
) {
    Column(
        modifier = Modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(0.dp)
    ) {
        slices.forEachIndexed { index, slice ->
            val hasChildren = nodes.firstOrNull { it.name == slice.root }
                ?.children
                ?.isNotEmpty() == true
            CompositionLegendRow(
                slice = slice,
                showAverage = showAverage,
                showFrequency = showFrequency,
                label = (parentPath + slice.root).joinToString(" › "),
                color = colors.getOrElse(index) { androidx.compose.ui.graphics.Color.Gray },
                hasChildren = hasChildren,
                onClick = { onSliceSelected(index) }
            )
            if (index < slices.lastIndex) {
                HorizontalDivider(
                    color = MaterialTheme.colorScheme.outline.copy(alpha = 0.6f)
                )
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

internal fun List<TreeNode>.toInsightsCompositionSlices(
    compositionMeasure: InsightsCompositionMeasure = InsightsCompositionMeasure.DURATION
): List<InsightsCompositionSlice> {
    val totalValue = sumOf { node ->
        when (compositionMeasure) {
            InsightsCompositionMeasure.DURATION -> node.durationSeconds ?: 0L
            InsightsCompositionMeasure.FREQUENCY -> node.occurrenceCount ?: 0L
        }
    }.coerceAtLeast(0L)
    return mapNotNull { node ->
        val value = when (compositionMeasure) {
            InsightsCompositionMeasure.DURATION -> node.durationSeconds
            InsightsCompositionMeasure.FREQUENCY -> node.occurrenceCount
        }?.coerceAtLeast(0L) ?: return@mapNotNull null
        if (node.name.isBlank() || value <= 0L) {
            return@mapNotNull null
        }
        InsightsCompositionSlice(
            root = node.name,
            durationSeconds = value,
            percent = if (totalValue > 0L) {
                value.toFloat() * 100f / totalValue.toFloat()
            } else {
                0f
            },
            totalDurationSeconds = node.durationSeconds,
            occurrenceCount = node.occurrenceCount,
            averageDurationSeconds = node.averageDurationSeconds,
            averageDurationPerOccurrenceSeconds =
                node.averageDurationPerOccurrenceSeconds,
            averageOccurrenceCount = node.averageOccurrenceCount,
            averageOccurrenceRatio = node.averageOccurrenceRatio
        )
    }.sortedWith(
        compareByDescending<InsightsCompositionSlice> { it.durationSeconds }
            .thenBy { it.root }
    )
}

