package com.example.tracer

import androidx.compose.foundation.Canvas
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
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
internal fun InsightsBarChart(
    points: List<InsightsChartPoint>,
    comparisonPoints: List<InsightsChartPoint>,
    comparisonPeriodLabel: String,
    selectedIndex: Int,
    averageDurationSeconds: Long,
    showAverageLine: Boolean,
    onPointSelected: (Int) -> Unit,
    modifier: Modifier = Modifier
) {
    val durationHours = remember(points) { points.map { it.durationSeconds.coerceAtLeast(0L) / 3600f } }
    val comparisonDurationHours = remember(comparisonPoints) {
        comparisonPoints.map { it.durationSeconds.coerceAtLeast(0L) / 3600f }
    }
    val hasComparison = comparisonDurationHours.isNotEmpty()
    val maxDurationHours = (durationHours + comparisonDurationHours).maxOrNull()?.coerceAtLeast(1f) ?: 1f
    var canvasSize by remember { mutableStateOf(IntSize.Zero) }
    val currentColor = MaterialTheme.colorScheme.primary.copy(alpha = 0.75f)
    val comparisonColor = MaterialTheme.colorScheme.primary.copy(alpha = 0.35f)
    val gridColor = MaterialTheme.colorScheme.outlineVariant
    val averageLineColor = MaterialTheme.colorScheme.tertiary
    val selectedGuideColor = MaterialTheme.colorScheme.secondary
    val selectedBarColor = MaterialTheme.colorScheme.tertiary
    val axisLabelColor = MaterialTheme.colorScheme.onSurfaceVariant

    Column(modifier = modifier) {
        Canvas(
            modifier = Modifier
                .fillMaxWidth()
                .height(220.dp)
                .onSizeChanged { canvasSize = it }
                .pointerInput(durationHours, comparisonDurationHours, canvasSize) {
                    detectTapGestures { tapOffset ->
                        if (durationHours.isEmpty() || canvasSize.width == 0 || canvasSize.height == 0) return@detectTapGestures
                        val size = Size(canvasSize.width.toFloat(), canvasSize.height.toFloat())
                        val centers = if (hasComparison) {
                            buildGroupedBarChartPlot(durationHours, comparisonDurationHours, size, maxDurationHours).currentCenters
                        } else {
                            buildBarChartPlot(durationHours, size).centers
                        }
                        val index = centers.indices.minByOrNull { i ->
                            val dx = centers[i].x - tapOffset.x
                            dx * dx
                        } ?: return@detectTapGestures
                        if (kotlin.math.abs(centers[index].x - tapOffset.x) <= 24.dp.toPx()) onPointSelected(index)
                    }
                }
        ) {
            if (durationHours.isEmpty()) return@Canvas
            val groupedPlot = if (hasComparison) {
                buildGroupedBarChartPlot(durationHours, comparisonDurationHours, size, maxDurationHours)
            } else null
            val barPlot = if (!hasComparison) buildBarChartPlot(durationHours, size) else null
            val left = if (groupedPlot != null) groupedPlot.leftPadding else barPlot!!.leftPadding
            val top = if (groupedPlot != null) groupedPlot.topPadding else barPlot!!.topPadding
            val width = if (groupedPlot != null) groupedPlot.chartWidth else barPlot!!.chartWidth
            val height = if (groupedPlot != null) groupedPlot.chartHeight else barPlot!!.chartHeight
            for (index in 0..4) {
                val y = top + height * index / 4f
                drawLine(gridColor, Offset(left, y), Offset(left + width, y), 1f)
            }
            drawDurationYAxisLabels(maxDurationHours, left, top, height, axisLabelColor)

            if (showAverageLine) {
                val averageHours = resolveAverageDurationHours(durationHours, averageDurationSeconds)
                if (averageHours != null) {
                    val y = top + height * (1f - (averageHours / maxDurationHours).coerceIn(0f, 1f))
                    drawLine(
                        averageLineColor,
                        Offset(left, y),
                        Offset(left + width, y),
                        2f,
                        pathEffect = PathEffect.dashPathEffect(floatArrayOf(14f, 8f), 0f)
                    )
                }
            }

            val currentBars: List<BarColumn>
            val comparisonBars: List<BarColumn>
            val centers: List<Offset>
            if (groupedPlot != null) {
                currentBars = groupedPlot.currentBars
                comparisonBars = groupedPlot.comparisonBars
                centers = groupedPlot.currentCenters
            } else {
                currentBars = barPlot!!.bars
                comparisonBars = emptyList()
                centers = barPlot.centers
            }
            comparisonBars.forEach { drawRect(comparisonColor, it.topLeft, it.size) }
            currentBars.forEachIndexed { index, bar ->
                drawRect(if (index == selectedIndex) selectedBarColor else currentColor, bar.topLeft, bar.size)
            }
            if (selectedIndex in centers.indices) {
                val x = centers[selectedIndex].x
                drawLine(selectedGuideColor.copy(alpha = 0.35f), Offset(x, top), Offset(x, top + height), 1.5f)
            }
        }
        if (hasComparison) InsightsChartComparisonLegend(comparisonLabel = comparisonPeriodLabel)
    }
}
