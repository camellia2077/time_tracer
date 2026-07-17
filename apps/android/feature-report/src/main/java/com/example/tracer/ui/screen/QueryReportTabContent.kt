package com.example.tracer

import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.verticalScroll
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import com.example.tracer.ui.components.CalendarAvailability

@Composable
fun QueryReportTabContent(
    modifier: Modifier = Modifier,
    queryUiState: QueryReportUiState,
    queryReportViewModel: QueryReportViewModel,
    availableTxtMonths: List<String>,
    preferredResultDisplayMode: ReportResultDisplayMode,
    onPreferredResultDisplayModeChange: (ReportResultDisplayMode) -> Unit,
    preferredParameterSection: ReportParameterSection,
    onPreferredParameterSectionChange: (ReportParameterSection) -> Unit,
    preferredChartSemanticMode: ReportChartSemanticMode,
    onPreferredChartSemanticModeChange: (ReportChartSemanticMode) -> Unit,
    chartShowAverageLine: Boolean,
    piePalettePreset: ReportPiePalettePreset,
    onChartShowAverageLineChange: (Boolean) -> Unit,
    heatmapTomlConfig: ReportHeatmapTomlConfig,
    heatmapStylePreference: ReportHeatmapStylePreference,
    onHeatmapThemePolicyChange: (ReportHeatmapThemePolicy) -> Unit,
    onHeatmapPaletteNameChange: (String) -> Unit,
    heatmapApplyMessage: String,
    isAppDarkThemeActive: Boolean,
    bottomContentPadding: Dp = 0.dp
) {
    LaunchedEffect(preferredChartSemanticMode) {
        queryReportViewModel.onPersistedChartSemanticModeChange(preferredChartSemanticMode)
    }
    LaunchedEffect(preferredResultDisplayMode) {
        queryReportViewModel.onResultDisplayModeChange(preferredResultDisplayMode)
    }
    LaunchedEffect(preferredParameterSection) {
        queryReportViewModel.onParameterSectionChange(preferredParameterSection)
    }
    // Report year menus intentionally follow existing TXT year directories so
    // users only pick years that actually back YYYY/YYYY-MM.txt storage.
    val calendarAvailability = CalendarAvailability.fromMonthKeys(availableTxtMonths)
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

    Column(modifier = modifier.fillMaxSize()) {
        ReportModeTabs(
            selectedIndex = ReportMode.entries.indexOf(queryUiState.reportMode),
            reportModes = ReportMode.entries,
            reportMode = queryUiState.reportMode,
            onReportModeChange = queryReportViewModel::onReportModeChange
        )

        Column(
            modifier = Modifier
                .weight(1f)
                .verticalScroll(rememberScrollState())
                // The mode tabs remain fixed above this scroll container. Keep
                // clearance inside the scroll content so the final expanded card
                // can move above the floating bottom navigation instead of being
                // permanently obscured by it.
                .padding(bottom = bottomContentPadding),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            QueryReportSection(
                showModeTabs = false,
                reportMode = queryUiState.reportMode,
                onReportModeChange = queryReportViewModel::onReportModeChange,
                reportDate = queryUiState.reportDate,
                onReportDateChange = queryReportViewModel::onReportDateChange,
                reportMonth = queryUiState.reportMonth,
                onReportMonthChange = queryReportViewModel::onReportMonthChange,
                calendarAvailability = calendarAvailability,
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
                onResultDisplayModeChange = { mode ->
                    queryReportViewModel.onResultDisplayModeChange(mode)
                    onPreferredResultDisplayModeChange(mode)
                },
                chartSemanticMode = queryUiState.chartSemanticMode,
                onChartSemanticModeChange = { mode ->
                    queryReportViewModel.onChartSemanticModeChange(mode)
                    onPreferredChartSemanticModeChange(mode)
                },
                selectedParameterSection = queryUiState.parameterSection,
                onSelectedParameterSectionChange = { section ->
                    queryReportViewModel.onParameterSectionChange(section)
                    onPreferredParameterSectionChange(section)
                },
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
                chartSemanticMode = queryUiState.chartSemanticMode,
                compositionVisualMode = queryUiState.compositionVisualMode,
                trendChartRoots = queryUiState.trendChartRoots,
                trendChartSelectedRoot = queryUiState.trendChartSelectedRoot,
                reportMode = queryUiState.reportMode,
                trendChartLoading = queryUiState.trendChartLoading,
                trendChartError = queryUiState.trendChartError,
                trendChartRenderModel = queryUiState.trendChartRenderModel,
                trendChartLastTrace = queryUiState.trendChartLastTrace,
                compositionChartLoading = queryUiState.compositionChartLoading,
                compositionChartError = queryUiState.compositionChartError,
                compositionChartRenderModel = queryUiState.compositionChartRenderModel,
                compositionChartLastTrace = queryUiState.compositionChartLastTrace,
                chartShowAverageLine = chartShowAverageLine,
                piePalettePreset = piePalettePreset,
                heatmapTomlConfig = heatmapTomlConfig,
                heatmapStylePreference = heatmapStylePreference,
                onHeatmapThemePolicyChange = onHeatmapThemePolicyChange,
                onHeatmapPaletteNameChange = onHeatmapPaletteNameChange,
                heatmapApplyMessage = heatmapApplyMessage,
                isAppDarkThemeActive = isAppDarkThemeActive,
                onCompositionVisualModeChange = queryReportViewModel::onCompositionVisualModeChange,
                onChartRootChange = queryReportViewModel::onChartRootChange,
                onChartShowAverageLineChange = onChartShowAverageLineChange,
                onLoadChart = queryReportViewModel::loadChart
            )
        }
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
