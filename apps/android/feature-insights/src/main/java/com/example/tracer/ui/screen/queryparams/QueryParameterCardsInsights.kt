package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.runtime.Composable
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.insights.R

@Composable
internal fun InsightsParametersCard(
    insightsMode: InsightsMode,
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
    expanded: Boolean,
    onExpandedChange: (Boolean) -> Unit
) {
    TemporalParametersCard(
        title = stringResource(
            R.string.insights_title_mode_parameters,
            stringResource(insightsMode.labelRes())
        ),
        period = insightsMode.toDataTreePeriod(),
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
        expanded = expanded,
        onExpandedChange = onExpandedChange
    )
}

@Composable
internal fun TemporalParametersCard(
    title: String,
    period: DataTreePeriod,
    maxAvailableDepth: Int = 0,
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
    expanded: Boolean,
    onExpandedChange: (Boolean) -> Unit,
    treeLevel: String? = null,
    onTreeLevelChange: ((String) -> Unit)? = null
) {
    ExpandableParameterCard(
        title = title,
        expanded = expanded,
        onToggle = { onExpandedChange(!expanded) }
    ) {
        Column(verticalArrangement = Arrangement.spacedBy(12.dp)) {
            InsightsTemporalInputFields(
                period = period,
                labels = TemporalInputLabels(
                    dayTitle = stringResource(R.string.insights_title_insights_day),
                    monthTitle = stringResource(R.string.insights_title_insights_month),
                    weekTitle = stringResource(R.string.insights_title_insights_week),
                    yearLabel = stringResource(R.string.insights_label_insights_year),
                    rangeStartTitle = stringResource(R.string.insights_title_start_date),
                    rangeEndTitle = stringResource(R.string.insights_title_end_date),
                    recentDaysLabel = stringResource(R.string.insights_label_recent_days)
                ),
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
                onInsightsRecentDaysChange = onInsightsRecentDaysChange
            )
            if (treeLevel != null && onTreeLevelChange != null) {
                TreeLevelDropdown(
                    treeLevel = treeLevel,
                    maxAvailableDepth = maxAvailableDepth,
                    keyboardOptions = keyboardOptions,
                    onTreeLevelChange = onTreeLevelChange
                )
            }
        }
    }
}
