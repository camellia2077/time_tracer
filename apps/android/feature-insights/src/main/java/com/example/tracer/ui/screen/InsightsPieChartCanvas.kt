package com.example.tracer

import android.graphics.Color as AndroidColor
import android.graphics.Paint
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
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.drawscope.DrawScope
import androidx.compose.ui.graphics.nativeCanvas
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.layout.onSizeChanged
import androidx.compose.ui.unit.IntSize
import androidx.compose.ui.unit.dp
import kotlin.math.cos
import kotlin.math.sin

private const val MIN_INLINE_PIE_LABEL_SWEEP_ANGLE = 43.2f
private const val MAX_INLINE_PIE_LABEL_SLICE_COUNT = 8

@Composable
fun InsightsPieChart(
    slices: List<InsightsCompositionSlice>,
    palettePreset: InsightsPiePalettePreset,
    selectedIndex: Int,
    onSliceSelected: (Int) -> Unit,
    sliceColors: List<Color>? = null,
    modifier: Modifier = Modifier
) {
    val durationHours = remember(slices) {
        slices.map { slice -> slice.measureValue.coerceAtLeast(0L) / 3600f }
    }
    var canvasSize by remember { mutableStateOf(IntSize.Zero) }
    val isDarkTheme = isSystemInDarkTheme()
    val defaultSliceColors = rememberPieSliceColors(slices, palettePreset)
    val resolvedSliceColors = sliceColors ?: defaultSliceColors
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
        plot.slices.forEachIndexed { index, slice ->
            if (slice.sweepAngle <= 0f) {
                return@forEachIndexed
            }
            val isSelected = index == selectedIndex
            val topLeft = Offset(
                x = plot.center.x - plot.radius,
                y = plot.center.y - plot.radius
            )
            val drawSize = Size(plot.radius * 2f, plot.radius * 2f)
            val sliceColor = resolvedSliceColors.getOrElse(index) {
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
            drawPieSliceLabel(
                label = slices[index].root,
                slice = slice,
                sliceCount = plot.slices.size,
                center = plot.center,
                radius = plot.radius,
                fillColor = sliceColor
            )
        }
    }
}

private fun DrawScope.drawPieSliceLabel(
    label: String,
    slice: PieSlice,
    sliceCount: Int,
    center: Offset,
    radius: Float,
    fillColor: Color
) {
    if (!shouldDrawInlinePieLabel(slice.sweepAngle, sliceCount) || label.isBlank()) {
        return
    }
    val displayLabel = label.trim().let {
        if (it.length <= 10) it else "${it.take(9)}…"
    }
    val midAngleRad = Math.toRadians((slice.startAngle + slice.sweepAngle / 2f).toDouble())
    val labelOffset = Offset(
        x = cos(midAngleRad).toFloat() * radius * 0.62f,
        y = sin(midAngleRad).toFloat() * radius * 0.62f
    )
    val lightness = (0.2126f * AndroidColor.red(fillColor.toArgb()) +
        0.7152f * AndroidColor.green(fillColor.toArgb()) +
        0.0722f * AndroidColor.blue(fillColor.toArgb())) / 255f
    val textPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = if (lightness > 0.64f) AndroidColor.BLACK else AndroidColor.WHITE
        textSize = 11.dp.toPx()
        textAlign = Paint.Align.CENTER
    }
    val baseline = center.y + labelOffset.y - (textPaint.ascent() + textPaint.descent()) / 2f
    drawContext.canvas.nativeCanvas.drawText(
        displayLabel,
        center.x + labelOffset.x,
        baseline,
        textPaint
    )
}

internal fun shouldDrawInlinePieLabel(sweepAngle: Float, sliceCount: Int): Boolean =
    sliceCount <= MAX_INLINE_PIE_LABEL_SLICE_COUNT &&
        sweepAngle >= MIN_INLINE_PIE_LABEL_SWEEP_ANGLE
