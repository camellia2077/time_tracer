package com.example.tracer

import android.content.Context
import androidx.compose.material3.MaterialTheme
import androidx.compose.ui.test.junit4.createComposeRule
import androidx.compose.ui.test.onNodeWithText
import androidx.test.core.app.ApplicationProvider
import com.example.tracer.feature.insights.R
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class InsightsChartSemanticModeSelectorRobolectricTest {
    @get:Rule
    val composeRule = createComposeRule()

    private val context: Context = ApplicationProvider.getApplicationContext()

    @Test
    fun dayHidesTrendButKeepsCompositionAndHierarchyAvailable() {
        composeRule.setContent {
            MaterialTheme {
                InsightsChartSemanticModeSelector(
                    insightsMode = InsightsMode.DAY,
                    chartSemanticMode = InsightsChartSemanticMode.COMPOSITION,
                    onChartSemanticModeChange = {}
                )
            }
        }

        composeRule.onNodeWithText(context.getString(R.string.insights_chart_semantic_trend))
            .assertDoesNotExist()
        composeRule.onNodeWithText(context.getString(R.string.insights_chart_semantic_composition))
            .assertExists()
        composeRule.onNodeWithText(context.getString(R.string.insights_chart_semantic_hierarchy))
            .assertExists()
    }

    @Test
    fun nonDayKeepsTrendAvailable() {
        composeRule.setContent {
            MaterialTheme {
                InsightsChartSemanticModeSelector(
                    insightsMode = InsightsMode.WEEK,
                    chartSemanticMode = InsightsChartSemanticMode.TREND,
                    onChartSemanticModeChange = {}
                )
            }
        }

        composeRule.onNodeWithText(context.getString(R.string.insights_chart_semantic_trend))
            .assertExists()
    }
}
