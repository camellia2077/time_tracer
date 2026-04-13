package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp

@Composable
fun QueryReportTabContent(
    queryUiState: QueryReportUiState,
    queryReportViewModel: QueryReportViewModel,
    availableTxtMonths: List<String>,
    chartShowAverageLine: Boolean,
    onChartShowAverageLineChange: (Boolean) -> Unit,
    heatmapTomlConfig: ReportHeatmapTomlConfig,
    heatmapStylePreference: ReportHeatmapStylePreference,
    onHeatmapThemePolicyChange: (ReportHeatmapThemePolicy) -> Unit,
    onHeatmapPaletteNameChange: (String) -> Unit,
    heatmapApplyMessage: String,
    isAppDarkThemeActive: Boolean
) {
    // Report year menus intentionally follow existing TXT year directories so
    // users only pick years that actually back YYYY/YYYY-MM.txt storage.
    val availableTxtYears = deriveTxtYearOptions(availableTxtMonths)
    val selectedPeriod = queryUiState.reportMode.toPeriod()
    val displayResult = resolveDisplayResult(
        uiState = queryUiState,
        selectedPeriod = selectedPeriod
    )
    val displayReportSummary = resolveDisplayReportSummary(
        uiState = queryUiState,
        selectedPeriod = selectedPeriod
    )
    val displayReportError = resolveDisplayReportError(
        uiState = queryUiState,
        selectedPeriod = selectedPeriod
    )

    Column(
        modifier = Modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        QueryReportSection(
            reportMode = queryUiState.reportMode,
            onReportModeChange = queryReportViewModel::onReportModeChange,
            reportDate = queryUiState.reportDate,
            onReportDateChange = queryReportViewModel::onReportDateChange,
            reportMonth = queryUiState.reportMonth,
            onReportMonthChange = queryReportViewModel::onReportMonthChange,
            availableTxtYears = availableTxtYears,
            reportYear = queryUiState.reportYear,
            onReportYearChange = queryReportViewModel::onReportYearChange,
            reportWeek = queryUiState.reportWeek,
            onReportWeekChange = queryReportViewModel::onReportWeekChange,
            reportRangeStartDate = queryUiState.reportRangeStartDate,
            onReportRangeStartDateChange = queryReportViewModel::onReportRangeStartDateChange,
            reportRangeEndDate = queryUiState.reportRangeEndDate,
            onReportRangeEndDateChange = queryReportViewModel::onReportRangeEndDateChange,
            reportRecentDays = queryUiState.reportRecentDays,
            onReportRecentDaysChange = queryReportViewModel::onReportRecentDaysChange,
            onReportDay = queryReportViewModel::reportDay,
            onReportMonth = queryReportViewModel::reportMonth,
            onReportYear = queryReportViewModel::reportYear,
            onReportWeek = queryReportViewModel::reportWeek,
            onReportRange = queryReportViewModel::reportRange,
            onReportRecent = queryReportViewModel::reportRecent,
            resultDisplayMode = queryUiState.resultDisplayMode,
            onResultDisplayModeChange = queryReportViewModel::onResultDisplayModeChange,
            analysisLoading = queryUiState.analysisLoading,
            onLoadDayStats = queryReportViewModel::loadDayStats,
            onLoadTree = queryReportViewModel::loadTree
        )

        QueryReportResultDisplay(
            resultDisplayMode = queryUiState.resultDisplayMode,
            activeResult = displayResult,
            reportSummary = displayReportSummary,
            reportError = displayReportError,
            analysisError = queryUiState.analysisError,
            chartRoots = queryUiState.chartRoots,
            chartSelectedRoot = queryUiState.chartSelectedRoot,
            reportMode = queryUiState.reportMode,
            chartLoading = queryUiState.chartLoading,
            chartError = queryUiState.chartError,
            chartRenderModel = queryUiState.chartRenderModel,
            chartLastTrace = queryUiState.chartLastTrace,
            chartShowAverageLine = chartShowAverageLine,
            heatmapTomlConfig = heatmapTomlConfig,
            heatmapStylePreference = heatmapStylePreference,
            onHeatmapThemePolicyChange = onHeatmapThemePolicyChange,
            onHeatmapPaletteNameChange = onHeatmapPaletteNameChange,
            heatmapApplyMessage = heatmapApplyMessage,
            isAppDarkThemeActive = isAppDarkThemeActive,
            onChartRootChange = queryReportViewModel::onChartRootChange,
            onChartShowAverageLineChange = onChartShowAverageLineChange,
            onLoadChart = queryReportViewModel::loadChart
        )
    }
}

private fun resolveDisplayResult(
    uiState: QueryReportUiState,
    selectedPeriod: DataTreePeriod
): QueryResult? {
    return when (uiState.activeResult) {
        is QueryResult.Report,
        null -> uiState.reportResultsByPeriod[selectedPeriod]
        else -> uiState.activeResult
    }
}

private fun resolveDisplayReportSummary(
    uiState: QueryReportUiState,
    selectedPeriod: DataTreePeriod
): ReportSummary? {
    val activeResult = uiState.activeResult
    return if (activeResult is QueryResult.Report) {
        activeResult.summary
    } else if (activeResult == null) {
        uiState.reportResultsByPeriod[selectedPeriod]?.summary
            ?: uiState.reportSummariesByPeriod[selectedPeriod]
    } else {
        null
    }
}

private fun resolveDisplayReportError(
    uiState: QueryReportUiState,
    selectedPeriod: DataTreePeriod
): String {
    val activeResult = uiState.activeResult
    return if (activeResult == null) {
        uiState.reportErrorsByPeriod[selectedPeriod].orEmpty()
    } else {
        ""
    }
}

private fun ReportMode.toPeriod(): DataTreePeriod =
    when (this) {
        ReportMode.DAY -> DataTreePeriod.DAY
        ReportMode.MONTH -> DataTreePeriod.MONTH
        ReportMode.WEEK -> DataTreePeriod.WEEK
        ReportMode.YEAR -> DataTreePeriod.YEAR
        ReportMode.RANGE -> DataTreePeriod.RANGE
        ReportMode.RECENT -> DataTreePeriod.RECENT
    }
