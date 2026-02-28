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
import java.time.LocalDate
import java.time.temporal.ChronoUnit
import kotlin.math.min

internal enum class ReportHeatmapMode {
    MONTH,
    YEAR
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
    heatmapTomlConfig: ReportHeatmapTomlConfig,
    heatmapStylePreference: ReportHeatmapStylePreference,
    isAppDarkThemeActive: Boolean,
    onPointSelected: (Int) -> Unit,
    modifier: Modifier = Modifier
) {
    val parsedPoints = remember(points) { parseHeatmapPoints(points) }
    val anchorDate = remember(parsedPoints, selectedIndex) {
        resolveAnchorDate(parsedPoints = parsedPoints, selectedIndex = selectedIndex)
    }
    var canvasSize by remember { mutableStateOf(IntSize.Zero) }

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
            size = size
        )
        if (plot.cells.isEmpty()) {
            return@Canvas
        }

        val selectedStrokeWidth = if (mode == ReportHeatmapMode.YEAR) 1.2f else 2f

        plot.cells.forEach { cell ->
            val color = resolveHeatmapColor(
                durationSeconds = cell.durationSeconds,
                thresholdsHours = resolvedThresholds,
                paletteColors = resolvedPaletteColors
            )
            val cornerRadius = resolveCellCornerRadius(cell = cell, mode = mode)
            drawRoundRect(
                color = color,
                topLeft = cell.rect.topLeft,
                size = cell.rect.size,
                cornerRadius = cornerRadius
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
    size: Size
): HeatmapPlot {
    if (size.width <= 0f || size.height <= 0f) {
        return HeatmapPlot(cells = emptyList())
    }
    return when (mode) {
        ReportHeatmapMode.MONTH -> HeatmapPlot(
            cells = buildMonthHeatmapCells(points = points, anchorDate = anchorDate, size = size)
        )

        ReportHeatmapMode.YEAR -> HeatmapPlot(
            cells = buildYearHeatmapCells(points = points, anchorDate = anchorDate, size = size)
        )
    }
}

private fun buildMonthHeatmapCells(
    points: List<ParsedHeatmapPoint>,
    anchorDate: LocalDate,
    size: Size
): List<HeatmapCell> {
    val columns = 7
    val rows = 6
    val spacing = 4f
    val horizontalPadding = 12f
    val verticalPadding = 12f
    val usableWidth = (size.width - horizontalPadding * 2f - spacing * (columns - 1))
        .coerceAtLeast(1f)
    val usableHeight = (size.height - verticalPadding * 2f - spacing * (rows - 1))
        .coerceAtLeast(1f)
    val slotWidth = usableWidth / columns.toFloat()
    val slotHeight = usableHeight / rows.toFloat()
    // Month heatmap uses square cells for better visual consistency.
    val cellSide = min(slotWidth, slotHeight)
    val cellOffsetX = (slotWidth - cellSide) / 2f
    val cellOffsetY = (slotHeight - cellSide) / 2f

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
            x = horizontalPadding + column * (slotWidth + spacing) + cellOffsetX,
            y = verticalPadding + row * (slotHeight + spacing) + cellOffsetY
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

private fun buildYearHeatmapCells(
    points: List<ParsedHeatmapPoint>,
    anchorDate: LocalDate,
    size: Size
): List<HeatmapCell> {
    val year = anchorDate.year
    val startDate = LocalDate.of(year, 1, 1)
    val endDate = LocalDate.of(year, 12, 31)

    val firstWeekStart = startDate.minusDays((startDate.dayOfWeek.value - 1).toLong())
    val lastWeekStart = endDate.minusDays((endDate.dayOfWeek.value - 1).toLong())
    val weekCount = ChronoUnit.WEEKS.between(firstWeekStart, lastWeekStart).toInt() + 1
    val rows = 7

    val spacing = 2f
    val horizontalPadding = 8f
    val verticalPadding = 8f
    val usableWidth = (size.width - horizontalPadding * 2f - spacing * (weekCount - 1))
        .coerceAtLeast(1f)
    val usableHeight = (size.height - verticalPadding * 2f - spacing * (rows - 1))
        .coerceAtLeast(1f)
    val cellWidth = usableWidth / weekCount.toFloat()
    val cellHeight = usableHeight / rows.toFloat()

    val pointsByDate = points.associateBy { point -> point.date }
    val cells = mutableListOf<HeatmapCell>()

    var cursor = startDate
    while (!cursor.isAfter(endDate)) {
        val weekStart = cursor.minusDays((cursor.dayOfWeek.value - 1).toLong())
        val column = ChronoUnit.WEEKS.between(firstWeekStart, weekStart).toInt()
        val row = cursor.dayOfWeek.value - 1
        val topLeft = Offset(
            x = horizontalPadding + column * (cellWidth + spacing),
            y = verticalPadding + row * (cellHeight + spacing)
        )
        val point = pointsByDate[cursor]
        cells += HeatmapCell(
            rect = Rect(topLeft, Size(cellWidth, cellHeight)),
            pointIndex = point?.index ?: -1,
            durationSeconds = point?.durationSeconds ?: 0L
        )
        cursor = cursor.plusDays(1)
    }
    return cells
}

private fun resolveHeatmapColor(
    durationSeconds: Long,
    thresholdsHours: List<Double>,
    paletteColors: List<Color>
): Color {
    if (paletteColors.isEmpty()) {
        return Color.Transparent
    }
    if (paletteColors.size == 1) {
        return paletteColors.first()
    }

    val safeDurationHours = durationSeconds.coerceAtLeast(0L).toDouble() / 3600.0
    if (safeDurationHours <= 0.0) {
        return paletteColors.first()
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

private fun resolveCellCornerRadius(
    cell: HeatmapCell,
    mode: ReportHeatmapMode
): CornerRadius {
    val base = min(cell.rect.width, cell.rect.height)
    val ratio = if (mode == ReportHeatmapMode.MONTH) 0.22f else 0.16f
    val minRadius = if (mode == ReportHeatmapMode.MONTH) 2f else 1f
    val maxRadius = if (mode == ReportHeatmapMode.MONTH) 8f else 4f
    val radius = (base * ratio).coerceIn(minRadius, maxRadius)
    return CornerRadius(radius, radius)
}
