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
    preferredTrendChartRoot: String,
    onPreferredTrendChartRootChange: (String) -> Unit,
    preferredAverageDayBasis: ReportAverageDayBasis,
    chartShowAverageLine: Boolean,
    piePalettePreset: ReportPiePalettePreset,
    onChartShowAverageLineChange: (Boolean) -> Unit,
    heatmapTomlConfig: ReportHeatmapTomlConfig,
    heatmapStylePreference: ReportHeatmapStylePreference,
    onHeatmapThemePolicyChange: (ReportHeatmapThemePolicy) -> Unit,
    onHeatmapPaletteNameChange: (String) -> Unit,
    heatmapApplyMessage: String,
    isAppDarkThemeActive: Boolean,
    onEditDailyStatuses: () -> Unit = {},
    bottomContentPadding: Dp = 0.dp
) {
    LaunchedEffect(
        preferredReportMode,
        preferredChartSemanticMode,
        preferredTrendChartRoot,
        preferredResultDisplayMode,
        preferredParameterSection,
        preferredAverageDayBasis
    ) {
        // These four DataStore values form one report presentation selection. Applying them
        // independently can launch a default text report before the persisted Chart mode has
        // arrived, whose late result overwrites the chart during cold start.
        queryReportViewModel.applyReportAverageDayBasis(preferredAverageDayBasis)
        queryReportViewModel.applyPersistedReportPresentation(
            reportMode = preferredReportMode,
            chartSemanticMode = preferredChartSemanticMode,
            trendChartSelectedRoot = preferredTrendChartRoot,
            resultDisplayMode = preferredResultDisplayMode,
            parameterSection = preferredParameterSection
        )
    }
    // DataStore preferences are asynchronous persistence input, while chart results and loading
    // state are produced by the ViewModel. Rendering the selectors from preferences but the
    // chart from queryUiState split one user selection across two clocks: after switching Month,
    // Compose could render the new Trend shell with the old empty/loading chart snapshot. Once
    // preferences have been applied above, use the ViewModel's single, atomic presentation state
    // for every visible report control and result.
    val displayedReportMode = queryUiState.reportMode
    val displayedResultDisplayMode = queryUiState.resultDisplayMode
    val displayedParameterSection = if (
        displayedReportMode != ReportMode.DAY &&
        queryUiState.parameterSection == ReportParameterSection.TIMELINE
    ) {
        ReportParameterSection.DAY
    } else {
        queryUiState.parameterSection
    }
    val displayedChartSemanticMode = queryUiState.chartSemanticMode

    // Report period selectors are backed by Core's database calendar query. TXT storage is
    // intentionally not consulted here because Report reads the database projection.
    val calendarAvailability = CalendarAvailability.fromMonthKeys(
        queryUiState.availableReportMonths
    )
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
                onChartRootChange = { root ->
                    queryReportViewModel.onChartRootChange(root)
                    onPreferredTrendChartRootChange(root)
                },
                onChartShowAverageLineChange = onChartShowAverageLineChange,
                onChartVisualModeChange = onPreferredChartVisualModeChange,
                onUpdateActivityRemark = queryReportViewModel::updateActivityRemark,
                onUpdateDayRemark = queryReportViewModel::updateDayRemark,
                onEditDailyStatuses = onEditDailyStatuses
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
        ReportParameterSection.ACTIVITY_HIERARCHY -> uiState.activeResult as? QueryResult.Tree
        ReportParameterSection.DAY,
        ReportParameterSection.TIMELINE -> uiState.reportResultsByPeriod[selectedPeriod]
    }
}

private fun resolveDisplayReportSummary(
    uiState: QueryReportUiState,
    selectedPeriod: DataTreePeriod,
    selectedSection: ReportParameterSection
): ReportSummary? {
    if (selectedSection == ReportParameterSection.ACTIVITY_HIERARCHY) {
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
    return if (selectedSection == ReportParameterSection.ACTIVITY_HIERARCHY) {
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
