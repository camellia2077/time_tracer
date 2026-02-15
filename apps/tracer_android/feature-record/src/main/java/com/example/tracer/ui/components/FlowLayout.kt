package com.example.tracer.ui.components

import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.layout.Layout
import androidx.compose.ui.layout.Placeable
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import kotlin.math.max

@Composable
fun SimpleFlowRow(
    modifier: Modifier = Modifier,
    horizontalGap: Dp = 0.dp,
    verticalGap: Dp = 0.dp,
    alignment: androidx.compose.ui.Alignment.Horizontal = androidx.compose.ui.Alignment.Start,
    content: @Composable () -> Unit
) {
    Layout(
        content = content,
        modifier = modifier
    ) { measurables, constraints ->
        val hGapPx = horizontalGap.roundToPx()
        val vGapPx = verticalGap.roundToPx()

        val rows = mutableListOf<List<Placeable>>()
        val rowWidths = mutableListOf<Int>()
        val rowHeights = mutableListOf<Int>()

        var currentRow = mutableListOf<Placeable>()
        var currentRowWidth = 0
        var currentRowHeight = 0

        for (measurable in measurables) {
            val placeable = measurable.measure(constraints.copy(minWidth = 0))
            if (currentRowWidth + placeable.width > constraints.maxWidth && currentRow.isNotEmpty()) {
                rows.add(currentRow)
                rowWidths.add(currentRowWidth)
                rowHeights.add(currentRowHeight)
                currentRow = mutableListOf()
                currentRowWidth = 0
                currentRowHeight = 0
            }
            if (currentRow.isNotEmpty()) {
                currentRowWidth += hGapPx
            }
            currentRow.add(placeable)
            currentRowWidth += placeable.width
            currentRowHeight = max(currentRowHeight, placeable.height)
        }

        if (currentRow.isNotEmpty()) {
            rows.add(currentRow)
            rowWidths.add(currentRowWidth)
            rowHeights.add(currentRowHeight)
        }

        val totalHeight = rowHeights.sum() + (max(0, rows.size - 1) * vGapPx)

        layout(width = constraints.maxWidth, height = totalHeight) {
            var yOffset = 0
            for (i in rows.indices) {
                val row = rows[i]
                val rowHeight = rowHeights[i]
                val rowWidth = rowWidths[i]
                var xOffset = when (alignment) {
                    androidx.compose.ui.Alignment.Start -> 0
                    androidx.compose.ui.Alignment.CenterHorizontally -> (constraints.maxWidth - rowWidth) / 2
                    androidx.compose.ui.Alignment.End -> constraints.maxWidth - rowWidth
                    else -> 0
                }

                for (placeable in row) {
                    placeable.placeRelative(x = xOffset, y = yOffset)
                    xOffset += placeable.width + hGapPx
                }
                yOffset += rowHeight + vGapPx
            }
        }
    }
}
