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
import android.util.Log
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import com.example.tracer.ui.components.CalendarAvailability


@Composable
fun QueryInsightsTabContent(
    modifier: Modifier = Modifier,
    queryUiState: QueryInsightsUiState,
    queryInsightsViewModel: QueryInsightsViewModel,
    preferredInsightsMode: InsightsMode,
    onPreferredInsightsModeChange: (InsightsMode) -> Unit,
    preferredResultDisplayMode: InsightsResultDisplayMode,
    onPreferredResultDisplayModeChange: (InsightsResultDisplayMode) -> Unit,
    preferredParameterSection: InsightsParameterSection,
    onPreferredParameterSectionChange: (InsightsParameterSection) -> Unit,
    dayActivitiesView: InsightsActivityView,
    periodActivitiesView: InsightsActivityView,
    onDayActivitiesViewChange: (InsightsActivityView) -> Unit,
    onPeriodActivitiesViewChange: (InsightsActivityView) -> Unit,
    timeParametersExpanded: Boolean,
    onTimeParametersExpandedChange: (Boolean) -> Unit,
    preferredChartSemanticMode: InsightsChartSemanticMode,
    onPreferredChartSemanticModeChange: (InsightsChartSemanticMode) -> Unit,
    preferredChartVisualMode: InsightsChartVisualMode,
    onPreferredChartVisualModeChange: (InsightsChartVisualMode) -> Unit,
    preferredTrendChartRoot: String,
    onPreferredTrendChartRootChange: (String) -> Unit,
    preferredAverageDayBasis: InsightsAverageDayBasis,
    chartShowAverageLine: Boolean,
    piePalettePreset: InsightsPiePalettePreset,
    onChartShowAverageLineChange: (Boolean) -> Unit,
    heatmapTomlConfig: InsightsHeatmapTomlConfig,
    heatmapStylePreference: InsightsHeatmapStylePreference,
    onHeatmapThemePolicyChange: (InsightsHeatmapThemePolicy) -> Unit,
    onHeatmapPaletteNameChange: (String) -> Unit,
    heatmapApplyMessage: String,
    isAppDarkThemeActive: Boolean,
    onEditDailyStatuses: () -> Unit = {},
    bottomContentPadding: Dp = 0.dp
) {
    LaunchedEffect(
        preferredInsightsMode,
        preferredChartSemanticMode,
        preferredTrendChartRoot,
        preferredResultDisplayMode,
        preferredParameterSection,
        preferredAverageDayBasis
    ) {
        // These four DataStore values form one insights presentation selection. Applying them
        // independently can launch a default text insights before the persisted Chart mode has
        // arrived, whose late result overwrites the chart during cold start.
        queryInsightsViewModel.applyInsightsAverageDayBasis(preferredAverageDayBasis)
        queryInsightsViewModel.applyPersistedInsightsPresentation(
            insightsMode = preferredInsightsMode,
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
    // for every visible insights control and result.
    val displayedInsightsMode = queryUiState.insightsMode
    val displayedResultDisplayMode = queryUiState.resultDisplayMode
    val displayedParameterSection = queryUiState.parameterSection
    val displayedChartSemanticMode = queryUiState.chartSemanticMode

    // Insights period selectors are backed by Core's database calendar query. TXT storage is
    // intentionally not consulted here because Insights reads the database projection.
    val calendarAvailability = CalendarAvailability.fromMonthKeys(
        queryUiState.availableInsightsMonths
    )
    LaunchedEffect(
        queryUiState.availableInsightsMonths,
        queryUiState.insightsDate,
        queryUiState.insightsMonth,
        queryUiState.insightsYear
    ) {
        Log.d(
            "InsightsCalendar",
            "selector render rawMonths=${queryUiState.availableInsightsMonths} " +
                "years=${calendarAvailability.years} " +
                "monthsByYear=${calendarAvailability.monthsByYear} " +
                "selected=(date=${queryUiState.insightsDate},month=${queryUiState.insightsMonth}," +
                "year=${queryUiState.insightsYear})"
        )
    }
    val selectedPeriod = displayedInsightsMode.toPeriod()
    val treeMaxAvailableDepth = (queryUiState.activeResult as? QueryResult.Tree)
        ?.maxAvailableDepth
        ?: 0
    val displayResult = resolveDisplayResult(
        uiState = queryUiState,
        selectedPeriod = selectedPeriod,
        selectedSection = displayedParameterSection
    )
    val displayInsightsSummary = resolveDisplayInsightsSummary(
        uiState = queryUiState,
        selectedPeriod = selectedPeriod,
        selectedSection = displayedParameterSection
    )
    val displayInsightsError = resolveDisplayInsightsError(
        uiState = queryUiState,
        selectedPeriod = selectedPeriod,
        selectedSection = displayedParameterSection
    )
    val onInsightsModeChange: (InsightsMode) -> Unit = { mode ->
        queryInsightsViewModel.onInsightsModeChange(mode)
        onPreferredInsightsModeChange(mode)
        queryInsightsViewModel.insightsCurrentSelection()
    }

    Column(modifier = modifier.fillMaxSize()) {
        InsightsModeTabs(
            selectedIndex = InsightsMode.entries.indexOf(displayedInsightsMode),
            insightsModes = InsightsMode.entries,
            insightsMode = displayedInsightsMode,
            onInsightsModeChange = onInsightsModeChange
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
            QueryInsightsSection(
                showModeTabs = false,
                insightsMode = displayedInsightsMode,
                onInsightsModeChange = onInsightsModeChange,
                insightsDate = queryUiState.insightsDate,
                onInsightsDateChange = queryInsightsViewModel::onInsightsDateChange,
                insightsMonth = queryUiState.insightsMonth,
                onInsightsMonthChange = queryInsightsViewModel::onInsightsMonthChange,
                calendarAvailability = calendarAvailability,
                insightsYear = queryUiState.insightsYear,
                onInsightsYearChange = queryInsightsViewModel::onInsightsYearChange,
                insightsWeek = queryUiState.insightsWeek,
                onInsightsWeekChange = queryInsightsViewModel::onInsightsWeekChange,
                insightsRangeStartDate = queryUiState.insightsRangeStartDate,
                onInsightsRangeStartDateChange = queryInsightsViewModel::onInsightsRangeStartDateChange,
                insightsRangeEndDate = queryUiState.insightsRangeEndDate,
                onInsightsRangeEndDateChange = queryInsightsViewModel::onInsightsRangeEndDateChange,
                insightsRecentDays = queryUiState.insightsRecentDays,
                onInsightsRecentDaysChange = queryInsightsViewModel::onInsightsRecentDaysChange,
                onInsightsActivityPeriodConfirmed = queryInsightsViewModel::onInsightsActivityPeriodConfirmed,
                resultDisplayMode = displayedResultDisplayMode,
                onResultDisplayModeChange = { mode ->
                    queryInsightsViewModel.onResultDisplayModeChange(mode)
                    onPreferredResultDisplayModeChange(mode)
                },
                chartSemanticMode = displayedChartSemanticMode,
                onChartSemanticModeChange = { mode ->
                    queryInsightsViewModel.onChartSemanticModeChange(mode)
                    onPreferredChartSemanticModeChange(mode)
                },
                selectedParameterSection = displayedParameterSection,
                treeLevel = queryUiState.treeLevel,
                treeMaxAvailableDepth = treeMaxAvailableDepth,
                onSelectedParameterSectionChange = { section ->
                    queryInsightsViewModel.onParameterSectionChange(section)
                    onPreferredParameterSectionChange(section)
                },
                onTreeLevelChange = { level ->
                    queryInsightsViewModel.onTreeLevelChange(level)
                },
                timeParametersExpanded = timeParametersExpanded,
                onTimeParametersExpandedChange = onTimeParametersExpandedChange,
            )

            QueryInsightsResultDisplay(
                resultDisplayMode = displayedResultDisplayMode,
                activeResult = displayResult,
                insightsSummary = displayInsightsSummary,
                dayTimeline = if (displayedInsightsMode == InsightsMode.DAY) {
                    queryUiState.dayTimeline
                } else {
                    null
                },
                periodActivityDays = queryUiState.periodActivityDays[selectedPeriod].orEmpty(),
                periodActivityProjectTree = queryUiState.periodActivityProjectTrees[selectedPeriod].orEmpty(),
                periodComparison = queryUiState.periodComparison,
                canComparePreviousPeriod = queryInsightsViewModel.canComparePreviousPeriod(),
                calendarAvailability = calendarAvailability,
                dayActivitiesView = dayActivitiesView,
                periodActivitiesView = periodActivitiesView,
                onDayActivitiesViewChange = onDayActivitiesViewChange,
                onPeriodActivitiesViewChange = onPeriodActivitiesViewChange,
                onPeriodComparisonToggle = queryInsightsViewModel::onPeriodComparisonToggle,
                onComparisonPeriodSelected =
                    queryInsightsViewModel::onComparisonPeriodSelected,
                parameterSection = displayedParameterSection,
                insightsError = displayInsightsError,
                analysisError = queryUiState.analysisError,
                chartSemanticMode = displayedChartSemanticMode,
                chartVisualMode = preferredChartVisualMode,
                compositionVisualMode = queryUiState.compositionVisualMode,
                trendChartRoots = queryUiState.trendChartRoots,
                trendChartSelectedRoot = queryUiState.trendChartSelectedRoot,
                insightsMode = displayedInsightsMode,
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
                onCompositionVisualModeChange = queryInsightsViewModel::onCompositionVisualModeChange,
                onChartRootChange = { root ->
                    queryInsightsViewModel.onChartRootChange(root)
                    onPreferredTrendChartRootChange(root)
                },
                onChartShowAverageLineChange = onChartShowAverageLineChange,
                onChartVisualModeChange = onPreferredChartVisualModeChange,
                onUpdateActivityRemark = queryInsightsViewModel::updateActivityRemark,
                onUpdateDayRemark = queryInsightsViewModel::updateDayRemark,
                onEditDailyStatuses = onEditDailyStatuses
            )
        }
    }
}

internal fun resolveDisplayResult(
    uiState: QueryInsightsUiState,
    selectedPeriod: DataTreePeriod,
    selectedSection: InsightsParameterSection
): QueryResult? {
    return when (selectedSection) {
        InsightsParameterSection.ACTIVITY_HIERARCHY -> uiState.activeResult as? QueryResult.Tree
        InsightsParameterSection.DAY,
        InsightsParameterSection.ACTIVITIES -> uiState.insightsResultsByPeriod[selectedPeriod]
    }
}

private fun resolveDisplayInsightsSummary(
    uiState: QueryInsightsUiState,
    selectedPeriod: DataTreePeriod,
    selectedSection: InsightsParameterSection
): InsightsSummary? {
    if (selectedSection == InsightsParameterSection.ACTIVITY_HIERARCHY) {
        return null
    }
    return uiState.insightsResultsByPeriod[selectedPeriod]?.summary
        ?: uiState.insightsSummariesByPeriod[selectedPeriod]
}

private fun resolveDisplayInsightsError(
    uiState: QueryInsightsUiState,
    selectedPeriod: DataTreePeriod,
    selectedSection: InsightsParameterSection
): String {
    return if (selectedSection == InsightsParameterSection.ACTIVITY_HIERARCHY) {
        ""
    } else {
        uiState.insightsErrorsByPeriod[selectedPeriod].orEmpty()
    }
}

private fun InsightsMode.toPeriod(): DataTreePeriod =
    when (this) {
        InsightsMode.DAY -> DataTreePeriod.DAY
        InsightsMode.MONTH -> DataTreePeriod.MONTH
        InsightsMode.WEEK -> DataTreePeriod.WEEK
        InsightsMode.YEAR -> DataTreePeriod.YEAR
        InsightsMode.RANGE -> DataTreePeriod.RANGE
        InsightsMode.RECENT -> DataTreePeriod.RECENT
    }
