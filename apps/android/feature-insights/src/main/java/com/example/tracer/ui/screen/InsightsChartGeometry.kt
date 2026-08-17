package com.example.tracer

import android.graphics.Paint
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.nativeCanvas
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.graphics.drawscope.DrawScope
import androidx.compose.ui.unit.dp
import java.util.Locale
import kotlin.math.atan2
import kotlin.math.sqrt

private const val CHART_GRID_LINE_COUNT = 4

internal fun DrawScope.drawDurationYAxisLabels(
    maxDurationHours: Float,
    leftPadding: Float,
    topPadding: Float,
    chartHeight: Float,
    labelColor: Color
) {
    val textPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = labelColor.toArgb()
        textSize = 10.dp.toPx()
        textAlign = Paint.Align.RIGHT
    }
    val labelX = leftPadding - 8.dp.toPx()
    val verticalCenterOffset = -(textPaint.ascent() + textPaint.descent()) / 2f
    val safeMaxDurationHours = maxDurationHours.coerceAtLeast(1f)

    for (index in 0..CHART_GRID_LINE_COUNT) {
        val y = topPadding + (chartHeight * index / CHART_GRID_LINE_COUNT.toFloat())
        val durationHours = (
            safeMaxDurationHours *
                (CHART_GRID_LINE_COUNT - index) /
                CHART_GRID_LINE_COUNT.toFloat()
            )
        drawContext.canvas.nativeCanvas.drawText(
            String.format(Locale.ROOT, "%.1f", durationHours),
            labelX,
            y + verticalCenterOffset,
            textPaint
        )
    }
}

internal data class ChartPlot(
    val offsets: List<Offset>,
    val leftPadding: Float,
    val topPadding: Float,
    val chartWidth: Float,
    val chartHeight: Float
)

internal data class BarColumn(
    val topLeft: Offset,
    val size: Size
)

internal data class BarChartPlot(
    val bars: List<BarColumn>,
    val centers: List<Offset>,
    val leftPadding: Float,
    val topPadding: Float,
    val chartWidth: Float,
    val chartHeight: Float
)

internal data class GroupedBarChartPlot(
    val currentBars: List<BarColumn>,
    val comparisonBars: List<BarColumn>,
    val currentCenters: List<Offset>,
    val leftPadding: Float,
    val topPadding: Float,
    val chartWidth: Float,
    val chartHeight: Float
)

internal data class PieSlice(
    val startAngle: Float,
    val sweepAngle: Float
)

internal data class PieChartPlot(
    val center: Offset,
    val radius: Float,
    val slices: List<PieSlice>
)

internal fun buildChartPlot(
    durationHours: List<Float>,
    size: Size,
    maxDurationHoursOverride: Float? = null
): ChartPlot {
    val leftPadding = 80f
    val rightPadding = 16f
    val topPadding = 16f
    val bottomPadding = 24f
    val chartWidth = (size.width - leftPadding - rightPadding).coerceAtLeast(1f)
    val chartHeight = (size.height - topPadding - bottomPadding).coerceAtLeast(1f)

    if (durationHours.isEmpty()) {
        return ChartPlot(
            offsets = emptyList(),
            leftPadding = leftPadding,
            topPadding = topPadding,
            chartWidth = chartWidth,
            chartHeight = chartHeight
        )
    }

    val maxY = (maxDurationHoursOverride ?: durationHours.maxOrNull())
        ?.coerceAtLeast(1f) ?: 1f
    val offsets = durationHours.mapIndexed { index, value ->
        val x = if (durationHours.size == 1) {
            leftPadding + chartWidth / 2f
        } else {
            leftPadding + (chartWidth * index / (durationHours.size - 1).toFloat())
        }
        val normalized = (value / maxY).coerceIn(0f, 1f)
        val y = topPadding + chartHeight * (1f - normalized)
        Offset(x, y)
    }
    return ChartPlot(
        offsets = offsets,
        leftPadding = leftPadding,
        topPadding = topPadding,
        chartWidth = chartWidth,
        chartHeight = chartHeight
    )
}

internal fun buildBarChartPlot(durationHours: List<Float>, size: Size): BarChartPlot {
    val leftPadding = 80f
    val rightPadding = 16f
    val topPadding = 16f
    val bottomPadding = 24f
    val chartWidth = (size.width - leftPadding - rightPadding).coerceAtLeast(1f)
    val chartHeight = (size.height - topPadding - bottomPadding).coerceAtLeast(1f)

    if (durationHours.isEmpty()) {
        return BarChartPlot(
            bars = emptyList(),
            centers = emptyList(),
            leftPadding = leftPadding,
            topPadding = topPadding,
            chartWidth = chartWidth,
            chartHeight = chartHeight
        )
    }

    val maxY = durationHours.maxOrNull()?.coerceAtLeast(1f) ?: 1f
    val slotWidth = chartWidth / durationHours.size.toFloat()
    val barWidth = (slotWidth * 0.7f).coerceAtLeast(2f).coerceAtMost(slotWidth * 0.9f)

    val bars = mutableListOf<BarColumn>()
    val centers = mutableListOf<Offset>()
    durationHours.forEachIndexed { index, value ->
        val normalized = (value / maxY).coerceIn(0f, 1f)
        val rawHeight = chartHeight * normalized
        val barHeight = if (value > 0f) rawHeight.coerceAtLeast(1f) else 0f
        val slotStart = leftPadding + slotWidth * index
        val centerX = slotStart + slotWidth / 2f
        val topY = topPadding + chartHeight - barHeight
        bars += BarColumn(
            topLeft = Offset(centerX - barWidth / 2f, topY),
            size = Size(barWidth, barHeight)
        )
        centers += Offset(centerX, topY)
    }

    return BarChartPlot(
        bars = bars,
        centers = centers,
        leftPadding = leftPadding,
        topPadding = topPadding,
        chartWidth = chartWidth,
        chartHeight = chartHeight
    )
}

internal fun buildPieChartPlot(durationHours: List<Float>, size: Size): PieChartPlot {
    val center = Offset(x = size.width / 2f, y = size.height / 2f)
    val radius = (minOf(size.width, size.height) * 0.38f).coerceAtLeast(1f)
    if (durationHours.isEmpty()) {
        return PieChartPlot(center = center, radius = radius, slices = emptyList())
    }

    val safeValues = durationHours.map { it.coerceAtLeast(0f) }
    val total = safeValues.sum()
    val slices = mutableListOf<PieSlice>()
    var accumulatedAngle = -90f

    safeValues.forEachIndexed { index, value ->
        val sweep = if (total > 0f) {
            (value / total) * 360f
        } else {
            if (index == safeValues.lastIndex) {
                360f - (accumulatedAngle + 90f)
            } else {
                360f / safeValues.size.toFloat()
            }
        }
        slices += PieSlice(
            startAngle = accumulatedAngle,
            sweepAngle = sweep.coerceAtLeast(0f)
        )
        accumulatedAngle += sweep
    }

    return PieChartPlot(
        center = center,
        radius = radius,
        slices = slices
    )
}

internal fun resolveAverageDurationHours(
    durationHours: List<Float>,
    averageDurationSeconds: Long
): Float? {
    if (durationHours.isEmpty()) {
        return null
    }
    val coreAverageHours = averageDurationSeconds
        .coerceAtLeast(0L)
        .toFloat()
        .div(3600f)
    return coreAverageHours
}

internal fun buildGroupedBarChartPlot(
    currentHours: List<Float>,
    comparisonHours: List<Float>,
    size: Size,
    maxDurationHours: Float
): GroupedBarChartPlot {
    val leftPadding = 80f
    val rightPadding = 16f
    val topPadding = 16f
    val bottomPadding = 24f
    val chartWidth = (size.width - leftPadding - rightPadding).coerceAtLeast(1f)
    val chartHeight = (size.height - topPadding - bottomPadding).coerceAtLeast(1f)
    val count = maxOf(currentHours.size, comparisonHours.size)
    if (count == 0) {
        return GroupedBarChartPlot(
            currentBars = emptyList(),
            comparisonBars = emptyList(),
            currentCenters = emptyList(),
            leftPadding = leftPadding,
            topPadding = topPadding,
            chartWidth = chartWidth,
            chartHeight = chartHeight
        )
    }

    val slotWidth = chartWidth / count.toFloat()
    val comparisonBarWidth = (slotWidth * 0.82f).coerceAtLeast(2f)
    val currentBarWidth = (slotWidth * 0.56f).coerceAtLeast(2f)
    val safeMax = maxDurationHours.coerceAtLeast(1f)
    val currentBars = mutableListOf<BarColumn>()
    val comparisonBars = mutableListOf<BarColumn>()
    val currentCenters = mutableListOf<Offset>()

    repeat(count) { index ->
        val currentValue = currentHours.getOrElse(index) { 0f }.coerceAtLeast(0f)
        val comparisonValue = comparisonHours.getOrElse(index) { 0f }.coerceAtLeast(0f)
        val currentHeight = if (currentValue > 0f) {
            chartHeight * (currentValue / safeMax).coerceIn(0f, 1f)
        } else {
            0f
        }
        val comparisonHeight = if (comparisonValue > 0f) {
            chartHeight * (comparisonValue / safeMax).coerceIn(0f, 1f)
        } else {
            0f
        }
        val currentTop = topPadding + chartHeight - currentHeight
        val comparisonTop = topPadding + chartHeight - comparisonHeight
        val slotCenter = leftPadding + slotWidth * index + slotWidth / 2f
        val currentLeft = slotCenter - currentBarWidth / 2f
        val comparisonLeft = slotCenter - comparisonBarWidth / 2f
        currentBars += BarColumn(
            topLeft = Offset(currentLeft, currentTop),
            size = Size(currentBarWidth, currentHeight)
        )
        comparisonBars += BarColumn(
            topLeft = Offset(comparisonLeft, comparisonTop),
            size = Size(comparisonBarWidth, comparisonHeight)
        )
        currentCenters += Offset(slotCenter, topPadding + chartHeight / 2f)
    }

    return GroupedBarChartPlot(
        currentBars = currentBars,
        comparisonBars = comparisonBars,
        currentCenters = currentCenters,
        leftPadding = leftPadding,
        topPadding = topPadding,
        chartWidth = chartWidth,
        chartHeight = chartHeight
    )
}

internal fun resolveAverageLineY(
    averageHours: Float,
    durationHours: List<Float>,
    topPadding: Float,
    chartHeight: Float
): Float {
    val maxY = durationHours.maxOrNull()?.coerceAtLeast(1f) ?: 1f
    val normalizedAverage = (averageHours / maxY).coerceIn(0f, 1f)
    return topPadding + chartHeight * (1f - normalizedAverage)
}

internal fun findPieSliceIndex(plot: PieChartPlot, tapOffset: Offset): Int {
    if (plot.slices.isEmpty()) {
        return -1
    }
    val dx = tapOffset.x - plot.center.x
    val dy = tapOffset.y - plot.center.y
    val distance = sqrt(dx * dx + dy * dy)
    if (distance > plot.radius) {
        return -1
    }

    var angle = Math.toDegrees(atan2(dy.toDouble(), dx.toDouble())).toFloat()
    if (angle < 0f) {
        angle += 360f
    }

    for (index in plot.slices.indices) {
        val slice = plot.slices[index]
        if (slice.sweepAngle <= 0f) {
            continue
        }
        val start = normalizeAngle(slice.startAngle)
        val end = start + slice.sweepAngle
        val hit = if (end <= 360f) {
            angle >= start && angle <= end
        } else {
            angle >= start || angle <= (end - 360f)
        }
        if (hit) {
            return index
        }
    }
    return plot.slices.lastIndex
}

private fun normalizeAngle(angle: Float): Float {
    var normalized = angle % 360f
    if (normalized < 0f) {
        normalized += 360f
    }
    return normalized
}
