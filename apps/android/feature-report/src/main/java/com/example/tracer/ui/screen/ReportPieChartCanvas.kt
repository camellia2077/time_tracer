package com.example.tracer

import androidx.compose.foundation.Canvas
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.isSystemInDarkTheme
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
fun ReportPieChart(
    slices: List<ReportCompositionSlice>,
    palettePreset: ReportPiePalettePreset,
    selectedIndex: Int,
    onSliceSelected: (Int) -> Unit,
    modifier: Modifier = Modifier
) {
    val durationHours = remember(slices) {
        slices.map { slice -> slice.durationSeconds.coerceAtLeast(0L) / 3600f }
    }
    var canvasSize by remember { mutableStateOf(IntSize.Zero) }
    val isDarkTheme = isSystemInDarkTheme()
    val sliceColors = rememberPieSliceColors(slices, palettePreset)
    val sliceOutlineColor = if (isDarkTheme) {
        androidx.compose.ui.graphics.Color.White.copy(alpha = 0.32f)
    } else {
        androidx.compose.ui.graphics.Color.Black.copy(alpha = 0.18f)
    }
    val selectedOutlineColor = if (isDarkTheme) {
        androidx.compose.ui.graphics.Color.White.copy(alpha = 0.88f)
    } else {
        androidx.compose.ui.graphics.Color.Black.copy(alpha = 0.72f)
    }

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
                        onSliceSelected(selectedSlice)
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
            val sliceColor = sliceColors.getOrElse(index) {
                resolvePieSliceColor(slices[index], palettePreset)
            }

            drawArc(
                color = sliceColor,
                startAngle = slice.startAngle,
                sweepAngle = slice.sweepAngle,
                useCenter = true,
                topLeft = topLeft,
                size = drawSize
            )
            drawArc(
                color = sliceOutlineColor,
                startAngle = slice.startAngle,
                sweepAngle = slice.sweepAngle,
                useCenter = true,
                topLeft = topLeft,
                size = drawSize,
                style = Stroke(width = 1.25f)
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
