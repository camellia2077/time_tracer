package com.example.tracer

import androidx.compose.ui.geometry.Size
import androidx.compose.ui.unit.Density
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class InsightsChartGeometryTest {
    @Test
    fun yAxisGutterExpandsForHighDensityAndFontScale() {
        val standardLayout = buildDurationChartLayout(
            maxDurationHours = 24.6f,
            density = Density(density = 3f, fontScale = 1f)
        )
        val largeLayout = buildDurationChartLayout(
            maxDurationHours = 24.6f,
            density = Density(density = 3.5f, fontScale = 1.3f)
        )

        assertTrue(largeLayout.leftPadding > standardLayout.leftPadding)
    }

    @Test
    fun linePlotUsesMeasuredYAxisGutter() {
        val layout = buildDurationChartLayout(
            maxDurationHours = 24.6f,
            density = Density(density = 3.5f, fontScale = 1.3f)
        )

        val plot = buildChartPlot(
            durationHours = listOf(24.6f, 12.3f),
            size = Size(1_200f, 600f),
            layout = layout
        )

        assertEquals(layout.leftPadding, plot.leftPadding)
        assertTrue(plot.chartWidth > 1f)
    }
}
