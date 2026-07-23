package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.runtime.Composable
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.report.R

@Composable
internal fun ReportParametersCard(
    reportMode: ReportMode,
    keyboardOptions: KeyboardOptions,
    reportDate: String,
    onReportDateChange: (String) -> Unit,
    reportMonth: String,
    onReportMonthChange: (String) -> Unit,
    calendarAvailability: com.example.tracer.ui.components.CalendarAvailability,
    reportYear: String,
    onReportYearChange: (String) -> Unit,
    reportWeek: String,
    onReportWeekChange: (String) -> Unit,
    reportRangeStartDate: String,
    onReportRangeStartDateChange: (String) -> Unit,
    reportRangeEndDate: String,
    onReportRangeEndDateChange: (String) -> Unit,
    reportRecentDays: String,
    onReportRecentDaysChange: (String) -> Unit,
    expanded: Boolean,
    onExpandedChange: (Boolean) -> Unit
) {
    TemporalParametersCard(
        title = stringResource(
            R.string.report_title_mode_parameters,
            stringResource(reportMode.labelRes())
        ),
        period = reportMode.toDataTreePeriod(),
        keyboardOptions = keyboardOptions,
        reportDate = reportDate,
        onReportDateChange = onReportDateChange,
        reportMonth = reportMonth,
        onReportMonthChange = onReportMonthChange,
        calendarAvailability = calendarAvailability,
        reportYear = reportYear,
        onReportYearChange = onReportYearChange,
        reportWeek = reportWeek,
        onReportWeekChange = onReportWeekChange,
        reportRangeStartDate = reportRangeStartDate,
        onReportRangeStartDateChange = onReportRangeStartDateChange,
        reportRangeEndDate = reportRangeEndDate,
        onReportRangeEndDateChange = onReportRangeEndDateChange,
        reportRecentDays = reportRecentDays,
        onReportRecentDaysChange = onReportRecentDaysChange,
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
    reportDate: String,
    onReportDateChange: (String) -> Unit,
    reportMonth: String,
    onReportMonthChange: (String) -> Unit,
    calendarAvailability: com.example.tracer.ui.components.CalendarAvailability,
    reportYear: String,
    onReportYearChange: (String) -> Unit,
    reportWeek: String,
    onReportWeekChange: (String) -> Unit,
    reportRangeStartDate: String,
    onReportRangeStartDateChange: (String) -> Unit,
    reportRangeEndDate: String,
    onReportRangeEndDateChange: (String) -> Unit,
    reportRecentDays: String,
    onReportRecentDaysChange: (String) -> Unit,
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
            ReportTemporalInputFields(
                period = period,
                labels = TemporalInputLabels(
                    dayTitle = stringResource(R.string.report_title_report_day),
                    monthTitle = stringResource(R.string.report_title_report_month),
                    weekTitle = stringResource(R.string.report_title_report_week),
                    yearLabel = stringResource(R.string.report_label_report_year),
                    rangeStartTitle = stringResource(R.string.report_title_start_date),
                    rangeEndTitle = stringResource(R.string.report_title_end_date),
                    recentDaysLabel = stringResource(R.string.report_label_recent_days)
                ),
                keyboardOptions = keyboardOptions,
                reportDate = reportDate,
                onReportDateChange = onReportDateChange,
                reportMonth = reportMonth,
                onReportMonthChange = onReportMonthChange,
                calendarAvailability = calendarAvailability,
                reportYear = reportYear,
                onReportYearChange = onReportYearChange,
                reportWeek = reportWeek,
                onReportWeekChange = onReportWeekChange,
                reportRangeStartDate = reportRangeStartDate,
                onReportRangeStartDateChange = onReportRangeStartDateChange,
                reportRangeEndDate = reportRangeEndDate,
                onReportRangeEndDateChange = onReportRangeEndDateChange,
                reportRecentDays = reportRecentDays,
                onReportRecentDaysChange = onReportRecentDaysChange
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
