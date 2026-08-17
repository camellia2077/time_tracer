package com.example.tracer

import android.content.Context
import androidx.compose.material3.MaterialTheme
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.test.getUnclippedBoundsInRoot
import androidx.compose.ui.test.junit4.createComposeRule
import androidx.compose.ui.test.onNodeWithTag
import androidx.compose.ui.test.onNodeWithText
import androidx.test.core.app.ApplicationProvider
import com.example.tracer.feature.insights.R
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class InsightsModeTabsRobolectricTest {
    @get:Rule
    val composeRule = createComposeRule()

    private val context: Context = ApplicationProvider.getApplicationContext()

    @Test
    fun selectedIndicator_extendsBeyondAndRemainsCenteredToItsLabel() {
        val selectedMode = InsightsMode.MONTH
        val selectedLabel = context.getString(R.string.insights_mode_month)

        composeRule.setContent {
            MaterialTheme {
                StaticScrollableTextTabRow(
                    labels = InsightsMode.entries.map { context.getString(it.labelRes()) },
                    selectedIndex = InsightsMode.entries.indexOf(selectedMode),
                    onSelectedIndexChange = {},
                    indicatorModifier = Modifier.testTag("insights-mode-indicator")
                )
            }
        }

        val indicatorBounds = composeRule.onNodeWithTag("insights-mode-indicator")
            .getUnclippedBoundsInRoot()
        val labelBounds = composeRule.onNodeWithText(selectedLabel, useUnmergedTree = true)
            .getUnclippedBoundsInRoot()

        val indicatorWidth = indicatorBounds.right - indicatorBounds.left
        val labelWidth = labelBounds.right - labelBounds.left
        val indicatorCenter = (indicatorBounds.left + indicatorBounds.right) / 2f
        val labelCenter = (labelBounds.left + labelBounds.right) / 2f

        assertTrue(indicatorWidth.value >= labelWidth.value)
        assertEquals(labelCenter.value, indicatorCenter.value, 1f)
    }
}
