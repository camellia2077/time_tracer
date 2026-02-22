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
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.layout.onSizeChanged
import androidx.compose.ui.unit.IntSize
import androidx.compose.ui.unit.dp
import kotlin.math.cos
import kotlin.math.sin

@Composable
internal fun ReportPieChart(
    points: List<ReportChartPoint>,
    selectedIndex: Int,
    onPointSelected: (Int) -> Unit,
    modifier: Modifier = Modifier
) {
    val sortedPoints = remember(points) { points.sortedBy { it.date } }
    val durationHours = remember(sortedPoints) {
        sortedPoints.map { point -> point.durationSeconds.coerceAtLeast(0L) / 3600f }
    }
    var canvasSize by remember { mutableStateOf(IntSize.Zero) }

    val selectedOutlineColor = MaterialTheme.colorScheme.surface
    val slicePalette = listOf(
        MaterialTheme.colorScheme.primary,
        MaterialTheme.colorScheme.secondary,
        MaterialTheme.colorScheme.tertiary,
        MaterialTheme.colorScheme.primaryContainer,
        MaterialTheme.colorScheme.secondaryContainer,
        MaterialTheme.colorScheme.tertiaryContainer,
        MaterialTheme.colorScheme.errorContainer,
        MaterialTheme.colorScheme.inversePrimary
    )

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
                    val plot = buildPieChartPlot(
                        durationHours = durationHours,
                        size = Size(
                            canvasSize.width.toFloat(),
                            canvasSize.height.toFloat()
                        )
                    )
                    val selectedSlice = findPieSliceIndex(plot = plot, tapOffset = tapOffset)
                    if (selectedSlice >= 0) {
                        onPointSelected(selectedSlice)
                    }
                }
            }
    ) {
        if (durationHours.isEmpty()) {
            return@Canvas
        }
        val plot = buildPieChartPlot(durationHours = durationHours, size = size)
        val diameter = plot.radius * 2f

        plot.slices.forEachIndexed { index, slice ->
            if (slice.sweepAngle <= 0f) {
                return@forEachIndexed
            }
            val isSelected = index == selectedIndex
            val explodeDistance = if (isSelected) 8.dp.toPx() else 0f
            val midAngleRad = Math.toRadians((slice.startAngle + slice.sweepAngle / 2f).toDouble())
            val explodeOffset = Offset(
                x = cos(midAngleRad).toFloat() * explodeDistance,
                y = sin(midAngleRad).toFloat() * explodeDistance
            )
            val topLeft = Offset(
                x = plot.center.x - plot.radius + explodeOffset.x,
                y = plot.center.y - plot.radius + explodeOffset.y
            )
            val drawSize = Size(diameter, diameter)
            val sliceColor = slicePalette[index % slicePalette.size]

            drawArc(
                color = sliceColor,
                startAngle = slice.startAngle,
                sweepAngle = slice.sweepAngle,
                useCenter = true,
                topLeft = topLeft,
                size = drawSize
            )
            if (isSelected) {
                drawArc(
                    color = selectedOutlineColor,
                    startAngle = slice.startAngle,
                    sweepAngle = slice.sweepAngle,
                    useCenter = true,
                    topLeft = topLeft,
                    size = drawSize,
                    style = Stroke(width = 2.5f)
                )
            }
        }
    }
}
