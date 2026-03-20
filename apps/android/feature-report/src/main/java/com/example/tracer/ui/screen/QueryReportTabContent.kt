package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp

@Composable
fun QueryReportTabContent(
    queryUiState: QueryReportUiState,
    queryReportViewModel: QueryReportViewModel,
    chartShowAverageLine: Boolean,
    onChartShowAverageLineChange: (Boolean) -> Unit,
    heatmapTomlConfig: ReportHeatmapTomlConfig,
    heatmapStylePreference: ReportHeatmapStylePreference,
    onHeatmapThemePolicyChange: (ReportHeatmapThemePolicy) -> Unit,
    onHeatmapPaletteNameChange: (String) -> Unit,
    heatmapApplyMessage: String,
    isAppDarkThemeActive: Boolean
) {
    var reportMode by rememberSaveable { mutableStateOf(ReportMode.DAY) }
    val displayResult = resolveDisplayResult(
        uiState = queryUiState,
        selectedMode = reportMode
    )

    Column(
        modifier = Modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        QueryReportSection(
            reportMode = reportMode,
            onReportModeChange = { reportMode = it },
            reportDate = queryUiState.reportDate,
            onReportDateChange = queryReportViewModel::onReportDateChange,
            reportMonth = queryUiState.reportMonth,
            onReportMonthChange = queryReportViewModel::onReportMonthChange,
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
            analysisError = queryUiState.analysisError,
            chartRoots = queryUiState.chartRoots,
            chartSelectedRoot = queryUiState.chartSelectedRoot,
            chartDateInputMode = queryUiState.chartDateInputMode,
            chartLookbackDays = queryUiState.chartLookbackDays,
            chartRangeStartDate = queryUiState.chartRangeStartDate,
            chartRangeEndDate = queryUiState.chartRangeEndDate,
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
            onChartDateInputModeChange = queryReportViewModel::onChartDateInputModeChange,
            onChartLookbackDaysChange = queryReportViewModel::onChartLookbackDaysChange,
            onChartRangeStartDateChange = queryReportViewModel::onChartRangeStartDateChange,
            onChartRangeEndDateChange = queryReportViewModel::onChartRangeEndDateChange,
            onChartShowAverageLineChange = onChartShowAverageLineChange,
            onLoadChart = queryReportViewModel::loadChart
        )
    }
}

private fun resolveDisplayResult(
    uiState: QueryReportUiState,
    selectedMode: ReportMode
): QueryResult? {
    val selectedPeriod = selectedMode.toPeriod()
    return when (uiState.activeResult) {
        is QueryResult.Report,
        null -> uiState.reportResultsByPeriod[selectedPeriod]
        else -> uiState.activeResult
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
