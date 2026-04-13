package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.PlayArrow
import androidx.compose.material3.Button
import androidx.compose.material3.Icon
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.report.R

@Composable
internal fun ReportParametersCard(
    reportMode: ReportMode,
    resultDisplayMode: ReportResultDisplayMode,
    expanded: Boolean,
    keyboardOptions: KeyboardOptions,
    reportDate: String,
    onReportDateChange: (String) -> Unit,
    reportMonth: String,
    onReportMonthChange: (String) -> Unit,
    availableTxtYears: List<String>,
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
    onToggle: () -> Unit,
    onRunReport: () -> Unit
) {
    ExpandableParameterCard(
        title = stringResource(
            R.string.report_title_mode_parameters,
            stringResource(reportMode.labelRes())
        ),
        expanded = expanded,
        onToggle = onToggle
    ) {
        Column(verticalArrangement = Arrangement.spacedBy(12.dp)) {
            ReportTemporalInputFields(
                period = reportMode.toDataTreePeriod(),
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
                availableTxtYears = availableTxtYears,
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

            if (resultDisplayMode == ReportResultDisplayMode.TEXT) {
                Button(
                    onClick = onRunReport,
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Icon(Icons.Filled.PlayArrow, contentDescription = null)
                    Spacer(modifier = Modifier.width(8.dp))
                    Text(stringResource(R.string.report_action_generate_report_md))
                }
            }
        }
    }
}
