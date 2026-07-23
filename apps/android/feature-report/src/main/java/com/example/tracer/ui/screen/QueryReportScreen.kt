package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material3.PrimaryScrollableTabRow
import androidx.compose.material3.Tab
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.report.R

enum class ReportMode {
    DAY,
    WEEK,
    MONTH,
    YEAR,
    RANGE,
    RECENT
}

@Composable
internal fun QueryReportSection(
    showModeTabs: Boolean = true,
    reportMode: ReportMode,
    onReportModeChange: (ReportMode) -> Unit,
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
    resultDisplayMode: ReportResultDisplayMode,
    onResultDisplayModeChange: (ReportResultDisplayMode) -> Unit,
    chartSemanticMode: ReportChartSemanticMode,
    onChartSemanticModeChange: (ReportChartSemanticMode) -> Unit,
    selectedParameterSection: ReportParameterSection,
    treeLevel: Int,
    treeMaxAvailableDepth: Int,
    onSelectedParameterSectionChange: (ReportParameterSection) -> Unit,
    onTreeLevelChange: (Int) -> Unit,
    timeParametersExpanded: Boolean,
    onTimeParametersExpandedChange: (Boolean) -> Unit
) {
    val analysisPeriod = reportMode.toDataTreePeriod()
    val reportModes = ReportMode.entries
    val selectedIndex = reportModes.indexOf(reportMode)
    val numericKeyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number)

    Column(
        modifier = Modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        if (showModeTabs) {
            ReportModeTabs(
                selectedIndex = selectedIndex,
                reportModes = reportModes,
                reportMode = reportMode,
                onReportModeChange = onReportModeChange
            )
        }
        Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
            ReportResultModeSwitcher(
                mode = resultDisplayMode,
                onModeChange = onResultDisplayModeChange,
                modifier = Modifier.fillMaxWidth()
            )
            if (resultDisplayMode == ReportResultDisplayMode.CHART &&
                reportMode != ReportMode.DAY
            ) {
                ReportChartSemanticModeSelector(
                    chartSemanticMode = chartSemanticMode,
                    onChartSemanticModeChange = onChartSemanticModeChange
                )
            }

            QueryReportParameterCards(
                reportMode = reportMode,
                resultDisplayMode = resultDisplayMode,
                analysisPeriod = analysisPeriod,
                selectedSection = selectedParameterSection,
                treeMaxAvailableDepth = treeMaxAvailableDepth,
                treeLevel = treeLevel.toString(),
                keyboardOptions = numericKeyboardOptions,
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
                timeParametersExpanded = timeParametersExpanded,
                onTimeParametersExpandedChange = onTimeParametersExpandedChange,
                onSelectedSectionChange = onSelectedParameterSectionChange,
                onTreeLevelChange = {
                    val normalizedLevel = normalizeSignedIntInput(it, 3)
                    onTreeLevelChange(normalizedLevel.toIntOrNull() ?: -1)
                }
            )
        }
    }
}

@Composable
internal fun ReportModeTabs(
    selectedIndex: Int,
    reportModes: List<ReportMode>,
    reportMode: ReportMode,
    onReportModeChange: (ReportMode) -> Unit
) {
    PrimaryScrollableTabRow(
        selectedTabIndex = selectedIndex,
        modifier = Modifier.fillMaxWidth(),
        edgePadding = 0.dp
    ) {
        reportModes.forEach { mode ->
            Tab(
                selected = reportMode == mode,
                onClick = { onReportModeChange(mode) },
                text = {
                    Text(
                        text = stringResource(mode.labelRes()),
                        maxLines = 1,
                        softWrap = false
                    )
                }
            )
        }
    }
}

internal fun ReportMode.toDataTreePeriod(): DataTreePeriod =
    when (this) {
        ReportMode.DAY -> DataTreePeriod.DAY
        ReportMode.WEEK -> DataTreePeriod.WEEK
        ReportMode.MONTH -> DataTreePeriod.MONTH
        ReportMode.YEAR -> DataTreePeriod.YEAR
        ReportMode.RANGE -> DataTreePeriod.RANGE
        ReportMode.RECENT -> DataTreePeriod.RECENT
    }

private fun normalizeSignedIntInput(value: String, maxDigits: Int): String {
    if (value.isEmpty()) {
        return ""
    }
    val trimmed = value.trim()
    if (trimmed == "-") {
        return "-"
    }
    val isNegative = trimmed.startsWith("-")
    val digits = trimmed.filter { it.isDigit() }.take(maxDigits)
    return if (isNegative) "-$digits" else digits
}
