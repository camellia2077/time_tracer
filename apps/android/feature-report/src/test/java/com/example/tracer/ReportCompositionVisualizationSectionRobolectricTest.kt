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
class ReportCompositionVisualizationSectionRobolectricTest {
    @get:Rule
    val composeRule = createComposeRule()

    @Test
    fun pieLegend_parentRow_drillsDownToChildren() {
        renderComposition(ReportCompositionVisualMode.PIE)

        composeRule.onNodeWithText("Parent").performClick()

        composeRule.onNodeWithText("Parent › Child").assertIsDisplayed()
    }

    @Test
    fun treemapLegend_parentRow_drillsDownToChildren() {
        renderComposition(ReportCompositionVisualMode.TREEMAP)

        composeRule.onNodeWithText("Parent").performClick()

        composeRule.onNodeWithText("Parent › Child").assertIsDisplayed()
    }

    @Test
    fun pieLegend_showsTotalsAndAveragesInTheLegend() {
        composeRule.setContent {
            MaterialTheme {
                ReportCompositionVisualizationSection(
                    chartError = "",
                    reportMode = ReportMode.WEEK,
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
                    compositionVisualMode = ReportCompositionVisualMode.PIE,
                    piePalettePreset = ReportPiePalettePreset.SOFT,
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

    private fun renderComposition(visualMode: ReportCompositionVisualMode) {
        composeRule.setContent {
            MaterialTheme {
                ReportCompositionVisualizationSection(
                    chartError = "",
                    reportMode = ReportMode.WEEK,
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
                    piePalettePreset = ReportPiePalettePreset.SOFT,
                    selectedItemIndex = -1,
                    onItemSelected = {},
                    onCompositionVisualModeChange = {}
                )
            }
        }
    }
}
