package com.example.tracer

import android.util.Log
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch
import java.time.Clock




private fun QueryInsightsViewModel.digitsOnly(value: String, maxLength: Int): String =
    value.filter { it.isDigit() }.take(maxLength)

fun QueryInsightsViewModel.onInsightsDateChange(value: String) {
    updateInsightsParams({
        copy(insightsDate = digitsOnly(value, 8))
    }, autoInsights = true)
}

fun QueryInsightsViewModel.onInsightsModeChange(mode: InsightsMode) {
    updateInsightsParams({
        val hidesStaleAnalysis = activeResult.isAnalysisResultForDifferentPeriod(
            mode.toDataTreePeriod()
        )
        copy(
            insightsMode = mode,
            parameterSection = if (
                mode != InsightsMode.DAY && parameterSection == InsightsParameterSection.TIMELINE
            ) {
                InsightsParameterSection.DAY
            } else {
                parameterSection
            },
            // Project Tree cards are scoped to the period used to query them.
            // Do not retain an analysis card after changing to a different insights window.
            activeResult = if (hidesStaleAnalysis) null else activeResult,
            analysisError = if (hidesStaleAnalysis) "" else analysisError,
            chartSemanticMode = preferredChartSemanticMode.normalizeForInsightsMode(mode)
        )
    })
    // Chart and Tree are independent pipelines, but both write the shared UiState. Starting
    // Tree here while Chart is loading lets a slower Tree result restore an older chart
    // loading/empty snapshot after the Chart result has already committed. Tree is not
    // visible in Chart mode, so defer it until the user is actually viewing Text/Tree.
    if (uiState.resultDisplayMode != InsightsResultDisplayMode.CHART &&
        uiState.parameterSection == InsightsParameterSection.ACTIVITY_HIERARCHY
    ) {
        loadTree(mode.toDataTreePeriod(), uiState.treeLevel)
    }
}

fun QueryInsightsViewModel.onPersistedInsightsModeChange(mode: InsightsMode) {
    if (uiState.insightsMode == mode) {
        return
    }
    onInsightsModeChange(mode)
}

fun QueryInsightsViewModel.onInsightsMonthChange(value: String) {
    updateInsightsParams({
        copy(insightsMonth = digitsOnly(value, 6))
    }, autoInsights = true)
}

fun QueryInsightsViewModel.onInsightsYearChange(value: String) {
    updateInsightsParams({
        copy(insightsYear = digitsOnly(value, 4))
    }, autoInsights = true)
}

fun QueryInsightsViewModel.onInsightsWeekChange(value: String) {
    updateInsightsParams({
        copy(insightsWeek = digitsOnly(value, 6))
    }, autoInsights = true)
}

fun QueryInsightsViewModel.onInsightsRecentDaysChange(value: String) {
    updateInsightsParams({
        copy(insightsRecentDays = value.filter { it.isDigit() })
    }, autoInsights = true)
}

fun QueryInsightsViewModel.refreshInsightsDayDefault() {
    refreshInsightsDayDefault(refresh = true)
}

fun QueryInsightsViewModel.onInsightsTabEntered() {
    // On a cold start the tab lifecycle reaches here before DataStore has delivered the
    // insights presentation preferences. Do not issue the default TEXT query yet: it can
    // finish after the persisted CHART query and overwrite the visible chart with a
    // Markdown/no-data state.
    logChart("tab entered; preferencesApplied=$insightsPresentationPreferencesApplied ${chartSelection()}")
    refreshInsightsCalendarAvailability()
    refreshInsightsDayDefault(refresh = insightsPresentationPreferencesApplied)
}

private fun QueryInsightsViewModel.refreshInsightsCalendarAvailability() {
    viewModelScope.launch {
        val result = queryGateway.queryInsightsCalendarAvailability()
        if (result.ok) {
            uiState = uiState.copy(availableInsightsMonths = result.months.distinct().sorted())
        }
    }
}

private fun QueryInsightsViewModel.refreshInsightsDayDefault(refresh: Boolean) {
    // Insights day parameters are intentionally session-local. When users return to the
    // Insights tab, the day field should reflect the current logical log day instead of
    // preserving a previously inspected date from an earlier tab visit.
    val currentLogicalDayDigits = currentDateDigits(clock)
    if (uiState.insightsDate != currentLogicalDayDigits) {
        uiState = uiState.copy(insightsDate = currentLogicalDayDigits)
    }
    // The underlying TXT may have changed while the user was on Record. The date often
    // remains the same, so changing only the parameter leaves the cached timeline stale.
    // Re-query the currently selected insights on every Insights-tab entry, including same-day
    // entries.
    // Tab entry can run before DataStore-backed insights preferences are applied during a
    // cold start. Waiting avoids a default text-insights request racing the chart request
    // and replacing an already rendered chart with its empty Markdown result.
    if (refresh) {
        refreshCurrentResult()
    }
}

fun QueryInsightsViewModel.applyPersistedInsightsPresentation(
    insightsMode: InsightsMode,
    chartSemanticMode: InsightsChartSemanticMode,
    resultDisplayMode: InsightsResultDisplayMode,
    parameterSection: InsightsParameterSection,
    trendChartSelectedRoot: String = ""
) {
    // Treat the persisted display, period, semantic mode, and parameter section as one
    // selection. Applying them in separate Compose effects briefly leaves the ViewModel in
    // its default TEXT state, which starts a query whose delayed result can replace CHART.
    val normalizedParameterSection = if (
        insightsMode != InsightsMode.DAY && parameterSection == InsightsParameterSection.TIMELINE
    ) {
        InsightsParameterSection.DAY
    } else {
        parameterSection
    }
    val normalizedChartSemanticMode = chartSemanticMode.normalizeForInsightsMode(insightsMode)
    val firstPreferenceApplication = !insightsPresentationPreferencesApplied
    val changed = uiState.insightsMode != insightsMode ||
        uiState.preferredChartSemanticMode != chartSemanticMode ||
        uiState.chartSemanticMode != normalizedChartSemanticMode ||
        uiState.resultDisplayMode != resultDisplayMode ||
        uiState.parameterSection != normalizedParameterSection ||
        uiState.trendChartSelectedRoot != trendChartSelectedRoot.trim()
    insightsPresentationPreferencesApplied = true
    if (!firstPreferenceApplication && !changed) {
        // This effect can be launched from the chart's loading composition. Its snapshot
        // may predate a just-committed result; copying the unchanged preference selection
        // back into uiState would then restore that stale empty/loading chart state.
        logChart("persisted presentation ignored because selection is unchanged; ${chartSelection()}")
        return
    }
    if (changed) {
        invalidateInFlightChartRequests("persisted presentation")
    }
    val restoredState = uiState.copy(
        insightsMode = insightsMode,
        preferredChartSemanticMode = chartSemanticMode,
        chartSemanticMode = normalizedChartSemanticMode,
        resultDisplayMode = resultDisplayMode,
        parameterSection = normalizedParameterSection,
        trendChartSelectedRoot = trendChartSelectedRoot.trim()
    )
    // A request that was already loading belongs to the pre-restoration selection. Clear
    // its loading flag along with its data so the fully restored chart can start its own
    // request instead of being suppressed by that stale in-flight request.
    uiState = if (changed) restoredState.invalidateChartState() else restoredState
    logChart(
        "persisted presentation applied; first=$firstPreferenceApplication changed=$changed " +
            chartSelection()
    )
    // Generate only for the fully restored selection. This makes the first result match
    // the screen the user actually persisted, rather than an intermediate default state.
    if (resultDisplayMode == InsightsResultDisplayMode.CHART) {
        refreshCurrentChart()
    } else if (normalizedParameterSection == InsightsParameterSection.ACTIVITY_HIERARCHY) {
        loadTree(insightsMode.toDataTreePeriod(), uiState.treeLevel)
    } else {
        insightsCurrentSelection()
    }
}

fun QueryInsightsViewModel.applyInsightsAverageDayBasis(value: InsightsAverageDayBasis) {
    if (uiState.averageDayBasis != value) {
        invalidateInFlightChartRequests("average day basis")
        uiState = uiState.copy(averageDayBasis = value).invalidateChartState()
        if (uiState.resultDisplayMode == InsightsResultDisplayMode.CHART) {
            refreshCurrentChart()
        }
    }
}

fun QueryInsightsViewModel.onResultDisplayModeChange(mode: InsightsResultDisplayMode) {
    invalidateInFlightChartRequests("display mode -> $mode")
    val normalizedState = if (mode == InsightsResultDisplayMode.CHART) {
        uiState.copy(
            resultDisplayMode = mode,
            chartSemanticMode = uiState.preferredChartSemanticMode
                .normalizeForInsightsMode(uiState.insightsMode)
        )
    } else {
        uiState.copy(resultDisplayMode = mode)
    }
    // Display changes can cancel an in-flight chart. Reset its loading state as well as
    // advancing the generation; otherwise returning to Chart could incorrectly believe a
    // cancelled request is still loading and never issue the replacement query.
    uiState = normalizedState.invalidateChartState()
    logChart("display mode applied; ${chartSelection()}")
    if (mode == InsightsResultDisplayMode.CHART) {
        refreshCurrentChart()
    } else if (mode == InsightsResultDisplayMode.TEXT) {
        // Switching from Chart to Text can happen after the insights mode has already
        // changed. Re-query the current period here so Week/Month/etc. does not wait for
        // a later tab visit or another parameter change to populate the Markdown result.
        insightsCurrentSelection()
    }
}

fun QueryInsightsViewModel.onChartRootChange(root: String) {
    invalidateInFlightChartRequests("trend root -> $root")
    uiState = uiState.copy(trendChartSelectedRoot = root).invalidateChartState()
    logChart("trend root applied; ${chartSelection()}")
    if (uiState.resultDisplayMode == InsightsResultDisplayMode.CHART &&
        uiState.chartSemanticMode == InsightsChartSemanticMode.TREND
    ) {
        loadChart()
    }
}

fun QueryInsightsViewModel.onChartSemanticModeChange(mode: InsightsChartSemanticMode) {
    val normalizedMode = mode.normalizeForInsightsMode(uiState.insightsMode)
    if (uiState.chartSemanticMode == normalizedMode &&
        uiState.preferredChartSemanticMode == mode
    ) {
        return
    }
    invalidateInFlightChartRequests("chart semantic -> $mode")
    uiState = uiState.copy(
        chartSemanticMode = normalizedMode,
        preferredChartSemanticMode = mode
    ).invalidateChartState()
    logChart("chart semantic applied; requested=$mode normalized=$normalizedMode ${chartSelection()}")
    if (uiState.resultDisplayMode == InsightsResultDisplayMode.CHART) {
        loadChart()
    }
}

fun QueryInsightsViewModel.onParameterSectionChange(section: InsightsParameterSection) {
    if (uiState.insightsMode != InsightsMode.DAY && section == InsightsParameterSection.TIMELINE) {
        return
    }
    val sectionChanged = uiState.parameterSection != section
    uiState = uiState.copy(parameterSection = section)
    if (section == InsightsParameterSection.ACTIVITY_HIERARCHY) {
        val selectedPeriod = uiState.insightsMode.toDataTreePeriod()
        val currentTree = uiState.activeResult as? QueryResult.Tree
        val hasCurrentTree = currentTree?.period == selectedPeriod
        // The persisted section can be re-applied after the ViewModel has already
        // been created (for example after a theme-driven recomposition). In that case
        // the section value itself may not change, but the Tree result can still be
        // missing. Rehydrate it while avoiding duplicate requests during loading.
        if (sectionChanged || (!hasCurrentTree && !uiState.analysisLoading)) {
            loadTree(selectedPeriod, uiState.treeLevel)
        }
    } else {
        // DAY, Markdown, and Timeline all consume the same cached day insights. Two flicker
        // bugs were caused by treating a presentation-only section change as a new query:
        // returning to Timeline recreated the tab and re-ran the persisted-section effect,
        // while Timeline -> Markdown changed the section value from TIMELINE to DAY. Both
        // paths cleared dayTimeline before reloading the unchanged insights. Re-query only
        // when the current period has no insights yet, so switching or recreating these views
        // preserves the existing data and avoids the empty-state frame.
        val currentPeriod = uiState.insightsMode.toDataTreePeriod()
        val hasCurrentInsights = uiState.insightsResultsByPeriod[currentPeriod] != null
        val needsInsightsRefresh = uiState.dayInsightsNeedsRefresh &&
            section != InsightsParameterSection.ACTIVITY_HIERARCHY
        if (!hasCurrentInsights || needsInsightsRefresh) {
            insightsCurrentSelection()
        }
    }
}

fun QueryInsightsViewModel.onPersistedChartSemanticModeChange(mode: InsightsChartSemanticMode) {
    // The persisted preference can arrive after the Chart display has already
    // been selected. Apply it through the same path as a user selection so the
    // newly visible semantic chart is hydrated instead of showing an empty state.
    onChartSemanticModeChange(mode)
}

fun QueryInsightsViewModel.onCompositionVisualModeChange(mode: InsightsCompositionVisualMode) {
    if (uiState.compositionVisualMode == mode) {
        return
    }
    uiState = uiState.copy(compositionVisualMode = mode)
}

fun QueryInsightsViewModel.onInsightsRangeStartDateChange(value: String) {
    updateInsightsParams({
        copy(insightsRangeStartDate = digitsOnly(value, 8))
    }, autoInsights = true)
}

fun QueryInsightsViewModel.onInsightsRangeEndDateChange(value: String) {
    updateInsightsParams({
        copy(insightsRangeEndDate = digitsOnly(value, 8))
    }, autoInsights = true)
}


internal fun QueryInsightsUiState.invalidateChartState(): QueryInsightsUiState = copy(
    trendChartRoots = emptyList(),
    trendChartRenderModel = null,
    trendChartLastTrace = null,
    trendChartPoints = emptyList(),
    trendChartAverageDurationSeconds = null,
    trendChartTotalDurationSeconds = null,
    trendChartActiveDays = null,
    trendChartRangeDays = null,
    trendChartUsesLegacyStatsFallback = false,
    trendChartLoading = false,
    trendChartError = "",
    compositionChartRenderModel = null,
    compositionChartLastTrace = null,
    compositionChartLoading = false,
    compositionChartError = ""
)

private fun QueryResult?.isAnalysisResultForDifferentPeriod(
    selectedPeriod: DataTreePeriod
): Boolean = when (this) {
    is QueryResult.Tree -> period != selectedPeriod
    else -> false
}

