package com.example.tracer

import androidx.compose.ui.geometry.Rect
import androidx.compose.ui.graphics.Color
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class InsightsChartHeatmapPolicyTest {
    @Test
    fun rangeUsesSingleMonthWhenExplicitWindowStaysWithinMonth() {
        assertEquals(
            InsightsChartVisualMode.HEATMAP_MONTH,
            resolveInsightsChartVisualMode(
                insightsMode = InsightsMode.RANGE,
                requestedMode = InsightsChartVisualMode.HEATMAP_MONTH,
                points = listOf(chartPoint("2026-07-01")),
                fromDateIso = "2026-07-20",
                toDateIso = "2026-07-31"
            )
        )
    }

    @Test
    fun rangeUsesMultiMonthWhenExplicitWindowCrossesMonth() {
        assertEquals(
            InsightsChartVisualMode.HEATMAP_MULTI_MONTH,
            resolveInsightsChartVisualMode(
                insightsMode = InsightsMode.RANGE,
                requestedMode = InsightsChartVisualMode.HEATMAP_MONTH,
                points = emptyList(),
                fromDateIso = "2026-07-31",
                toDateIso = "2026-08-01"
            )
        )
    }

    @Test
    fun recentFallsBackToPointMonthsWhenExplicitWindowIsUnavailable() {
        assertEquals(
            InsightsChartVisualMode.HEATMAP_MULTI_MONTH,
            resolveInsightsChartVisualMode(
                insightsMode = InsightsMode.RECENT,
                requestedMode = InsightsChartVisualMode.HEATMAP_MONTH,
                points = listOf(
                    chartPoint("2026-07-31"),
                    chartPoint("2026-08-01")
                )
            )
        )
    }

    @Test
    fun yearOnlyOffersMultiMonthHeatmap() {
        val modes = availableInsightsChartVisualModes(InsightsMode.YEAR)

        assertEquals(false, InsightsChartVisualMode.HEATMAP_MONTH in modes)
        assertEquals(true, InsightsChartVisualMode.HEATMAP_MULTI_MONTH in modes)
    }

    @Test
    fun selectionOutlineUsesContrastingColorForHeatmapFill() {
        assertEquals(Color.White, resolveHeatmapSelectionOutlineColor(Color(0xFF123456)))
        assertEquals(Color.Black, resolveHeatmapSelectionOutlineColor(Color(0xFFE8EEF5)))
    }

    @Test
    fun switchableThemeSelectionContrastsWithSurfaceAndFill() {
        val lightSurfaceSelection = resolveHeatmapSelectionOutlineColors(
            fillColor = Color(0xFF123456),
            surfaceColor = Color.White,
            adaptToSurface = true
        )
        val darkSurfaceSelection = resolveHeatmapSelectionOutlineColors(
            fillColor = Color(0xFFE8EEF5),
            surfaceColor = Color.Black,
            adaptToSurface = true
        )

        assertEquals(Color.Black, lightSurfaceSelection.surfaceContrast)
        assertEquals(Color.White, lightSurfaceSelection.fillContrast)
        assertEquals(Color.White, darkSurfaceSelection.surfaceContrast)
        assertEquals(Color.Black, darkSurfaceSelection.fillContrast)
    }

    @Test
    fun fixedAppearanceSelectionKeepsSingleFillContrastOutline() {
        val selection = resolveHeatmapSelectionOutlineColors(
            fillColor = Color(0xFF123456),
            surfaceColor = Color.White,
            adaptToSurface = false
        )

        assertEquals(Color.White, selection.surfaceContrast)
        assertEquals(null, selection.fillContrast)
    }

    @Test
    fun selectionOutlineStaysInsideAnimatedHeatmapCell() {
        val cell = Rect(10f, 20f, 30f, 40f)
        val animationStart = resolveHeatmapSelectedCellRect(cell, animationProgress = 0f)
        val animationEnd = resolveHeatmapSelectedCellRect(cell, animationProgress = 1f)
        val outline = resolveHeatmapSelectionOutlineRect(animationEnd, strokeWidth = 2f)

        assertTrue(animationStart.width < cell.width)
        assertTrue(animationStart.height < cell.height)
        assertEquals(cell, animationEnd)
        assertTrue(outline.left > cell.left)
        assertTrue(outline.top > cell.top)
        assertTrue(outline.right < cell.right)
        assertTrue(outline.bottom < cell.bottom)
    }

    private fun chartPoint(date: String): InsightsChartPoint = InsightsChartPoint(
        date = date,
        durationSeconds = 60L
    )
}
