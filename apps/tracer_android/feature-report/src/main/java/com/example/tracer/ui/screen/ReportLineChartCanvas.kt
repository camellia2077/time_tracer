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
import androidx.compose.ui.graphics.Path
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.graphics.StrokeJoin
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.layout.onSizeChanged
import androidx.compose.ui.unit.IntSize
import androidx.compose.ui.unit.dp

@Composable
internal fun ReportLineChart(
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

    val lineColor = MaterialTheme.colorScheme.primary
    val pointColor = MaterialTheme.colorScheme.primary
    val gridColor = MaterialTheme.colorScheme.outlineVariant
    val averageLineColor = MaterialTheme.colorScheme.tertiary
    val selectedGuideColor = MaterialTheme.colorScheme.secondary
    val selectedPointColor = MaterialTheme.colorScheme.tertiary

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
                    val plot = buildChartPlot(
                        durationHours = durationHours,
                        size = Size(
                            canvasSize.width.toFloat(),
                            canvasSize.height.toFloat()
                        )
                    )
                    val nearestIndex = plot.offsets.indices.minByOrNull { index ->
                        val dx = plot.offsets[index].x - tapOffset.x
                        val dy = plot.offsets[index].y - tapOffset.y
                        (dx * dx) + (dy * dy)
                    } ?: return@detectTapGestures

                    val selectedOffset = plot.offsets[nearestIndex]
                    val dx = selectedOffset.x - tapOffset.x
                    val dy = selectedOffset.y - tapOffset.y
                    val hitRadiusPx = 24.dp.toPx()
                    if ((dx * dx) + (dy * dy) <= hitRadiusPx * hitRadiusPx) {
                        onPointSelected(nearestIndex)
                    }
                }
            }
    ) {
        if (durationHours.isEmpty()) {
            return@Canvas
        }
        val plot = buildChartPlot(durationHours = durationHours, size = size)

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

        val linePath = Path().apply {
            plot.offsets.forEachIndexed { index, offset ->
                if (index == 0) {
                    moveTo(offset.x, offset.y)
                } else {
                    lineTo(offset.x, offset.y)
                }
            }
        }
        drawPath(
            path = linePath,
            color = lineColor,
            style = Stroke(width = 3.5f, cap = StrokeCap.Round, join = StrokeJoin.Round)
        )

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

        if (selectedIndex in plot.offsets.indices) {
            val selectedOffset = plot.offsets[selectedIndex]
            drawLine(
                color = selectedGuideColor.copy(alpha = 0.35f),
                start = Offset(selectedOffset.x, plot.topPadding),
                end = Offset(selectedOffset.x, plot.topPadding + plot.chartHeight),
                strokeWidth = 1.5f
            )
            drawCircle(
                color = selectedGuideColor.copy(alpha = 0.2f),
                radius = 11f,
                center = selectedOffset
            )
        }

        plot.offsets.forEach { offset ->
            drawCircle(
                color = pointColor,
                radius = 4.5f,
                center = offset
            )
        }

        if (selectedIndex in plot.offsets.indices) {
            drawCircle(
                color = selectedPointColor,
                radius = 6.5f,
                center = plot.offsets[selectedIndex]
            )
        }
    }
}
