package com.example.tracer

import androidx.compose.material3.MaterialTheme
import androidx.compose.ui.test.assertIsDisplayed
import androidx.compose.ui.test.junit4.createComposeRule
import androidx.compose.ui.test.onNodeWithText
import androidx.compose.ui.test.onAllNodesWithText
import androidx.compose.ui.test.onFirst
import androidx.compose.ui.test.performClick
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class InsightsCompositionVisualizationSectionRobolectricTest {
    @get:Rule
    val composeRule = createComposeRule()

    @Test
    fun pieLegend_parentRow_drillsDownToChildren() {
        renderComposition(InsightsCompositionVisualMode.PIE)

        composeRule.onNodeWithText("Parent").performClick()

        composeRule.onNodeWithText("Parent › Child").assertIsDisplayed()
    }

    @Test
    fun treemapLegend_parentRow_drillsDownToChildren() {
        renderComposition(InsightsCompositionVisualMode.TREEMAP)

        composeRule.onNodeWithText("Parent").performClick()

        composeRule.onNodeWithText("Parent › Child").assertIsDisplayed()
    }

    @Test
    fun pieLegend_showsTotalsAndAveragesInTheLegend() {
        composeRule.setContent {
            MaterialTheme {
                InsightsCompositionVisualizationSection(
                    chartError = "",
                    insightsMode = InsightsMode.WEEK,
                    renderModel = CompositionChartRenderModel(
                        totalDurationSeconds = 3_600L,
                        activeRootCount = 1,
                        activeDays = 2,
                        rangeDays = 7,
                        tree = listOf(
                            TreeNode(
                                name = "Parent",
                                durationSeconds = 3_600L,
                                occurrenceCount = 2L,
                                averageDurationSeconds = 1_800L,
                                averageOccurrenceCount = 1.0,
                                averageOccurrenceRatio = 1.0,
                            )
                        )
                    ),
                    compositionVisualMode = InsightsCompositionVisualMode.PIE,
                    piePalettePreset = InsightsPiePalettePreset.SOFT,
                    selectedItemIndex = 0,
                    onItemSelected = {},
                    onCompositionVisualModeChange = {}
                )
            }
        }

        composeRule.onAllNodesWithText("Total: 1h 0m 0s", useUnmergedTree = true)
            .onFirst()
            .assertIsDisplayed()
        composeRule.onNodeWithText("Avg", substring = true, useUnmergedTree = true)
            .assertIsDisplayed()
        composeRule.onNodeWithText("100.0%", useUnmergedTree = true)
            .assertIsDisplayed()
    }

    private fun renderComposition(visualMode: InsightsCompositionVisualMode) {
        composeRule.setContent {
            MaterialTheme {
                InsightsCompositionVisualizationSection(
                    chartError = "",
                    insightsMode = InsightsMode.WEEK,
                    renderModel = CompositionChartRenderModel(
                        totalDurationSeconds = 3_600L,
                        activeRootCount = 1,
                        rangeDays = 7,
                        tree = listOf(
                            TreeNode(
                                name = "Parent",
                                durationSeconds = 3_600L,
                                children = listOf(
                                    TreeNode(name = "Child", durationSeconds = 3_600L)
                                )
                            )
                        )
                    ),
                    compositionVisualMode = visualMode,
                    piePalettePreset = InsightsPiePalettePreset.SOFT,
                    selectedItemIndex = -1,
                    onItemSelected = {},
                    onCompositionVisualModeChange = {}
                )
            }
        }
    }
}
