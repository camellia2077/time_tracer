package com.example.tracer

import android.graphics.Color as AndroidColor
import androidx.compose.animation.core.Animatable
import androidx.compose.animation.core.tween
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
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
    adaptSelectionToSurface: Boolean,
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
    val selectionProgress = remember { Animatable(0f) }

    LaunchedEffect(selectedIndex) {
        if (selectedIndex < 0) {
            selectionProgress.snapTo(0f)
        } else {
            selectionProgress.snapTo(0f)
            selectionProgress.animateTo(
                targetValue = 1f,
                animationSpec = tween(durationMillis = 200)
            )
        }
    }

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
    val cellBorderColor = MaterialTheme.colorScheme.outlineVariant
    val selectionSurfaceColor = MaterialTheme.colorScheme.surfaceContainerLow
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

        val fixedSelectionStrokeWidth = with(density) { 2.dp.toPx() }
        val surfaceSelectionStrokeWidth = with(density) { 3.dp.toPx() }
        val fillSelectionStrokeWidth = with(density) { 1.dp.toPx() }

        plot.cells.forEach { cell ->
            val isSelected = cell.pointIndex == selectedIndex && cell.pointIndex >= 0
            val renderedRect = if (isSelected) {
                resolveHeatmapSelectedCellRect(cell.rect, selectionProgress.value)
            } else {
                cell.rect
            }
            val color = resolveHeatmapColor(
                durationSeconds = cell.durationSeconds,
                thresholdsHours = resolvedThresholds,
                paletteColors = resolvedPaletteColors,
                noTimeColor = noTimeColor
            )
            val cornerRadius = CornerRadius(cellCornerRadius, cellCornerRadius)
            drawRoundRect(
                color = color,
                topLeft = renderedRect.topLeft,
                size = renderedRect.size,
                cornerRadius = cornerRadius
            )
            drawRoundRect(
                color = cellBorderColor,
                topLeft = renderedRect.topLeft,
                size = renderedRect.size,
                cornerRadius = cornerRadius,
                style = Stroke(width = cellBorderWidth)
            )
            if (isSelected) {
                val selectionColors = resolveHeatmapSelectionOutlineColors(
                    fillColor = color,
                    surfaceColor = selectionSurfaceColor,
                    adaptToSurface = adaptSelectionToSurface
                )
                val selectionAlpha = 0.25f + 0.75f * selectionProgress.value
                if (selectionColors.fillContrast == null) {
                    val selectedRect = resolveHeatmapSelectionOutlineRect(
                        cellRect = renderedRect,
                        strokeWidth = fixedSelectionStrokeWidth
                    )
                    val selectedOutlineInset = fixedSelectionStrokeWidth / 2f
                    drawRoundRect(
                        color = selectionColors.surfaceContrast.copy(alpha = selectionAlpha),
                        topLeft = selectedRect.topLeft,
                        size = selectedRect.size,
                        cornerRadius = CornerRadius(
                            (cellCornerRadius - selectedOutlineInset).coerceAtLeast(0f),
                            (cellCornerRadius - selectedOutlineInset).coerceAtLeast(0f)
                        ),
                        style = Stroke(width = fixedSelectionStrokeWidth)
                    )
                } else {
                    drawRoundRect(
                        color = selectionColors.surfaceContrast.copy(alpha = selectionAlpha),
                        topLeft = renderedRect.topLeft,
                        size = renderedRect.size,
                        cornerRadius = cornerRadius,
                        style = Stroke(width = surfaceSelectionStrokeWidth)
                    )

                    val innerInset = surfaceSelectionStrokeWidth
                    val innerRect = Rect(
                        left = renderedRect.left + innerInset,
                        top = renderedRect.top + innerInset,
                        right = renderedRect.right - innerInset,
                        bottom = renderedRect.bottom - innerInset
                    )
                    if (innerRect.width > 0f && innerRect.height > 0f) {
                        drawRoundRect(
                            color = selectionColors.fillContrast.copy(alpha = selectionAlpha),
                            topLeft = innerRect.topLeft,
                            size = innerRect.size,
                            cornerRadius = CornerRadius(
                                (cellCornerRadius - innerInset).coerceAtLeast(0f),
                                (cellCornerRadius - innerInset).coerceAtLeast(0f)
                            ),
                            style = Stroke(width = fillSelectionStrokeWidth)
                        )
                    }
                }
            }
        }
    }
}
