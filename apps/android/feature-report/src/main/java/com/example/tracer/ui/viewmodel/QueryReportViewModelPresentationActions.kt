package com.example.tracer

import android.util.Log
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch
import java.time.Clock




private fun QueryReportViewModel.digitsOnly(value: String, maxLength: Int): String =
    value.filter { it.isDigit() }.take(maxLength)

fun QueryReportViewModel.onReportDateChange(value: String) {
    updateReportParams({
        copy(reportDate = digitsOnly(value, 8))
    }, autoReport = true)
}

fun QueryReportViewModel.onReportModeChange(mode: ReportMode) {
    updateReportParams({
        val hidesStaleAnalysis = activeResult.isAnalysisResultForDifferentPeriod(
            mode.toDataTreePeriod()
        )
        copy(
            reportMode = mode,
            parameterSection = if (
                mode != ReportMode.DAY && parameterSection == ReportParameterSection.TIMELINE
            ) {
                ReportParameterSection.DAY
            } else {
                parameterSection
            },
            // Project Tree cards are scoped to the period used to query them.
            // Do not retain an analysis card after changing to a different report window.
            activeResult = if (hidesStaleAnalysis) null else activeResult,
            analysisError = if (hidesStaleAnalysis) "" else analysisError,
            chartSemanticMode = preferredChartSemanticMode.normalizeForReportMode(mode)
        )
    })
    // Chart and Tree are independent pipelines, but both write the shared UiState. Starting
    // Tree here while Chart is loading lets a slower Tree result restore an older chart
    // loading/empty snapshot after the Chart result has already committed. Tree is not
    // visible in Chart mode, so defer it until the user is actually viewing Text/Tree.
    if (uiState.resultDisplayMode != ReportResultDisplayMode.CHART &&
        uiState.parameterSection == ReportParameterSection.ACTIVITY_HIERARCHY
    ) {
        loadTree(mode.toDataTreePeriod(), uiState.treeLevel)
    }
}

fun QueryReportViewModel.onPersistedReportModeChange(mode: ReportMode) {
    if (uiState.reportMode == mode) {
        return
    }
    onReportModeChange(mode)
}

fun QueryReportViewModel.onReportMonthChange(value: String) {
    updateReportParams({
        copy(reportMonth = digitsOnly(value, 6))
    }, autoReport = true)
}

fun QueryReportViewModel.onReportYearChange(value: String) {
    updateReportParams({
        copy(reportYear = digitsOnly(value, 4))
    }, autoReport = true)
}

fun QueryReportViewModel.onReportWeekChange(value: String) {
    updateReportParams({
        copy(reportWeek = digitsOnly(value, 6))
    }, autoReport = true)
}

fun QueryReportViewModel.onReportRecentDaysChange(value: String) {
    updateReportParams({
        copy(reportRecentDays = value.filter { it.isDigit() })
    }, autoReport = true)
}

fun QueryReportViewModel.refreshReportDayDefault() {
    refreshReportDayDefault(refresh = true)
}

fun QueryReportViewModel.onReportTabEntered() {
    // On a cold start the tab lifecycle reaches here before DataStore has delivered the
    // report presentation preferences. Do not issue the default TEXT query yet: it can
    // finish after the persisted CHART query and overwrite the visible chart with a
    // Markdown/no-data state.
    logChart("tab entered; preferencesApplied=$reportPresentationPreferencesApplied ${chartSelection()}")
    refreshReportCalendarAvailability()
    refreshReportDayDefault(refresh = reportPresentationPreferencesApplied)
}

private fun QueryReportViewModel.refreshReportCalendarAvailability() {
    viewModelScope.launch {
        val result = queryGateway.queryReportCalendarAvailability()
        if (result.ok) {
            uiState = uiState.copy(availableReportMonths = result.months.distinct().sorted())
        }
    }
}

private fun QueryReportViewModel.refreshReportDayDefault(refresh: Boolean) {
    // Report day parameters are intentionally session-local. When users return to the
    // Report tab, the day field should reflect the current logical log day instead of
    // preserving a previously inspected date from an earlier tab visit.
    val currentLogicalDayDigits = currentDateDigits(clock)
    if (uiState.reportDate != currentLogicalDayDigits) {
        uiState = uiState.copy(reportDate = currentLogicalDayDigits)
    }
    // The underlying TXT may have changed while the user was on Record. The date often
    // remains the same, so changing only the parameter leaves the cached timeline stale.
    // Re-query the currently selected report on every Report-tab entry, including same-day
    // entries.
    // Tab entry can run before DataStore-backed report preferences are applied during a
    // cold start. Waiting avoids a default text-report request racing the chart request
    // and replacing an already rendered chart with its empty Markdown result.
    if (refresh) {
        refreshCurrentResult()
    }
}

fun QueryReportViewModel.applyPersistedReportPresentation(
    reportMode: ReportMode,
    chartSemanticMode: ReportChartSemanticMode,
    resultDisplayMode: ReportResultDisplayMode,
    parameterSection: ReportParameterSection,
    trendChartSelectedRoot: String = ""
) {
    // Treat the persisted display, period, semantic mode, and parameter section as one
    // selection. Applying them in separate Compose effects briefly leaves the ViewModel in
    // its default TEXT state, which starts a query whose delayed result can replace CHART.
    val normalizedParameterSection = if (
        reportMode != ReportMode.DAY && parameterSection == ReportParameterSection.TIMELINE
    ) {
        ReportParameterSection.DAY
    } else {
        parameterSection
    }
    val normalizedChartSemanticMode = chartSemanticMode.normalizeForReportMode(reportMode)
    val firstPreferenceApplication = !reportPresentationPreferencesApplied
    val changed = uiState.reportMode != reportMode ||
        uiState.preferredChartSemanticMode != chartSemanticMode ||
        uiState.chartSemanticMode != normalizedChartSemanticMode ||
        uiState.resultDisplayMode != resultDisplayMode ||
        uiState.parameterSection != normalizedParameterSection ||
        uiState.trendChartSelectedRoot != trendChartSelectedRoot.trim()
    reportPresentationPreferencesApplied = true
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
        reportMode = reportMode,
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
    if (resultDisplayMode == ReportResultDisplayMode.CHART) {
        refreshCurrentChart()
    } else if (normalizedParameterSection == ReportParameterSection.ACTIVITY_HIERARCHY) {
        loadTree(reportMode.toDataTreePeriod(), uiState.treeLevel)
    } else {
        reportCurrentSelection()
    }
}

fun QueryReportViewModel.applyReportAverageDayBasis(value: ReportAverageDayBasis) {
    if (uiState.averageDayBasis != value) {
        invalidateInFlightChartRequests("average day basis")
        uiState = uiState.copy(averageDayBasis = value).invalidateChartState()
        if (uiState.resultDisplayMode == ReportResultDisplayMode.CHART) {
            refreshCurrentChart()
        }
    }
}

fun QueryReportViewModel.onResultDisplayModeChange(mode: ReportResultDisplayMode) {
    invalidateInFlightChartRequests("display mode -> $mode")
    val normalizedState = if (mode == ReportResultDisplayMode.CHART) {
        uiState.copy(
            resultDisplayMode = mode,
            chartSemanticMode = uiState.preferredChartSemanticMode
                .normalizeForReportMode(uiState.reportMode)
        )
    } else {
        uiState.copy(resultDisplayMode = mode)
    }
    // Display changes can cancel an in-flight chart. Reset its loading state as well as
    // advancing the generation; otherwise returning to Chart could incorrectly believe a
    // cancelled request is still loading and never issue the replacement query.
    uiState = normalizedState.invalidateChartState()
    logChart("display mode applied; ${chartSelection()}")
    if (mode == ReportResultDisplayMode.CHART) {
        refreshCurrentChart()
    } else if (mode == ReportResultDisplayMode.TEXT) {
        // Switching from Chart to Text can happen after the report mode has already
        // changed. Re-query the current period here so Week/Month/etc. does not wait for
        // a later tab visit or another parameter change to populate the Markdown result.
        reportCurrentSelection()
    }
}

fun QueryReportViewModel.onChartRootChange(root: String) {
    invalidateInFlightChartRequests("trend root -> $root")
    uiState = uiState.copy(trendChartSelectedRoot = root).invalidateChartState()
    logChart("trend root applied; ${chartSelection()}")
    if (uiState.resultDisplayMode == ReportResultDisplayMode.CHART &&
        uiState.chartSemanticMode == ReportChartSemanticMode.TREND
    ) {
        loadChart()
    }
}

fun QueryReportViewModel.onChartSemanticModeChange(mode: ReportChartSemanticMode) {
    val normalizedMode = mode.normalizeForReportMode(uiState.reportMode)
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
    if (uiState.resultDisplayMode == ReportResultDisplayMode.CHART) {
        loadChart()
    }
}

fun QueryReportViewModel.onParameterSectionChange(section: ReportParameterSection) {
    if (uiState.reportMode != ReportMode.DAY && section == ReportParameterSection.TIMELINE) {
        return
    }
    val sectionChanged = uiState.parameterSection != section
    uiState = uiState.copy(parameterSection = section)
    if (section == ReportParameterSection.ACTIVITY_HIERARCHY) {
        val selectedPeriod = uiState.reportMode.toDataTreePeriod()
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
        // DAY, Markdown, and Timeline all consume the same cached day report. Two flicker
        // bugs were caused by treating a presentation-only section change as a new query:
        // returning to Timeline recreated the tab and re-ran the persisted-section effect,
        // while Timeline -> Markdown changed the section value from TIMELINE to DAY. Both
        // paths cleared dayTimeline before reloading the unchanged report. Re-query only
        // when the current period has no report yet, so switching or recreating these views
        // preserves the existing data and avoids the empty-state frame.
        val currentPeriod = uiState.reportMode.toDataTreePeriod()
        val hasCurrentReport = uiState.reportResultsByPeriod[currentPeriod] != null
        val needsReportRefresh = uiState.dayReportNeedsRefresh &&
            section != ReportParameterSection.ACTIVITY_HIERARCHY
        if (!hasCurrentReport || needsReportRefresh) {
            reportCurrentSelection()
        }
    }
}

fun QueryReportViewModel.onPersistedChartSemanticModeChange(mode: ReportChartSemanticMode) {
    // The persisted preference can arrive after the Chart display has already
    // been selected. Apply it through the same path as a user selection so the
    // newly visible semantic chart is hydrated instead of showing an empty state.
    onChartSemanticModeChange(mode)
}

fun QueryReportViewModel.onCompositionVisualModeChange(mode: ReportCompositionVisualMode) {
    if (uiState.compositionVisualMode == mode) {
        return
    }
    uiState = uiState.copy(compositionVisualMode = mode)
}

fun QueryReportViewModel.onReportRangeStartDateChange(value: String) {
    updateReportParams({
        copy(reportRangeStartDate = digitsOnly(value, 8))
    }, autoReport = true)
}

fun QueryReportViewModel.onReportRangeEndDateChange(value: String) {
    updateReportParams({
        copy(reportRangeEndDate = digitsOnly(value, 8))
    }, autoReport = true)
}


internal fun QueryReportUiState.invalidateChartState(): QueryReportUiState = copy(
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

