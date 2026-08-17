package com.example.tracer

import androidx.compose.material3.MaterialTheme
import androidx.compose.ui.test.assertIsDisplayed
import androidx.compose.ui.test.junit4.createComposeRule
import androidx.compose.ui.test.onNodeWithText
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class InsightsChartVisualizationSummaryRobolectricTest {
    @get:Rule
    val composeRule = createComposeRule()

    @Test
    fun summary_displaysCoreDistributionStatistics() {
        composeRule.setContent {
            MaterialTheme {
                InsightsChartVisualizationSummary(
                    sortedChartPoints = listOf(
                        InsightsChartPoint("2026-02-01", 3_600L),
                        InsightsChartPoint("2026-02-02", 1_800L),
                    ),
                    recordedPoints = listOf(
                        InsightsChartPoint("2026-02-01", 3_600L),
                        InsightsChartPoint("2026-02-02", 1_800L),
                    ),
                    selectedPointIndex = 0,
                    chartFromDateIso = "2026-02-01",
                    chartToDateIso = "2026-02-02",
                    chartAverageDurationSeconds = 2_700L,
                    chartTotalOccurrenceCount = 3L,
                    chartTotalDurationSeconds = 5_400L,
                    chartAverageDurationPerOccurrenceSeconds = 900L,
                    chartModeDurationSeconds = 1_800.0,
                    chartMedianDurationSeconds = 2_700.0,
                    chartMinimumDurationSeconds = 0.0,
                    chartMaximumDurationSeconds = 3_600.0,
                    chartLowerQuartileDurationSeconds = 2_250.0,
                    chartUpperQuartileDurationSeconds = 3_150.0,
                    chartCoefficientOfVariation = 1.0 / 3.0,
                    chartMeanAbsoluteDeviationSeconds = 900.0,
                    chartVisualMode = InsightsChartVisualMode.LINE
                )
            }
        }

        composeRule.onNodeWithText("Record completeness: 2 / 2 days (100%)").assertIsDisplayed()
        composeRule.onNodeWithText("Total occurrences: 3").assertIsDisplayed()
        composeRule.onNodeWithText("Total: 1h 30m 0s").assertIsDisplayed()
        composeRule.onNodeWithText("Average: 0h 45m 0s").assertIsDisplayed()
        composeRule.onNodeWithText("Avg per occurrence: 0h 15m 0s").assertIsDisplayed()
        composeRule.onNodeWithText("Mode: 0h 30m 0s").assertIsDisplayed()
        composeRule.onNodeWithText("Median: 0h 45m 0s").assertIsDisplayed()
        composeRule.onNodeWithText("Full range: 0h 0m 0s–1h 0m 0s").assertIsDisplayed()
        composeRule.onNodeWithText("Typical daily range: 0h 37m 30s–0h 52m 30s").assertIsDisplayed()
        composeRule.onNodeWithText("Relative variability (CV): 33%").assertIsDisplayed()
        composeRule.onNodeWithText("Mean absolute deviation (MAD): 0h 15m 0s").assertIsDisplayed()
    }
}
