package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material3.Button
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.report.R

@Composable
internal fun StatsParametersCard(
    analysisPeriod: DataTreePeriod,
    expanded: Boolean,
    keyboardOptions: KeyboardOptions,
    reportDate: String,
    onReportDateChange: (String) -> Unit,
    reportMonth: String,
    onReportMonthChange: (String) -> Unit,
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
    analysisLoading: Boolean,
    onToggle: () -> Unit,
    onLoadStats: () -> Unit
) {
    ExpandableParameterCard(
        title = stringResource(R.string.report_title_stats_parameters),
        expanded = expanded,
        onToggle = onToggle
    ) {
        Column(verticalArrangement = Arrangement.spacedBy(12.dp)) {
            SegmentedAnalysisPeriodInputs(
                section = stringResource(R.string.report_section_stats),
                period = analysisPeriod,
                keyboardOptions = keyboardOptions,
                reportDate = reportDate,
                onReportDateChange = onReportDateChange,
                reportMonth = reportMonth,
                onReportMonthChange = onReportMonthChange,
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
            Button(
                onClick = onLoadStats,
                enabled = !analysisLoading,
                modifier = Modifier.fillMaxWidth()
            ) {
                val periodLabel = stringResource(analysisPeriod.labelRes())
                Text(
                    if (analysisLoading) {
                        stringResource(R.string.report_action_loading)
                    } else {
                        stringResource(R.string.report_action_generate_stats, periodLabel)
                    }
                )
            }
        }
    }
}
