package com.example.tracer

import android.util.Log
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch
import java.time.Clock



class QueryReportViewModel(
    reportGateway: ReportGateway,
    private val queryGateway: QueryGateway,
    private val recordGateway: RecordGateway? = null,
    textProvider: QueryReportTextProvider = DefaultQueryReportTextProvider,
    private val clock: Clock = Clock.systemDefaultZone()
) : ViewModel() {
    private companion object {
        const val REPORT_CHART_LOG_TAG = "TracerReportChart"
    }

    var uiState by mutableStateOf(initialQueryReportUiState(clock))
        private set
    private val useCases = QueryReportUseCases(
        reportGateway = reportGateway,
        queryGateway = queryGateway,
        textProvider = textProvider
    )
    private val inputValidator = QueryInputValidator(textProvider)
    private var reportPresentationPreferencesApplied = false
    // Chart queries can overlap while DataStore restores the persisted chart selection. Keep
    // only the newest query's state updates so a delayed request for the transient selection
    // cannot replace the chart selected by the user (or by the restored preferences).
    private var chartRequestGeneration = 0L

    private sealed interface QueryReportIntent {
        data object ReportDay : QueryReportIntent
        data object ReportMonth : QueryReportIntent
        data object ReportYear : QueryReportIntent
        data object ReportWeek : QueryReportIntent
        data object ReportRecent : QueryReportIntent
        data object ReportRange : QueryReportIntent
        data class LoadTree(val period: DataTreePeriod, val level: Int) : QueryReportIntent
        data class LoadChart(val generation: Long) : QueryReportIntent
    }

    private fun digitsOnly(value: String, maxLength: Int): String =
        value.filter { it.isDigit() }.take(maxLength)

    fun onReportDateChange(value: String) {
        updateReportParams({
            copy(reportDate = digitsOnly(value, 8))
        }, autoReport = true)
    }

    fun onReportModeChange(mode: ReportMode) {
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

    fun onPersistedReportModeChange(mode: ReportMode) {
        if (uiState.reportMode == mode) {
            return
        }
        onReportModeChange(mode)
    }

    fun onReportMonthChange(value: String) {
        updateReportParams({
            copy(reportMonth = digitsOnly(value, 6))
        }, autoReport = true)
    }

    fun onReportYearChange(value: String) {
        updateReportParams({
            copy(reportYear = digitsOnly(value, 4))
        }, autoReport = true)
    }

    fun onReportWeekChange(value: String) {
        updateReportParams({
            copy(reportWeek = digitsOnly(value, 6))
        }, autoReport = true)
    }

    fun onReportRecentDaysChange(value: String) {
        updateReportParams({
            copy(reportRecentDays = value.filter { it.isDigit() })
        }, autoReport = true)
    }

    fun refreshReportDayDefault() {
        refreshReportDayDefault(refresh = true)
    }

    fun onReportTabEntered() {
        // On a cold start the tab lifecycle reaches here before DataStore has delivered the
        // report presentation preferences. Do not issue the default TEXT query yet: it can
        // finish after the persisted CHART query and overwrite the visible chart with a
        // Markdown/no-data state.
        logChart("tab entered; preferencesApplied=$reportPresentationPreferencesApplied ${chartSelection()}")
        refreshReportCalendarAvailability()
        refreshReportDayDefault(refresh = reportPresentationPreferencesApplied)
    }

    private fun refreshReportCalendarAvailability() {
        viewModelScope.launch {
            val result = queryGateway.queryReportCalendarAvailability()
            if (result.ok) {
                uiState = uiState.copy(availableReportMonths = result.months.distinct().sorted())
            }
        }
    }

    private fun refreshReportDayDefault(refresh: Boolean) {
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

    fun applyPersistedReportPresentation(
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

    fun applyReportAverageDayBasis(value: ReportAverageDayBasis) {
        if (uiState.averageDayBasis != value) {
            invalidateInFlightChartRequests("average day basis")
            uiState = uiState.copy(averageDayBasis = value).invalidateChartState()
            if (uiState.resultDisplayMode == ReportResultDisplayMode.CHART) {
                refreshCurrentChart()
            }
        }
    }

    fun onResultDisplayModeChange(mode: ReportResultDisplayMode) {
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

    fun onChartRootChange(root: String) {
        invalidateInFlightChartRequests("trend root -> $root")
        uiState = uiState.copy(trendChartSelectedRoot = root).invalidateChartState()
        logChart("trend root applied; ${chartSelection()}")
        if (uiState.resultDisplayMode == ReportResultDisplayMode.CHART &&
            uiState.chartSemanticMode == ReportChartSemanticMode.TREND
        ) {
            loadChart()
        }
    }

    fun onChartSemanticModeChange(mode: ReportChartSemanticMode) {
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

    fun onParameterSectionChange(section: ReportParameterSection) {
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

    fun onPersistedChartSemanticModeChange(mode: ReportChartSemanticMode) {
        // The persisted preference can arrive after the Chart display has already
        // been selected. Apply it through the same path as a user selection so the
        // newly visible semantic chart is hydrated instead of showing an empty state.
        onChartSemanticModeChange(mode)
    }

    fun onCompositionVisualModeChange(mode: ReportCompositionVisualMode) {
        if (uiState.compositionVisualMode == mode) {
            return
        }
        uiState = uiState.copy(compositionVisualMode = mode)
    }

    fun onReportRangeStartDateChange(value: String) {
        updateReportParams({
            copy(reportRangeStartDate = digitsOnly(value, 8))
        }, autoReport = true)
    }

    fun onReportRangeEndDateChange(value: String) {
        updateReportParams({
            copy(reportRangeEndDate = digitsOnly(value, 8))
        }, autoReport = true)
    }

    fun reportDay() {
        dispatchIntent(QueryReportIntent.ReportDay)
    }

    suspend fun updateActivityRemark(
        activity: ActivityTimelineItem,
        remark: String
    ): RecordActionResult {
        val gateway = recordGateway ?: return RecordActionResult(
            ok = false,
            message = "Report Timeline remark editing is not wired."
        )
        val dayDigits = uiState.reportDate.trim()
        val validationError = inputValidator.validateDateDigits(dayDigits)
        if (validationError != null) {
            return RecordActionResult(ok = false, message = validationError)
        }
        val result = gateway.updateActivityRemark(
            targetDateIso = inputValidator.toIsoDate(dayDigits),
            logicalId = activity.logicalId,
            remark = remark,
            preferredTxtPath = null
        )
        if (result.ok) {
            uiState = uiState.updateLocalActivityRemark(activity.logicalId, remark)
        }
        return result
    }

    suspend fun updateDayRemark(remark: String): RecordActionResult {
        val gateway = recordGateway ?: return RecordActionResult(
            ok = false,
            message = "Report Timeline day remark editing is not wired."
        )
        val dayDigits = uiState.reportDate.trim()
        val validationError = inputValidator.validateDateDigits(dayDigits)
        if (validationError != null) {
            return RecordActionResult(ok = false, message = validationError)
        }
        val result = gateway.updateDayRemark(
            targetDateIso = inputValidator.toIsoDate(dayDigits),
            remark = remark,
            preferredTxtPath = null
        )
        if (result.ok) {
            uiState = uiState.updateLocalDayRemark(remark)
        }
        return result
    }

    private fun QueryReportUiState.updateLocalActivityRemark(
        logicalId: Long,
        remark: String
    ): QueryReportUiState {
        val timeline = dayTimeline ?: return copy(dayReportNeedsRefresh = true)
        val updatedActivities = timeline.activities.map { activity ->
            if (activity.logicalId == logicalId) activity.copy(remark = remark) else activity
        }
        return copy(
            dayTimeline = timeline.copy(activities = updatedActivities),
            dayReportNeedsRefresh = true
        )
    }

    private fun QueryReportUiState.updateLocalDayRemark(
        remark: String
    ): QueryReportUiState = copy(
        dayTimeline = dayTimeline?.copy(dayRemark = remark),
        dayReportNeedsRefresh = true
    )

    fun reportMonth() {
        dispatchIntent(QueryReportIntent.ReportMonth)
    }

    fun reportYear() {
        dispatchIntent(QueryReportIntent.ReportYear)
    }

    fun reportWeek() {
        dispatchIntent(QueryReportIntent.ReportWeek)
    }

    fun reportRecent() {
        dispatchIntent(QueryReportIntent.ReportRecent)
    }

    fun reportRange() {
        dispatchIntent(QueryReportIntent.ReportRange)
    }

    fun reportCurrentSelection() {
        if (uiState.resultDisplayMode != ReportResultDisplayMode.TEXT ||
            uiState.parameterSection == ReportParameterSection.ACTIVITY_HIERARCHY
        ) {
            return
        }
        dispatchIntent(
            when (uiState.reportMode) {
                ReportMode.DAY -> QueryReportIntent.ReportDay
                ReportMode.MONTH -> QueryReportIntent.ReportMonth
                ReportMode.YEAR -> QueryReportIntent.ReportYear
                ReportMode.WEEK -> QueryReportIntent.ReportWeek
                ReportMode.RANGE -> QueryReportIntent.ReportRange
                ReportMode.RECENT -> QueryReportIntent.ReportRecent
            }
        )
    }

    /**
     * Report text is generated by core, so a UI language change must invalidate the
     * cached Markdown and issue the same query with the new report locale.
     */
    fun onReportLocaleChange(locale: String) {
        if (locale == useCases.currentReportLocale()) {
            return
        }
        // Locale is restored before the presentation preferences during cold start. Updating
        // the locale is safe, but querying here is not: wait for applyPersistedReportPresentation
        // to select TEXT or CHART before generating its first result.
        useCases.updateReportLocale(locale)
        if (reportPresentationPreferencesApplied) {
            refreshCurrentResult()
        }
    }

    fun loadTree(
        period: DataTreePeriod,
        level: Int = uiState.treeLevel
    ) {
        uiState = uiState.copy(treeLevel = level.coerceAtLeast(-1))
        dispatchIntent(QueryReportIntent.LoadTree(period, level))
    }

    fun onTreeLevelChange(level: Int) {
        loadTree(
            period = uiState.reportMode.toDataTreePeriod(),
            level = level
        )
    }

    fun loadChart() {
        chartRequestGeneration += 1L
        logChart("chart request queued; generation=$chartRequestGeneration ${chartSelection()}")
        dispatchIntent(QueryReportIntent.LoadChart(chartRequestGeneration))
    }

    private fun refreshCurrentResult() {
        if (uiState.resultDisplayMode == ReportResultDisplayMode.CHART) {
            refreshCurrentChart()
        } else {
            reportCurrentSelection()
        }
    }

    private fun refreshCurrentChart() {
        if (uiState.isChartLoading()) {
            logChart("chart refresh skipped because current semantic is loading; ${chartSelection()}")
            return
        }
        // Re-entering the Report/Chart page must reflect any records edited while it was
        // hidden. Do not reuse an in-memory chart model for that navigation refresh.
        useCases.invalidateChartCache()
        loadChart()
    }

    private fun updateReportParams(
        transform: QueryReportUiState.() -> QueryReportUiState,
        autoReport: Boolean = false
    ) {
        invalidateInFlightChartRequests("report parameters changed")
        val nextState = uiState.transform().invalidateChartState()
        val shouldReloadChart = nextState.resultDisplayMode == ReportResultDisplayMode.CHART &&
            !nextState.isChartLoading()
        uiState = nextState
        logChart("report parameters applied; reloadChart=$shouldReloadChart ${chartSelection()}")
        // Report parameters define the chart query window, so any parameter change must
        // invalidate the current chart instead of leaving a stale series on screen.
        if (shouldReloadChart) {
            loadChart()
        }
        if (autoReport && shouldAutoReport(nextState)) {
            if (nextState.parameterSection == ReportParameterSection.ACTIVITY_HIERARCHY) {
                loadTree(nextState.reportMode.toDataTreePeriod(), nextState.treeLevel)
            } else {
                reportCurrentSelection()
            }
        }
    }

    private fun shouldAutoReport(state: QueryReportUiState): Boolean {
        if (state.resultDisplayMode != ReportResultDisplayMode.TEXT) {
            return false
        }
        return when (state.reportMode) {
            ReportMode.DAY -> inputValidator.validateDateDigits(state.reportDate).isNullOrBlank()
            ReportMode.MONTH -> inputValidator.validateMonthDigits(state.reportMonth).isNullOrBlank()
            ReportMode.YEAR -> inputValidator.validateIsoYear(state.reportYear).isNullOrBlank()
            ReportMode.WEEK -> inputValidator.validateWeekDigits(state.reportWeek).isNullOrBlank()
            ReportMode.RANGE ->
                inputValidator.validateDateDigits(state.reportRangeStartDate).isNullOrBlank() &&
                    inputValidator.validateDateDigits(state.reportRangeEndDate).isNullOrBlank() &&
                    inputValidator.validateRangeOrder(
                        state.reportRangeStartDate,
                        state.reportRangeEndDate
                    ).isNullOrBlank()
            ReportMode.RECENT -> inputValidator.validateRecentDays(state.reportRecentDays)
                .isNullOrBlank()
        }
    }

    private fun QueryReportUiState.invalidateChartState(): QueryReportUiState = copy(
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

    private fun QueryReportUiState.isChartLoading(): Boolean =
        when (chartSemanticMode.normalizeForReportMode(reportMode)) {
            ReportChartSemanticMode.TREND -> trendChartLoading
            ReportChartSemanticMode.COMPOSITION -> compositionChartLoading
        }

    private fun QueryReportUiState.hasChartData(): Boolean =
        when (chartSemanticMode.normalizeForReportMode(reportMode)) {
            ReportChartSemanticMode.TREND -> trendChartRenderModel != null
            ReportChartSemanticMode.COMPOSITION ->
                compositionChartRenderModel != null
        }

    private fun invalidateInFlightChartRequests(reason: String) {
        val previousGeneration = chartRequestGeneration
        chartRequestGeneration += 1L
        logChart(
            "chart request invalidated; reason=$reason generation=$previousGeneration->$chartRequestGeneration " +
                chartSelection()
        )
    }

    private fun chartSelection(state: QueryReportUiState = uiState): String =
        "display=${state.resultDisplayMode} semantic=${state.chartSemanticMode} " +
            "mode=${state.reportMode} date=${state.reportDate} month=${state.reportMonth} " +
            "week=${state.reportWeek} year=${state.reportYear} range=${state.reportRangeStartDate}-" +
            "${state.reportRangeEndDate} recent=${state.reportRecentDays} root=${state.trendChartSelectedRoot}"

    private fun chartResultSummary(state: QueryReportUiState): String =
        "trendLoading=${state.trendChartLoading} trendPoints=${state.trendChartPoints.size} " +
            "trendError=${state.trendChartError.ifBlank { "<none>" }} " +
            "compositionLoading=${state.compositionChartLoading} " +
            "compositionData=${state.compositionChartRenderModel != null} " +
            "compositionError=${state.compositionChartError.ifBlank { "<none>" }}"

    private fun logChart(message: String) {
        runCatching { Log.i(REPORT_CHART_LOG_TAG, message) }
    }

    private fun dispatchIntent(intent: QueryReportIntent) {
        // Capture the complete query selection before launching. The coroutine may start after
        // another UI event has changed uiState; using the later state would make this request
        // appear to belong to the new selection and defeat the generation guard below.
        val chartRequestState = if (intent is QueryReportIntent.LoadChart) uiState else null
        if (intent is QueryReportIntent.LoadChart) {
            logChart(
                "chart request started; generation=${intent.generation} " +
                    chartSelection(requireNotNull(chartRequestState))
            )
        } else {
            logChart(
                "report request started; kind=${intent.logName()} " +
                    "generation=$chartRequestGeneration ${chartSelection()}"
            )
        }
        viewModelScope.launch {
            if (intent is QueryReportIntent.LoadChart) {
                val result = useCases.loadChart(
                    currentState = requireNotNull(chartRequestState),
                    emit = { state ->
                        if (intent.generation == chartRequestGeneration) {
                            logChart(
                                "chart state accepted; generation=${intent.generation} " +
                                    "${chartResultSummary(state)}"
                            )
                            uiState = state
                        } else {
                            logChart(
                                "chart state dropped; request=${intent.generation} " +
                                    "current=$chartRequestGeneration ${chartResultSummary(state)}"
                            )
                        }
                    }
                )
                // Do not return the chart result through the outer `uiState = when (...)`
                // assignment. The pipeline emits its loading state while that expression is
                // evaluating; committing the terminal state explicitly afterwards makes the
                // final chart model the last state write for this request.
                if (intent.generation == chartRequestGeneration) {
                    uiState = result
                    logChart(
                        "chart result committed; generation=${intent.generation} " +
                            "${chartResultSummary(uiState)}"
                    )
                } else {
                    logChart(
                        "chart result dropped; request=${intent.generation} " +
                            "current=$chartRequestGeneration ${chartResultSummary(result)}"
                    )
                }
                return@launch
            }
            uiState = when (intent) {
                QueryReportIntent.ReportDay -> {
                    useCases.reportDay(
                        currentState = uiState,
                        emit = { state -> uiState = state }
                    )
                }

                QueryReportIntent.ReportMonth -> {
                    useCases.reportMonth(
                        currentState = uiState,
                        emit = { state -> uiState = state }
                    )
                }

                QueryReportIntent.ReportYear -> {
                    useCases.reportYear(
                        currentState = uiState,
                        emit = { state -> uiState = state }
                    )
                }

                QueryReportIntent.ReportWeek -> {
                    useCases.reportWeek(
                        currentState = uiState,
                        emit = { state -> uiState = state }
                    )
                }

                QueryReportIntent.ReportRecent -> {
                    useCases.reportRecent(
                        currentState = uiState,
                        emit = { state -> uiState = state }
                    )
                }

                QueryReportIntent.ReportRange -> {
                    useCases.reportRange(
                        currentState = uiState,
                        emit = { state -> uiState = state }
                    )
                }

                is QueryReportIntent.LoadTree -> {
                    useCases.loadTree(
                        currentState = uiState,
                        period = intent.period,
                        level = intent.level,
                        source = currentPeriodSource()
                    )
                }

                is QueryReportIntent.LoadChart -> error("Chart intents are handled before dispatch")
            }
            logChart(
                "report request committed; kind=${intent.logName()} " +
                    "generation=$chartRequestGeneration ${chartSelection()} " +
                    "status=${uiState.statusText.take(120)}"
            )
        }
    }

    private fun QueryReportIntent.logName(): String = when (this) {
        QueryReportIntent.ReportDay -> "DAY"
        QueryReportIntent.ReportMonth -> "MONTH"
        QueryReportIntent.ReportYear -> "YEAR"
        QueryReportIntent.ReportWeek -> "WEEK"
        QueryReportIntent.ReportRecent -> "RECENT"
        QueryReportIntent.ReportRange -> "RANGE"
        is QueryReportIntent.LoadTree -> "TREE:${period}"
        is QueryReportIntent.LoadChart -> "CHART"
    }

    private fun currentPeriodSource(): QueryPeriodSource = QueryPeriodSource(
        dayDigits = uiState.reportDate,
        monthDigits = uiState.reportMonth,
        yearDigits = uiState.reportYear,
        weekDigits = uiState.reportWeek,
        rangeStartDigits = uiState.reportRangeStartDate,
        rangeEndDigits = uiState.reportRangeEndDate,
        recentDays = uiState.reportRecentDays
    )
}
