package com.example.tracer

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.animation.expandVertically
import androidx.compose.animation.shrinkVertically
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ExpandLess
import androidx.compose.material.icons.filled.ExpandMore
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.report.R
import com.example.tracer.ui.components.TracerOutlinedTextFieldDefaults

@Composable
internal fun SegmentedAnalysisPeriodInputs(
    section: String,
    period: DataTreePeriod,
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
    onReportRecentDaysChange: (String) -> Unit
) {
    ReportTemporalInputFields(
        period = period,
        labels = TemporalInputLabels(
            dayTitle = stringResource(R.string.report_title_section_day, section),
            monthTitle = stringResource(R.string.report_title_section_month, section),
            weekTitle = stringResource(R.string.report_title_section_week, section),
            yearLabel = stringResource(R.string.report_label_section_year, section),
            rangeStartTitle = stringResource(R.string.report_title_section_start_date, section),
            rangeEndTitle = stringResource(R.string.report_title_section_end_date, section),
            recentDaysLabel = stringResource(R.string.report_label_section_recent_days, section)
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
}

@Composable
internal fun ExpandableParameterCard(
    title: String,
    expanded: Boolean,
    onToggle: () -> Unit,
    content: @Composable () -> Unit
) {
    ElevatedCard(
        modifier = Modifier.fillMaxWidth(),
        shape = MaterialTheme.shapes.extraLarge
    ) {
        Column(modifier = Modifier.fillMaxWidth()) {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .clickable { onToggle() }
                    .padding(16.dp),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = title,
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.primary
                )
                Icon(
                    imageVector = if (expanded) Icons.Default.ExpandLess else Icons.Default.ExpandMore,
                    contentDescription = if (expanded) {
                        stringResource(R.string.report_cd_collapse)
                    } else {
                        stringResource(R.string.report_cd_expand)
                    },
                    tint = MaterialTheme.colorScheme.primary
                )
            }
            AnimatedVisibility(
                visible = expanded,
                enter = expandVertically(),
                exit = shrinkVertically()
            ) {
                Column(
                    modifier = Modifier
                        .padding(horizontal = 16.dp)
                        .padding(bottom = 16.dp),
                    verticalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    content()
                }
            }
        }
    }
}
