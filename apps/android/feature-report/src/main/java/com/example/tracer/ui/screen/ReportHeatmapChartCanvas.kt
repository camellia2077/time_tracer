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

internal enum class ReportHeatmapMode {
    MONTH
}

private data class ParsedHeatmapPoint(
    val index: Int,
    val date: LocalDate,
    val durationSeconds: Long
)

private data class HeatmapCell(
    val rect: Rect,
    val pointIndex: Int,
    val durationSeconds: Long
)

private data class HeatmapPlot(
    val cells: List<HeatmapCell>
)

@Composable
internal fun ReportHeatmapChart(
    points: List<ReportChartPoint>,
    selectedIndex: Int,
    mode: ReportHeatmapMode,
    anchorDateOverride: LocalDate? = null,
    heatmapTomlConfig: ReportHeatmapTomlConfig,
    heatmapStylePreference: ReportHeatmapStylePreference,
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

private fun parseHeatmapPoints(points: List<ReportChartPoint>): List<ParsedHeatmapPoint> =
    points.mapIndexedNotNull { index, point ->
        val parsedDate = try {
            LocalDate.parse(point.date)
        } catch (_: Exception) {
            null
        } ?: return@mapIndexedNotNull null
        ParsedHeatmapPoint(
            index = index,
            date = parsedDate,
            durationSeconds = point.durationSeconds.coerceAtLeast(0L)
        )
    }

private fun resolveAnchorDate(
    parsedPoints: List<ParsedHeatmapPoint>,
    selectedIndex: Int
): LocalDate? {
    val selectedDate = parsedPoints.firstOrNull { point -> point.index == selectedIndex }?.date
    return selectedDate ?: parsedPoints.maxByOrNull { point -> point.date }?.date
}

private fun buildHeatmapPlot(
    points: List<ParsedHeatmapPoint>,
    anchorDate: LocalDate,
    mode: ReportHeatmapMode,
    spacing: Float,
    size: Size
): HeatmapPlot {
    if (size.width <= 0f || size.height <= 0f) {
        return HeatmapPlot(cells = emptyList())
    }
    return when (mode) {
        ReportHeatmapMode.MONTH -> HeatmapPlot(
            cells = buildMonthHeatmapCells(
                points = points,
                anchorDate = anchorDate,
                spacing = spacing,
                size = size
            )
        )
    }
}

private fun buildMonthHeatmapCells(
    points: List<ParsedHeatmapPoint>,
    anchorDate: LocalDate,
    spacing: Float,
    size: Size
): List<HeatmapCell> {
    val columns = 7
    val rows = 6
    val horizontalPadding = 12f
    val verticalPadding = 12f
    val usableWidth = (size.width - horizontalPadding * 2f - spacing * (columns - 1))
        .coerceAtLeast(1f)
    val usableHeight = (size.height - verticalPadding * 2f - spacing * (rows - 1))
        .coerceAtLeast(1f)
    // Size the complete square grid first, then center the grid as a whole.
    // Centering each cell inside independently sized row/column slots makes
    // the unused height look like a larger vertical gap.
    val cellSide = min(
        usableWidth / columns.toFloat(),
        usableHeight / rows.toFloat()
    )
    val gridWidth = columns * cellSide + spacing * (columns - 1)
    val gridHeight = rows * cellSide + spacing * (rows - 1)
    val gridOffsetX = (size.width - gridWidth) / 2f
    val gridOffsetY = (size.height - gridHeight) / 2f

    val firstDay = anchorDate.withDayOfMonth(1)
    val firstColumn = firstDay.dayOfWeek.value - 1
    val daysInMonth = firstDay.lengthOfMonth()
    val pointsByDate = points.associateBy { point -> point.date }

    val cells = mutableListOf<HeatmapCell>()
    for (day in 1..daysInMonth) {
        val date = firstDay.withDayOfMonth(day)
        val slotIndex = firstColumn + day - 1
        val row = slotIndex / columns
        val column = slotIndex % columns
        if (row !in 0 until rows) {
            continue
        }
        val topLeft = Offset(
            x = gridOffsetX + column * (cellSide + spacing),
            y = gridOffsetY + row * (cellSide + spacing)
        )
        val point = pointsByDate[date]
        cells += HeatmapCell(
            rect = Rect(topLeft, Size(cellSide, cellSide)),
            pointIndex = point?.index ?: -1,
            durationSeconds = point?.durationSeconds ?: 0L
        )
    }
    return cells
}

private fun resolveHeatmapColor(
    durationSeconds: Long,
    thresholdsHours: List<Double>,
    paletteColors: List<Color>,
    noTimeColor: Color
): Color {
    if (paletteColors.isEmpty()) {
        return Color.Transparent
    }
    if (paletteColors.size == 1) {
        return paletteColors.first()
    }

    val safeDurationHours = durationSeconds.coerceAtLeast(0L).toDouble() / 3600.0
    if (safeDurationHours <= 0.0) {
        return noTimeColor
    }

    var bucketIndex = 1
    for (threshold in thresholdsHours) {
        if (safeDurationHours <= threshold) {
            return paletteColors.getOrElse(bucketIndex) { paletteColors.last() }
        }
        bucketIndex += 1
    }
    return paletteColors.last()
}

private fun normalizeThresholds(rawThresholds: List<Double>): List<Double> {
    if (rawThresholds.isEmpty()) {
        return defaultReportHeatmapTomlConfig().thresholdsHours
    }
    val normalized = rawThresholds
        .filter { it.isFinite() && it > 0.0 }
        .sorted()
        .distinct()
    if (normalized.isEmpty()) {
        return defaultReportHeatmapTomlConfig().thresholdsHours
    }
    return normalized
}

private fun resolveHeatmapPaletteColors(
    config: ReportHeatmapTomlConfig,
    stylePreference: ReportHeatmapStylePreference,
    isSystemDark: Boolean,
    fallbackEmptyColor: Color,
    fallbackActiveColor: Color
): List<Color> {
    val paletteName = resolvePaletteName(
        config = config,
        stylePreference = stylePreference,
        isSystemDark = isSystemDark
    )
    val configuredColors = config.palettes[paletteName].orEmpty()
    val parsedColors = configuredColors.mapNotNull(::parseHexColor)
    if (parsedColors.isNotEmpty()) {
        return parsedColors
    }
    return listOf(
        fallbackEmptyColor,
        fallbackActiveColor
    )
}

private fun resolvePaletteName(
    config: ReportHeatmapTomlConfig,
    stylePreference: ReportHeatmapStylePreference,
    isSystemDark: Boolean
): String {
    val availableNames = config.palettes.keys
    if (availableNames.isEmpty()) {
        val fallbackConfig = defaultReportHeatmapTomlConfig()
        return fallbackConfig.defaultLightPalette
    }
    if (stylePreference.themePolicy == ReportHeatmapThemePolicy.PALETTE &&
        stylePreference.paletteName in availableNames
    ) {
        return stylePreference.paletteName
    }

    val autoPaletteName = when (stylePreference.themePolicy) {
        ReportHeatmapThemePolicy.FOLLOW_SYSTEM -> {
            if (isSystemDark) config.defaultDarkPalette else config.defaultLightPalette
        }

        ReportHeatmapThemePolicy.PALETTE -> config.defaultLightPalette
    }

    if (autoPaletteName in availableNames) {
        return autoPaletteName
    }
    return availableNames.first()
}

private fun parseHexColor(raw: String): Color? {
    val normalized = raw.trim()
    return runCatching {
        Color(AndroidColor.parseColor(normalized))
    }.getOrNull()
}
