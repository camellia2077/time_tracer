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
    TIMELINE,
    DAY,
    ACTIVITY_HIERARCHY
}

@Composable
internal fun QueryReportParameterCards(
    reportMode: ReportMode,
    resultDisplayMode: ReportResultDisplayMode,
    analysisPeriod: DataTreePeriod,
    selectedSection: ReportParameterSection,
    treeMaxAvailableDepth: Int,
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
    timeParametersExpanded: Boolean,
    onTimeParametersExpandedChange: (Boolean) -> Unit,
    onSelectedSectionChange: (ReportParameterSection) -> Unit,
    onTreeLevelChange: (String) -> Unit
) {
    val availableSections = if (reportMode == ReportMode.DAY) {
        listOf(
            ReportParameterSection.TIMELINE,
            ReportParameterSection.ACTIVITY_HIERARCHY,
            ReportParameterSection.DAY
        )
    } else {
        listOf(
            ReportParameterSection.ACTIVITY_HIERARCHY,
            ReportParameterSection.DAY
        )
    }
    val effectiveSelectedSection = if (selectedSection in availableSections) {
        selectedSection
    } else {
        ReportParameterSection.DAY
    }

    if (resultDisplayMode == ReportResultDisplayMode.TEXT) {
        ReportParameterSectionSelector(
            selectedSection = effectiveSelectedSection,
            sections = availableSections,
            onSelectedSectionChange = onSelectedSectionChange
        )
    }

    when (if (resultDisplayMode == ReportResultDisplayMode.TEXT) {
        effectiveSelectedSection
    } else {
        ReportParameterSection.DAY
    }) {
        ReportParameterSection.DAY -> ReportParametersCard(
            reportMode = reportMode,
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
            expanded = timeParametersExpanded,
            onExpandedChange = onTimeParametersExpandedChange
        )

        ReportParameterSection.TIMELINE -> ReportParametersCard(
            reportMode = reportMode,
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
            expanded = timeParametersExpanded,
            onExpandedChange = onTimeParametersExpandedChange
        )

        ReportParameterSection.ACTIVITY_HIERARCHY -> TreeParametersCard(
            analysisPeriod = analysisPeriod,
            maxAvailableDepth = treeMaxAvailableDepth,
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
            onTreeLevelChange = onTreeLevelChange,
            expanded = timeParametersExpanded,
            onExpandedChange = onTimeParametersExpandedChange
        )
    }
}

@Composable
private fun ReportParameterSectionSelector(
    selectedSection: ReportParameterSection,
    sections: List<ReportParameterSection>,
    onSelectedSectionChange: (ReportParameterSection) -> Unit
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
                        if (section == ReportParameterSection.DAY) {
                            stringResource(R.string.report_parameter_section_markdown)
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
    ReportParameterSection.DAY -> R.string.report_parameter_section_markdown
    ReportParameterSection.ACTIVITY_HIERARCHY -> R.string.report_parameter_section_tree
    ReportParameterSection.TIMELINE -> R.string.report_parameter_section_timeline
}
