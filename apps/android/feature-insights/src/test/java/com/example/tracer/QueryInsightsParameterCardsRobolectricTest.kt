package com.example.tracer

import android.content.Context
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material3.MaterialTheme
import androidx.compose.ui.test.assertIsDisplayed
import androidx.compose.ui.test.assertCountEquals
import androidx.compose.ui.test.junit4.createComposeRule
import androidx.compose.ui.test.onAllNodesWithContentDescription
import androidx.compose.ui.test.onAllNodesWithText
import androidx.compose.ui.test.onNodeWithContentDescription
import androidx.compose.ui.test.onNodeWithText
import androidx.compose.ui.test.performClick
import androidx.test.core.app.ApplicationProvider
import com.example.tracer.feature.insights.R
import com.example.tracer.ui.components.CalendarAvailability
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class QueryInsightsParameterCardsRobolectricTest {
    @get:Rule
    val composeRule = createComposeRule()

    private val context: Context = ApplicationProvider.getApplicationContext()

    @Test
    fun detailsActivities_usesTheSharedCollapsibleDateCardAndPickerSheet() {
        composeRule.setContent {
            MaterialTheme {
                var parametersExpanded by remember { mutableStateOf(true) }
                QueryInsightsParameterCards(
                    insightsMode = InsightsMode.DAY,
                    resultDisplayMode = InsightsResultDisplayMode.DETAILS,
                    selectedChartSemanticMode = InsightsChartSemanticMode.COMPOSITION,
                    analysisPeriod = DataTreePeriod.DAY,
                    selectedSection = InsightsParameterSection.ACTIVITIES,
                    treeMaxAvailableDepth = 0,
                    treeLevel = "-1",
                    keyboardOptions = KeyboardOptions.Default,
                    insightsDate = "20260911",
                    onInsightsDateChange = {},
                    insightsMonth = "202609",
                    onInsightsMonthChange = {},
                    calendarAvailability = CalendarAvailability.fromMonthKeys(
                        listOf("2026-09")
                    ),
                    insightsYear = "2026",
                    onInsightsYearChange = {},
                    insightsWeek = "202637",
                    onInsightsWeekChange = {},
                    insightsRangeStartDate = "20260901",
                    onInsightsRangeStartDateChange = {},
                    insightsRangeEndDate = "20260911",
                    onInsightsRangeEndDateChange = {},
                    insightsRecentDays = "7",
                    onInsightsRecentDaysChange = {},
                    timeParametersExpanded = parametersExpanded,
                    onTimeParametersExpandedChange = { parametersExpanded = it },
                    onSelectedSectionChange = {},
                    onTreeLevelChange = {}
                )
            }
        }

        composeRule.onNodeWithText(
            context.getString(R.string.insights_title_mode_parameters)
        ).assertIsDisplayed()
        composeRule.onAllNodesWithText(
            context.getString(R.string.insights_activities_period_selector_title)
        ).assertCountEquals(0)

        composeRule.onNodeWithContentDescription(
            context.getString(R.string.insights_cd_collapse)
        ).performClick()
        composeRule.onAllNodesWithText(
            context.getString(R.string.insights_title_insights_day)
        ).assertCountEquals(0)
        composeRule.onNodeWithContentDescription(
            context.getString(R.string.insights_cd_expand)
        ).performClick()

        composeRule.onAllNodesWithContentDescription(
            context.getString(com.example.tracer.feature.uicommon.R.string.calendar_cd_select_day)
        )[0].performClick()

        composeRule.onNodeWithText(
            context.getString(R.string.insights_title_insights_month)
        ).assertIsDisplayed()
        composeRule.onAllNodesWithText(
            context.getString(R.string.insights_activities_period_selector_sheet_title, "Day")
        ).assertCountEquals(0)
    }
}
