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

internal fun parseHeatmapPoints(points: List<ReportChartPoint>): List<ParsedHeatmapPoint> =
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

internal fun resolveAnchorDate(
    parsedPoints: List<ParsedHeatmapPoint>,
    selectedIndex: Int
): LocalDate? {
    val selectedDate = parsedPoints.firstOrNull { point -> point.index == selectedIndex }?.date
    return selectedDate ?: parsedPoints.maxByOrNull { point -> point.date }?.date
}

internal fun buildHeatmapPlot(
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

internal fun buildMonthHeatmapCells(
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

internal fun resolveHeatmapColor(
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

internal fun normalizeThresholds(rawThresholds: List<Double>): List<Double> {
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

internal fun resolveHeatmapPaletteColors(
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

internal fun resolvePaletteName(
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

internal fun parseHexColor(raw: String): Color? {
    val normalized = raw.trim()
    return runCatching {
        Color(AndroidColor.parseColor(normalized))
    }.getOrNull()
}

