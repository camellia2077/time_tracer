package com.example.tracer

import androidx.compose.material3.MaterialTheme
import androidx.compose.ui.test.assertIsDisplayed
import androidx.compose.ui.test.junit4.createComposeRule
import androidx.compose.ui.test.onNodeWithContentDescription
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
                    chartMedianDurationSeconds = 2_700.0,
                    chartMinimumDurationSeconds = 0.0,
                    chartMaximumDurationSeconds = 3_600.0,
                    chartLowerQuartileDurationSeconds = 2_250.0,
                    chartUpperQuartileDurationSeconds = 3_150.0,
                    chartCoefficientOfVariation = 1.0 / 3.0,
                    chartMeanAbsoluteDeviationSeconds = 900.0
                )
            }
        }

        composeRule.onNodeWithText(
            "Selected day: 1h 0m 0s, above the usual daily band of 0h 37m 30s–0h 52m 30s."
        ).assertIsDisplayed()
        composeRule.onNodeWithText("Trend overview").assertIsDisplayed()
        composeRule.onNodeWithText("2 / 2 days (100%)").assertIsDisplayed()
        composeRule.onNodeWithText("Total time").assertIsDisplayed()
        composeRule.onNodeWithText("Records").assertIsDisplayed()
        composeRule.onNodeWithText("Daily statistics").assertExists()
        composeRule.onNodeWithText("Daily average").assertExists()
        composeRule.onNodeWithText("Median day").assertExists()
        composeRule.onNodeWithText("Usual daily band").assertExists()
        composeRule.onNodeWithText("Avg per record").assertExists()
        composeRule.onNodeWithText("Variation").assertExists()
        composeRule.onNodeWithText("Lowest–highest day").assertExists()
        composeRule.onNodeWithText("Relative variation").assertExists()
        composeRule.onNodeWithText("Typical daily difference").assertExists()

        composeRule.onNodeWithContentDescription("Show explanation for Relative variation")
            .assertExists()
    }
}
