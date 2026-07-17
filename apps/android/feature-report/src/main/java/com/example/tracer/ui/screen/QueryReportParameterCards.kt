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
import com.example.tracer.feature.report.R
import com.example.tracer.ui.components.TracerSegmentedButtonDefaults

enum class ReportParameterSection {
    DAY,
    TREE,
    STATS
}

@Composable
internal fun QueryReportParameterCards(
    reportMode: ReportMode,
    resultDisplayMode: ReportResultDisplayMode,
    analysisPeriod: DataTreePeriod,
    selectedSection: ReportParameterSection,
    treeLevel: String,
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
    analysisLoading: Boolean,
    onSelectedSectionChange: (ReportParameterSection) -> Unit,
    onTreeLevelChange: (String) -> Unit,
    onRunReport: () -> Unit,
    onLoadStats: () -> Unit,
    onLoadTree: () -> Unit
) {
    if (resultDisplayMode == ReportResultDisplayMode.TEXT) {
        ReportParameterSectionSelector(
            reportMode = reportMode,
            selectedSection = selectedSection,
            onSelectedSectionChange = onSelectedSectionChange
        )
    }

    when (if (resultDisplayMode == ReportResultDisplayMode.TEXT) {
        selectedSection
    } else {
        ReportParameterSection.DAY
    }) {
        ReportParameterSection.DAY -> ReportParametersCard(
            reportMode = reportMode,
            resultDisplayMode = resultDisplayMode,
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
            onRunReport = onRunReport
        )

        ReportParameterSection.STATS -> StatsParametersCard(
            analysisPeriod = analysisPeriod,
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
            analysisLoading = analysisLoading,
            onLoadStats = onLoadStats
        )

        ReportParameterSection.TREE -> TreeParametersCard(
            analysisPeriod = analysisPeriod,
            treeLevel = treeLevel,
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
            analysisLoading = analysisLoading,
            onTreeLevelChange = onTreeLevelChange,
            onLoadTree = onLoadTree
        )
    }
}

@Composable
private fun ReportParameterSectionSelector(
    reportMode: ReportMode,
    selectedSection: ReportParameterSection,
    onSelectedSectionChange: (ReportParameterSection) -> Unit
) {
    val sections = ReportParameterSection.entries
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
                        if (section == ReportParameterSection.DAY) {
                            stringResource(reportMode.labelRes())
                        } else {
                            stringResource(section.labelRes())
                        }
                    )
                }
            )
        }
    }
}

private fun ReportParameterSection.labelRes(): Int = when (this) {
    ReportParameterSection.DAY -> R.string.report_mode_day
    ReportParameterSection.TREE -> R.string.report_parameter_section_tree
    ReportParameterSection.STATS -> R.string.report_parameter_section_stats
}
