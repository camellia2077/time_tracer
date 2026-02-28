package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material3.PrimaryScrollableTabRow
import androidx.compose.material3.Tab
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.report.R

internal enum class ReportMode {
    DAY,
    MONTH,
    WEEK,
    YEAR,
    RANGE,
    RECENT
}

@Composable
internal fun QueryReportSection(
    reportMode: ReportMode,
    onReportModeChange: (ReportMode) -> Unit,
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
    onReportDay: () -> Unit,
    onReportMonth: () -> Unit,
    onReportYear: () -> Unit,
    onReportWeek: () -> Unit,
    onReportRange: () -> Unit,
    onReportRecent: () -> Unit,
    resultDisplayMode: ReportResultDisplayMode,
    onResultDisplayModeChange: (ReportResultDisplayMode) -> Unit,
    analysisLoading: Boolean,
    onLoadDayStats: (DataTreePeriod) -> Unit,
    onLoadTree: (DataTreePeriod, Int) -> Unit
) {
    val analysisPeriod = reportMode.toDataTreePeriod()
    var treeLevel by rememberSaveable { mutableStateOf("-1") }

    var reportExpanded by rememberSaveable { mutableStateOf(true) }
    var statsExpanded by rememberSaveable { mutableStateOf(false) }
    var treeExpanded by rememberSaveable { mutableStateOf(false) }
    val reportModes = ReportMode.entries
    val selectedIndex = reportModes.indexOf(reportMode)
    val numericKeyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number)

    Column(
        modifier = Modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        ReportModeTabs(
            selectedIndex = selectedIndex,
            reportModes = reportModes,
            reportMode = reportMode,
            onReportModeChange = onReportModeChange
        )
        ReportResultModeSwitcher(
            mode = resultDisplayMode,
            onModeChange = onResultDisplayModeChange,
            modifier = Modifier.fillMaxWidth()
        )
        Spacer(modifier = Modifier.height(4.dp))

        if (resultDisplayMode == ReportResultDisplayMode.TEXT) {
            QueryReportParameterCards(
                reportMode = reportMode,
                analysisPeriod = analysisPeriod,
                reportExpanded = reportExpanded,
                statsExpanded = statsExpanded,
                treeExpanded = treeExpanded,
                treeLevel = treeLevel,
                keyboardOptions = numericKeyboardOptions,
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
                onReportRecentDaysChange = onReportRecentDaysChange,
                analysisLoading = analysisLoading,
                onReportExpandedChange = { reportExpanded = it },
                onStatsExpandedChange = { statsExpanded = it },
                onTreeExpandedChange = { treeExpanded = it },
                onTreeLevelChange = { treeLevel = normalizeSignedIntInput(it, 3) },
                onRunReport = {
                    runReportForMode(
                        reportMode = reportMode,
                        onReportDay = onReportDay,
                        onReportMonth = onReportMonth,
                        onReportWeek = onReportWeek,
                        onReportYear = onReportYear,
                        onReportRange = onReportRange,
                        onReportRecent = onReportRecent
                    )
                    reportExpanded = false
                },
                onLoadStats = {
                    onLoadDayStats(analysisPeriod)
                    statsExpanded = false
                },
                onLoadTree = {
                    val parsedLevel = treeLevel.trim().toIntOrNull() ?: -1
                    onLoadTree(analysisPeriod, parsedLevel)
                    treeExpanded = false
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

private fun runReportForMode(
    reportMode: ReportMode,
    onReportDay: () -> Unit,
    onReportMonth: () -> Unit,
    onReportWeek: () -> Unit,
    onReportYear: () -> Unit,
    onReportRange: () -> Unit,
    onReportRecent: () -> Unit
) {
    when (reportMode) {
        ReportMode.DAY -> onReportDay()
        ReportMode.MONTH -> onReportMonth()
        ReportMode.WEEK -> onReportWeek()
        ReportMode.YEAR -> onReportYear()
        ReportMode.RANGE -> onReportRange()
        ReportMode.RECENT -> onReportRecent()
    }
}

private fun ReportMode.toDataTreePeriod(): DataTreePeriod =
    when (this) {
        ReportMode.DAY -> DataTreePeriod.DAY
        ReportMode.MONTH -> DataTreePeriod.MONTH
        ReportMode.WEEK -> DataTreePeriod.WEEK
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
