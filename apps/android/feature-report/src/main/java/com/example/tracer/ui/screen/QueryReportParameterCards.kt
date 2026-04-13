package com.example.tracer

import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.runtime.Composable

@Composable
internal fun QueryReportParameterCards(
    reportMode: ReportMode,
    resultDisplayMode: ReportResultDisplayMode,
    analysisPeriod: DataTreePeriod,
    reportExpanded: Boolean,
    statsExpanded: Boolean,
    treeExpanded: Boolean,
    treeLevel: String,
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
    analysisLoading: Boolean,
    onReportExpandedChange: (Boolean) -> Unit,
    onStatsExpandedChange: (Boolean) -> Unit,
    onTreeExpandedChange: (Boolean) -> Unit,
    onTreeLevelChange: (String) -> Unit,
    onRunReport: () -> Unit,
    onLoadStats: () -> Unit,
    onLoadTree: () -> Unit
) {
    ReportParametersCard(
        reportMode = reportMode,
        resultDisplayMode = resultDisplayMode,
        expanded = reportExpanded,
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
        onReportRecentDaysChange = onReportRecentDaysChange,
        onToggle = { onReportExpandedChange(!reportExpanded) },
        onRunReport = onRunReport
    )

    if (resultDisplayMode == ReportResultDisplayMode.TEXT) {
        StatsParametersCard(
            analysisPeriod = analysisPeriod,
            expanded = statsExpanded,
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
            onReportRecentDaysChange = onReportRecentDaysChange,
            analysisLoading = analysisLoading,
            onToggle = { onStatsExpandedChange(!statsExpanded) },
            onLoadStats = onLoadStats
        )

        TreeParametersCard(
            analysisPeriod = analysisPeriod,
            expanded = treeExpanded,
            treeLevel = treeLevel,
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
            onReportRecentDaysChange = onReportRecentDaysChange,
            analysisLoading = analysisLoading,
            onTreeLevelChange = onTreeLevelChange,
            onToggle = { onTreeExpandedChange(!treeExpanded) },
            onLoadTree = onLoadTree
        )
    }
}
