package com.example.tracer

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ChevronRight
import androidx.compose.material.icons.filled.DateRange
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
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
import com.example.tracer.ui.components.formatWeekRangeText
import java.time.LocalDate

@Composable
internal fun InsightsActivitiesPeriodSelector(
    insightsMode: InsightsMode,
    keyboardOptions: androidx.compose.foundation.text.KeyboardOptions,
    insightsDate: String,
    insightsMonth: String,
    calendarAvailability: CalendarAvailability,
    insightsYear: String,
    insightsWeek: String,
    onPeriodConfirmed: (InsightsPeriodSelection) -> Unit
) {
    var visible by remember { mutableStateOf(false) }
    val currentDraft = InsightsPeriodSelection(
        date = insightsDate,
        month = insightsMonth,
        year = insightsYear,
        week = insightsWeek
    )
    var draft by remember {
        mutableStateOf(
            currentDraft
        )
    }
    val title = stringResource(R.string.insights_activities_period_selector_title)

    ParameterContentCard {
        OutlinedButton(
            onClick = {
                draft = currentDraft
                visible = true
            },
            modifier = Modifier.fillMaxWidth()
        ) {
            Icon(
                imageVector = Icons.Filled.DateRange,
                contentDescription = null,
                tint = MaterialTheme.colorScheme.primary
            )
            Column(
                modifier = Modifier
                    .weight(1f)
                    .padding(start = 12.dp),
                horizontalAlignment = Alignment.Start
            ) {
                Text(
                    text = title,
                    style = MaterialTheme.typography.labelMedium,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
                Text(
                    text = formatInsightsActivityPeriodSummary(insightsMode, currentDraft),
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.onSurface
                )
            }
            Icon(
                imageVector = Icons.Filled.ChevronRight,
                contentDescription = null,
                tint = MaterialTheme.colorScheme.onSurfaceVariant
            )
        }
    }

    if (visible) {
        InsightsPeriodPickerSheet(
            title = stringResource(
                R.string.insights_activities_period_selector_sheet_title,
                stringResource(insightsMode.labelRes())
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
                keyboardOptions = keyboardOptions,
                insightsDate = draft.date,
                onInsightsDateChange = {
                    val nextDraft = draft.copy(date = it)
                    onPeriodConfirmed(nextDraft)
                    visible = false
                },
                insightsMonth = draft.month,
                onInsightsMonthChange = {
                    val nextDraft = draft.copy(month = it)
                    if (insightsMode == InsightsMode.WEEK) {
                        draft = nextDraft
                    } else {
                        onPeriodConfirmed(nextDraft)
                        visible = false
                    }
                },
                calendarAvailability = calendarAvailability,
                insightsYear = draft.year,
                onInsightsYearChange = {
                    val nextDraft = draft.copy(year = it)
                    onPeriodConfirmed(nextDraft)
                    visible = false
                },
                insightsWeek = draft.week,
                onInsightsWeekChange = {
                    val nextDraft = draft.copy(week = it)
                    onPeriodConfirmed(nextDraft)
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

internal fun formatInsightsActivityPeriodSummary(
    insightsMode: InsightsMode,
    draft: InsightsPeriodSelection
): String = when (insightsMode) {
    InsightsMode.DAY -> draft.date.takeIf { it.length == 8 }?.let {
        runCatching { LocalDate.of(it.take(4).toInt(), it.substring(4, 6).toInt(), it.takeLast(2).toInt()).toString() }
            .getOrDefault(it)
    } ?: draft.date
    InsightsMode.WEEK -> resolveIsoWeekSelection(draft.week)?.let { selection ->
        "${formatWeekRangeText(selection.weekStart, selection.weekEnd)} · W${draft.week.takeLast(2)}"
    } ?: draft.week
    InsightsMode.MONTH -> draft.month.takeIf { it.length == 6 }?.let {
        "${it.take(4)}-${it.takeLast(2)}"
    } ?: draft.month
    InsightsMode.YEAR -> draft.year
    InsightsMode.RANGE, InsightsMode.RECENT -> ""
}
