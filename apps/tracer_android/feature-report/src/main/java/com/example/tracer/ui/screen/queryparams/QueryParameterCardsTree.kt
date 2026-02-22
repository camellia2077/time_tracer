package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material3.Button
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.report.R

@Composable
internal fun TreeParametersCard(
    analysisPeriod: DataTreePeriod,
    expanded: Boolean,
    treeLevel: String,
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
    onTreeLevelChange: (String) -> Unit,
    onToggle: () -> Unit,
    onLoadTree: () -> Unit
) {
    ExpandableParameterCard(
        title = stringResource(R.string.report_title_project_tree_parameters),
        expanded = expanded,
        onToggle = onToggle
    ) {
        Column(verticalArrangement = Arrangement.spacedBy(12.dp)) {
            SegmentedAnalysisPeriodInputs(
                section = stringResource(R.string.report_section_tree),
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
            OutlinedTextField(
                value = treeLevel,
                onValueChange = onTreeLevelChange,
                label = { Text(stringResource(R.string.report_label_tree_level)) },
                singleLine = true,
                keyboardOptions = keyboardOptions,
                modifier = Modifier.fillMaxWidth()
            )
            Button(
                onClick = onLoadTree,
                enabled = !analysisLoading,
                modifier = Modifier.fillMaxWidth()
            ) {
                val periodLabel = stringResource(analysisPeriod.labelRes())
                Text(
                    if (analysisLoading) {
                        stringResource(R.string.report_action_loading)
                    } else {
                        stringResource(R.string.report_action_load_project_tree, periodLabel)
                    }
                )
            }
        }
    }
}
