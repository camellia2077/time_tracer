package com.example.tracer

import androidx.compose.foundation.Canvas
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.material3.MaterialTheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.PathEffect
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.layout.onSizeChanged
import androidx.compose.ui.unit.IntSize
import androidx.compose.ui.unit.dp

@Composable
internal fun ReportBarChart(
    points: List<ReportChartPoint>,
    selectedIndex: Int,
    averageDurationSeconds: Long?,
    usesLegacyStatsFallback: Boolean,
    showAverageLine: Boolean,
    onPointSelected: (Int) -> Unit,
    modifier: Modifier = Modifier
) {
    val durationHours = remember(points) {
        points.map { point -> point.durationSeconds.coerceAtLeast(0L) / 3600f }
    }
    var canvasSize by remember { mutableStateOf(IntSize.Zero) }

    val barColor = MaterialTheme.colorScheme.primary.copy(alpha = 0.75f)
    val gridColor = MaterialTheme.colorScheme.outlineVariant
    val averageLineColor = MaterialTheme.colorScheme.tertiary
    val selectedGuideColor = MaterialTheme.colorScheme.secondary
    val selectedBarColor = MaterialTheme.colorScheme.tertiary

    Canvas(
        modifier = modifier
            .onSizeChanged { canvasSize = it }
            .pointerInput(durationHours, canvasSize) {
                detectTapGestures { tapOffset ->
                    if (durationHours.isEmpty() ||
                        canvasSize.width == 0 ||
                        canvasSize.height == 0
                    ) {
                        return@detectTapGestures
                    }
                    val plot = buildBarChartPlot(
                        durationHours = durationHours,
                        size = Size(
                            canvasSize.width.toFloat(),
                            canvasSize.height.toFloat()
                        )
                    )
                    val nearestIndex = plot.centers.indices.minByOrNull { index ->
                        val dx = plot.centers[index].x - tapOffset.x
                        dx * dx
                    } ?: return@detectTapGestures

                    val selectedBar = plot.bars[nearestIndex]
                    val hitPadding = 10.dp.toPx()
                    val hitMinX = selectedBar.topLeft.x - hitPadding
                    val hitMaxX = selectedBar.topLeft.x + selectedBar.size.width + hitPadding
                    val hitMinY = plot.topPadding
                    val hitMaxY = plot.topPadding + plot.chartHeight
                    val isInsideHitArea = tapOffset.x in hitMinX..hitMaxX &&
                        tapOffset.y in hitMinY..hitMaxY
                    if (isInsideHitArea) {
                        onPointSelected(nearestIndex)
                    }
                }
            }
    ) {
        if (durationHours.isEmpty()) {
            return@Canvas
        }
        val plot = buildBarChartPlot(durationHours = durationHours, size = size)

        val gridLineCount = 4
        for (index in 0..gridLineCount) {
            val y = plot.topPadding + (plot.chartHeight * index / gridLineCount.toFloat())
            drawLine(
                color = gridColor,
                start = Offset(plot.leftPadding, y),
                end = Offset(plot.leftPadding + plot.chartWidth, y),
                strokeWidth = 1f
            )
        }

        if (showAverageLine) {
            // Backward-compat fallback for legacy payloads without core stats fields.
            // Planned removal: after one compatibility cycle (target Android v0.3.0).
            val averageHours = resolveAverageDurationHours(
                durationHours = durationHours,
                averageDurationSeconds = averageDurationSeconds,
                usesLegacyStatsFallback = usesLegacyStatsFallback
            )
            if (averageHours != null) {
                val averageY = resolveAverageLineY(
                    averageHours = averageHours,
                    durationHours = durationHours,
                    topPadding = plot.topPadding,
                    chartHeight = plot.chartHeight
                )
                drawLine(
                    color = averageLineColor,
                    start = Offset(plot.leftPadding, averageY),
                    end = Offset(plot.leftPadding + plot.chartWidth, averageY),
                    strokeWidth = 2f,
                    pathEffect = PathEffect.dashPathEffect(floatArrayOf(14f, 8f), 0f)
                )
            }
        }

        if (selectedIndex in plot.centers.indices) {
            val selectedCenter = plot.centers[selectedIndex]
            drawLine(
                color = selectedGuideColor.copy(alpha = 0.35f),
                start = Offset(selectedCenter.x, plot.topPadding),
                end = Offset(selectedCenter.x, plot.topPadding + plot.chartHeight),
                strokeWidth = 1.5f
            )
        }

        plot.bars.forEachIndexed { index, bar ->
            drawRect(
                color = if (index == selectedIndex) selectedBarColor else barColor,
                topLeft = bar.topLeft,
                size = bar.size
            )
        }
    }
}
