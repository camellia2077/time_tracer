package com.example.tracer

import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import com.example.tracer.feature.insights.R
import com.example.tracer.ui.components.TracerSegmentedButtonDefaults

enum class InsightsParameterSection {
    ACTIVITIES,
    DAY
}

@Composable
internal fun QueryInsightsParameterCards(
    insightsMode: InsightsMode,
    resultDisplayMode: InsightsResultDisplayMode,
    selectedChartSemanticMode: InsightsChartSemanticMode,
    analysisPeriod: DataTreePeriod,
    selectedSection: InsightsParameterSection,
    treeMaxAvailableDepth: Int,
    treeLevel: String,
    keyboardOptions: KeyboardOptions,
    insightsDate: String,
    onInsightsDateChange: (String) -> Unit,
    insightsMonth: String,
    onInsightsMonthChange: (String) -> Unit,
    calendarAvailability: com.example.tracer.ui.components.CalendarAvailability,
    insightsYear: String,
    onInsightsYearChange: (String) -> Unit,
    insightsWeek: String,
    onInsightsWeekChange: (String) -> Unit,
    insightsRangeStartDate: String,
    onInsightsRangeStartDateChange: (String) -> Unit,
    insightsRangeEndDate: String,
    onInsightsRangeEndDateChange: (String) -> Unit,
    insightsRecentDays: String,
    onInsightsRecentDaysChange: (String) -> Unit,
    onInsightsActivityPeriodConfirmed: (InsightsPeriodSelection) -> Unit = {},
    timeParametersExpanded: Boolean,
    onTimeParametersExpandedChange: (Boolean) -> Unit,
    onSelectedSectionChange: (InsightsParameterSection) -> Unit,
    onTreeLevelChange: (String) -> Unit
) {
    val availableSections = listOf(
        InsightsParameterSection.ACTIVITIES,
        InsightsParameterSection.DAY
    )

    if (resultDisplayMode == InsightsResultDisplayMode.DETAILS) {
        InsightsParameterSectionSelector(
            selectedSection = selectedSection,
            sections = availableSections,
            onSelectedSectionChange = onSelectedSectionChange
        )
    }

    val showHierarchyParameters = resultDisplayMode == InsightsResultDisplayMode.CHART &&
        selectedChartSemanticMode == InsightsChartSemanticMode.HIERARCHY

    if (showHierarchyParameters) {
        TreeParametersCard(
            analysisPeriod = analysisPeriod,
            maxAvailableDepth = treeMaxAvailableDepth,
            treeLevel = treeLevel,
            keyboardOptions = keyboardOptions,
            insightsDate = insightsDate,
            onInsightsDateChange = onInsightsDateChange,
            insightsMonth = insightsMonth,
            onInsightsMonthChange = onInsightsMonthChange,
            calendarAvailability = calendarAvailability,
            insightsYear = insightsYear,
            onInsightsYearChange = onInsightsYearChange,
            insightsWeek = insightsWeek,
            onInsightsWeekChange = onInsightsWeekChange,
            insightsRangeStartDate = insightsRangeStartDate,
            onInsightsRangeStartDateChange = onInsightsRangeStartDateChange,
            insightsRangeEndDate = insightsRangeEndDate,
            onInsightsRangeEndDateChange = onInsightsRangeEndDateChange,
            insightsRecentDays = insightsRecentDays,
            onInsightsRecentDaysChange = onInsightsRecentDaysChange,
            onTreeLevelChange = onTreeLevelChange,
            expanded = timeParametersExpanded,
            onExpandedChange = onTimeParametersExpandedChange
        )
        return
    }

    val contentSection = if (resultDisplayMode == InsightsResultDisplayMode.DETAILS) {
        selectedSection
    } else {
        InsightsParameterSection.DAY
    }

    if (contentSection == InsightsParameterSection.ACTIVITIES &&
        insightsMode in setOf(InsightsMode.DAY, InsightsMode.WEEK, InsightsMode.MONTH, InsightsMode.YEAR)
    ) {
        InsightsActivitiesPeriodSelector(
            insightsMode = insightsMode,
            keyboardOptions = keyboardOptions,
            insightsDate = insightsDate,
            insightsMonth = insightsMonth,
            calendarAvailability = calendarAvailability,
            insightsYear = insightsYear,
            insightsWeek = insightsWeek,
            onPeriodConfirmed = onInsightsActivityPeriodConfirmed
        )
        return
    }

    InsightsParametersCard(
        insightsMode = insightsMode,
        keyboardOptions = keyboardOptions,
        insightsDate = insightsDate,
        onInsightsDateChange = onInsightsDateChange,
        insightsMonth = insightsMonth,
        onInsightsMonthChange = onInsightsMonthChange,
        calendarAvailability = calendarAvailability,
        insightsYear = insightsYear,
        onInsightsYearChange = onInsightsYearChange,
        insightsWeek = insightsWeek,
        onInsightsWeekChange = onInsightsWeekChange,
        insightsRangeStartDate = insightsRangeStartDate,
        onInsightsRangeStartDateChange = onInsightsRangeStartDateChange,
        insightsRangeEndDate = insightsRangeEndDate,
        onInsightsRangeEndDateChange = onInsightsRangeEndDateChange,
        insightsRecentDays = insightsRecentDays,
        onInsightsRecentDaysChange = onInsightsRecentDaysChange,
        expanded = timeParametersExpanded,
        onExpandedChange = onTimeParametersExpandedChange
    )
}

@Composable
private fun InsightsParameterSectionSelector(
    selectedSection: InsightsParameterSection,
    sections: List<InsightsParameterSection>,
    onSelectedSectionChange: (InsightsParameterSection) -> Unit
) {
    SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
        sections.forEachIndexed { index, section ->
            SegmentedButton(
                shape = SegmentedButtonDefaults.itemShape(
                    index = index,
                    count = sections.size
                ),
                onClick = { onSelectedSectionChange(section) },
                selected = section == selectedSection,
                colors = TracerSegmentedButtonDefaults.colors(),
                label = {
                    Text(
                        if (section == InsightsParameterSection.DAY) {
                            stringResource(R.string.insights_parameter_section_markdown)
                        } else {
                            stringResource(section.labelRes())
                        }
                    )
                }
            )
        }
    }
}

private fun InsightsParameterSection.labelRes(): Int = when (this) {
    InsightsParameterSection.DAY -> R.string.insights_parameter_section_markdown
    InsightsParameterSection.ACTIVITIES -> R.string.insights_parameter_section_activity
}
