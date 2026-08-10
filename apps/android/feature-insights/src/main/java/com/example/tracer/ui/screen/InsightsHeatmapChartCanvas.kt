package com.example.tracer

import android.graphics.Color as AndroidColor
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.CornerRadius
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Rect
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.layout.onSizeChanged
import androidx.compose.ui.unit.IntSize
import androidx.compose.ui.unit.dp
import androidx.compose.ui.platform.LocalDensity
import java.time.LocalDate
import kotlin.math.min

internal enum class InsightsHeatmapMode {
    MONTH
}
internal data class ParsedHeatmapPoint(
    val index: Int,
    val date: LocalDate,
    val durationSeconds: Long
)

internal data class HeatmapCell(
    val rect: Rect,
    val pointIndex: Int,
    val durationSeconds: Long
)

internal data class HeatmapPlot(
    val cells: List<HeatmapCell>
)

@Composable
internal fun InsightsHeatmapChart(
    points: List<InsightsChartPoint>,
    selectedIndex: Int,
    mode: InsightsHeatmapMode,
    anchorDateOverride: LocalDate? = null,
    heatmapTomlConfig: InsightsHeatmapTomlConfig,
    heatmapStylePreference: InsightsHeatmapStylePreference,
    isAppDarkThemeActive: Boolean,
    onPointSelected: (Int) -> Unit,
    modifier: Modifier = Modifier
) {
    val parsedPoints = remember(points) { parseHeatmapPoints(points) }
    val anchorDate = remember(parsedPoints, selectedIndex, anchorDateOverride) {
        anchorDateOverride ?: resolveAnchorDate(parsedPoints = parsedPoints, selectedIndex = selectedIndex)
    }
    var canvasSize by remember { mutableStateOf(IntSize.Zero) }
    val density = LocalDensity.current
    val cellSpacing = with(density) { 3.dp.toPx() }
    val cellCornerRadius = with(density) { 2.dp.toPx() }
    val cellBorderWidth = with(density) { 1.dp.toPx() }

    val isSystemDark = isSystemInDarkTheme()
    val resolvedThresholds = remember(heatmapTomlConfig.thresholdsHours) {
        normalizeThresholds(heatmapTomlConfig.thresholdsHours)
    }
    val fallbackEmptyColor = MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.45f)
    val fallbackActiveColor = MaterialTheme.colorScheme.primary
    val resolvedPaletteColors = remember(
        heatmapTomlConfig,
        heatmapStylePreference,
        isSystemDark,
        isAppDarkThemeActive,
        fallbackEmptyColor,
        fallbackActiveColor
    ) {
        resolveHeatmapPaletteColors(
            config = heatmapTomlConfig,
            stylePreference = heatmapStylePreference,
            isSystemDark = isSystemDark,
            fallbackEmptyColor = fallbackEmptyColor,
            fallbackActiveColor = fallbackActiveColor
        )
    }
    val selectedOutlineColor = MaterialTheme.colorScheme.tertiary
    val cellBorderColor = MaterialTheme.colorScheme.outlineVariant
    // The TOML palette defines colors[0] as the no-time bucket. Keep this
    // separate from positive-duration buckets so the base color remains
    // theme/palette driven even when the thresholds change.
    val noTimeColor = resolvedPaletteColors.firstOrNull() ?: fallbackEmptyColor

    Canvas(
        modifier = modifier
            .onSizeChanged { canvasSize = it }
            .pointerInput(parsedPoints, anchorDate, mode, canvasSize) {
                detectTapGestures { tapOffset ->
                    if (parsedPoints.isEmpty() || anchorDate == null ||
                        canvasSize.width == 0 || canvasSize.height == 0
                    ) {
                        return@detectTapGestures
                    }
                    val plot = buildHeatmapPlot(
                        points = parsedPoints,
                        anchorDate = anchorDate,
                        mode = mode,
                        spacing = cellSpacing,
                        size = Size(
                            canvasSize.width.toFloat(),
                            canvasSize.height.toFloat()
                        )
                    )
                    val hitCell = plot.cells.firstOrNull { cell -> cell.rect.contains(tapOffset) }
                    if (hitCell != null && hitCell.pointIndex >= 0) {
                        onPointSelected(hitCell.pointIndex)
                    }
                }
            }
    ) {
        if (parsedPoints.isEmpty() || anchorDate == null) {
            return@Canvas
        }
        val plot = buildHeatmapPlot(
            points = parsedPoints,
            anchorDate = anchorDate,
            mode = mode,
            spacing = cellSpacing,
            size = size
        )
        if (plot.cells.isEmpty()) {
            return@Canvas
        }

        val selectedStrokeWidth = 2f

        plot.cells.forEach { cell ->
            val color = resolveHeatmapColor(
                durationSeconds = cell.durationSeconds,
                thresholdsHours = resolvedThresholds,
                paletteColors = resolvedPaletteColors,
                noTimeColor = noTimeColor
            )
            val cornerRadius = CornerRadius(cellCornerRadius, cellCornerRadius)
            drawRoundRect(
                color = color,
                topLeft = cell.rect.topLeft,
                size = cell.rect.size,
                cornerRadius = cornerRadius
            )
            drawRoundRect(
                color = cellBorderColor,
                topLeft = cell.rect.topLeft,
                size = cell.rect.size,
                cornerRadius = cornerRadius,
                style = Stroke(width = cellBorderWidth)
            )
            if (cell.pointIndex == selectedIndex && cell.pointIndex >= 0) {
                drawRoundRect(
                    color = selectedOutlineColor,
                    topLeft = cell.rect.topLeft,
                    size = cell.rect.size,
                    cornerRadius = cornerRadius,
                    style = Stroke(width = selectedStrokeWidth)
                )
            }
        }
    }
}
