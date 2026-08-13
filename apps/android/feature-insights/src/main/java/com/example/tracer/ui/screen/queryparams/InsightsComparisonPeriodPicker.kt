package com.example.tracer

import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.DateRange
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.insights.R
import com.example.tracer.ui.components.CalendarAvailability

@Composable
internal fun InsightsComparisonPeriodPicker(
    insightsMode: InsightsMode,
    periodComparison: InsightsPeriodComparisonState,
    calendarAvailability: CalendarAvailability,
    onPeriodSelected: (InsightsPeriodSelection) -> Unit
) {
    if (insightsMode !in setOf(InsightsMode.DAY, InsightsMode.WEEK, InsightsMode.MONTH, InsightsMode.YEAR)) {
        return
    }
    val selected = periodComparison.selectionOrNull() ?: return
    var visible by remember { mutableStateOf(false) }
    var draft by remember(selected) { mutableStateOf(selected) }
    val openPicker = {
        draft = selected
        visible = true
    }

    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable(onClick = openPicker),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Icon(
            imageVector = Icons.Filled.DateRange,
            contentDescription = stringResource(R.string.insights_cd_select_comparison_period),
            tint = MaterialTheme.colorScheme.primary
        )
        Column(modifier = Modifier.padding(start = 4.dp)) {
            Text(
                text = stringResource(R.string.insights_period_activities_select_comparison_period),
                style = MaterialTheme.typography.labelMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            Text(
                text = formatInsightsActivityPeriodSummary(insightsMode, selected),
                style = MaterialTheme.typography.bodyLarge
            )
        }
    }

    if (visible) {
        InsightsPeriodPickerSheet(
            title = stringResource(
                R.string.insights_period_activities_comparison_period_picker_title
            ),
            onDismissRequest = { visible = false }
        ) {
            InsightsTemporalInputFields(
                period = insightsMode.toDataTreePeriod(),
                labels = TemporalInputLabels(
                    dayTitle = stringResource(R.string.insights_title_insights_day),
                    monthTitle = stringResource(R.string.insights_title_insights_month),
                    weekTitle = stringResource(R.string.insights_title_insights_week),
                    yearLabel = stringResource(R.string.insights_label_insights_year),
                    rangeStartTitle = stringResource(R.string.insights_title_start_date),
                    rangeEndTitle = stringResource(R.string.insights_title_end_date),
                    recentDaysLabel = stringResource(R.string.insights_label_recent_days)
                ),
                keyboardOptions = androidx.compose.foundation.text.KeyboardOptions.Default,
                insightsDate = draft.date,
                onInsightsDateChange = { date ->
                    val nextDraft = draft.copy(date = date)
                    onPeriodSelected(nextDraft)
                    visible = false
                },
                insightsMonth = draft.month,
                onInsightsMonthChange = { month ->
                    val nextDraft = draft.copy(month = month)
                    if (insightsMode == InsightsMode.WEEK) {
                        // A week picker uses the month only to navigate to a week. Wait for the
                        // actual week tap before replacing the comparison period.
                        draft = nextDraft
                    } else {
                        onPeriodSelected(nextDraft)
                        visible = false
                    }
                },
                yearMonthValueFormat = InsightsYearMonthValueFormat.ISO,
                calendarAvailability = calendarAvailability,
                insightsYear = draft.year,
                onInsightsYearChange = { year ->
                    val nextDraft = draft.copy(year = year)
                    onPeriodSelected(nextDraft)
                    visible = false
                },
                insightsWeek = draft.week,
                onInsightsWeekChange = { week ->
                    val nextDraft = draft.copy(week = week)
                    onPeriodSelected(nextDraft)
                    visible = false
                },
                insightsRangeStartDate = "",
                onInsightsRangeStartDateChange = {},
                insightsRangeEndDate = "",
                onInsightsRangeEndDateChange = {},
                insightsRecentDays = "",
                onInsightsRecentDaysChange = {}
            )
        }
    }
}
