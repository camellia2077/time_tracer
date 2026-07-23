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
    preferredReportMode: ReportMode,
    onPreferredReportModeChange: (ReportMode) -> Unit,
    preferredResultDisplayMode: ReportResultDisplayMode,
    onPreferredResultDisplayModeChange: (ReportResultDisplayMode) -> Unit,
    preferredParameterSection: ReportParameterSection,
    onPreferredParameterSectionChange: (ReportParameterSection) -> Unit,
    timeParametersExpanded: Boolean,
    onTimeParametersExpandedChange: (Boolean) -> Unit,
    preferredChartSemanticMode: ReportChartSemanticMode,
    onPreferredChartSemanticModeChange: (ReportChartSemanticMode) -> Unit,
    preferredChartVisualMode: ReportChartVisualMode,
    onPreferredChartVisualModeChange: (ReportChartVisualMode) -> Unit,
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
    LaunchedEffect(preferredReportMode) {
        queryReportViewModel.onPersistedReportModeChange(preferredReportMode)
    }
    LaunchedEffect(preferredChartSemanticMode) {
        queryReportViewModel.onPersistedChartSemanticModeChange(preferredChartSemanticMode)
    }
    LaunchedEffect(preferredResultDisplayMode) {
        queryReportViewModel.onResultDisplayModeChange(preferredResultDisplayMode)
    }
    LaunchedEffect(preferredParameterSection) {
        queryReportViewModel.onParameterSectionChange(preferredParameterSection)
    }
    val displayedReportMode = preferredReportMode
    val displayedResultDisplayMode = preferredResultDisplayMode
    val displayedParameterSection = if (
        displayedReportMode != ReportMode.DAY &&
        preferredParameterSection == ReportParameterSection.TIMELINE
    ) {
        ReportParameterSection.DAY
    } else {
        preferredParameterSection
    }
    val displayedChartSemanticMode = preferredChartSemanticMode
        .normalizeForReportMode(displayedReportMode)

    // Report year menus intentionally follow existing TXT year directories so
    // users only pick years that actually back YYYY/YYYY-MM.txt storage.
    val calendarAvailability = CalendarAvailability.fromMonthKeys(availableTxtMonths)
    val selectedPeriod = displayedReportMode.toPeriod()
    val treeMaxAvailableDepth = (queryUiState.activeResult as? QueryResult.Tree)
        ?.maxAvailableDepth
        ?: 0
    val displayResult = resolveDisplayResult(
        uiState = queryUiState,
        selectedPeriod = selectedPeriod,
        selectedSection = displayedParameterSection
    )
    val displayReportSummary = resolveDisplayReportSummary(
        uiState = queryUiState,
        selectedPeriod = selectedPeriod,
        selectedSection = displayedParameterSection
    )
    val displayReportError = resolveDisplayReportError(
        uiState = queryUiState,
        selectedPeriod = selectedPeriod,
        selectedSection = displayedParameterSection
    )
    val onReportModeChange: (ReportMode) -> Unit = { mode ->
        queryReportViewModel.onReportModeChange(mode)
        onPreferredReportModeChange(mode)
        queryReportViewModel.reportCurrentSelection()
    }

    Column(modifier = modifier.fillMaxSize()) {
        ReportModeTabs(
            selectedIndex = ReportMode.entries.indexOf(displayedReportMode),
            reportModes = ReportMode.entries,
            reportMode = displayedReportMode,
            onReportModeChange = onReportModeChange
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
                reportMode = displayedReportMode,
                onReportModeChange = onReportModeChange,
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
                resultDisplayMode = displayedResultDisplayMode,
                onResultDisplayModeChange = { mode ->
                    queryReportViewModel.onResultDisplayModeChange(mode)
                    onPreferredResultDisplayModeChange(mode)
                },
                chartSemanticMode = displayedChartSemanticMode,
                onChartSemanticModeChange = { mode ->
                    queryReportViewModel.onChartSemanticModeChange(mode)
                    onPreferredChartSemanticModeChange(mode)
                },
                selectedParameterSection = displayedParameterSection,
                treeLevel = queryUiState.treeLevel,
                treeMaxAvailableDepth = treeMaxAvailableDepth,
                onSelectedParameterSectionChange = { section ->
                    queryReportViewModel.onParameterSectionChange(section)
                    onPreferredParameterSectionChange(section)
                },
                onTreeLevelChange = { level ->
                    queryReportViewModel.onTreeLevelChange(level)
                },
                timeParametersExpanded = timeParametersExpanded,
                onTimeParametersExpandedChange = onTimeParametersExpandedChange,
            )

            QueryReportResultDisplay(
                resultDisplayMode = displayedResultDisplayMode,
                activeResult = displayResult,
                reportSummary = displayReportSummary,
                dayTimeline = if (displayedReportMode == ReportMode.DAY) {
                    queryUiState.dayTimeline
                } else {
                    null
                },
                parameterSection = displayedParameterSection,
                reportError = displayReportError,
                analysisError = queryUiState.analysisError,
                chartSemanticMode = displayedChartSemanticMode,
                chartVisualMode = preferredChartVisualMode,
                compositionVisualMode = queryUiState.compositionVisualMode,
                trendChartRoots = queryUiState.trendChartRoots,
                trendChartSelectedRoot = queryUiState.trendChartSelectedRoot,
                reportMode = displayedReportMode,
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
                onChartVisualModeChange = onPreferredChartVisualModeChange,
                onLoadChart = queryReportViewModel::loadChart,
                onUpdateActivityRemark = queryReportViewModel::updateActivityRemark,
                onUpdateDayRemark = queryReportViewModel::updateDayRemark
            )
        }
    }
}

internal fun resolveDisplayResult(
    uiState: QueryReportUiState,
    selectedPeriod: DataTreePeriod,
    selectedSection: ReportParameterSection
): QueryResult? {
    return when (selectedSection) {
        ReportParameterSection.TREE -> uiState.activeResult as? QueryResult.Tree
        ReportParameterSection.DAY,
        ReportParameterSection.TIMELINE -> uiState.reportResultsByPeriod[selectedPeriod]
    }
}

private fun resolveDisplayReportSummary(
    uiState: QueryReportUiState,
    selectedPeriod: DataTreePeriod,
    selectedSection: ReportParameterSection
): ReportSummary? {
    if (selectedSection == ReportParameterSection.TREE) {
        return null
    }
    return uiState.reportResultsByPeriod[selectedPeriod]?.summary
        ?: uiState.reportSummariesByPeriod[selectedPeriod]
}

private fun resolveDisplayReportError(
    uiState: QueryReportUiState,
    selectedPeriod: DataTreePeriod,
    selectedSection: ReportParameterSection
): String {
    return if (selectedSection == ReportParameterSection.TREE) {
        ""
    } else {
        uiState.reportErrorsByPeriod[selectedPeriod].orEmpty()
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
