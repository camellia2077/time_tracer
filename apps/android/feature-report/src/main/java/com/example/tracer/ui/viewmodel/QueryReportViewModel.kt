package com.example.tracer

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch
import java.time.Clock



class QueryReportViewModel(
    reportGateway: ReportGateway,
    queryGateway: QueryGateway,
    private val recordGateway: RecordGateway? = null,
    textProvider: QueryReportTextProvider = DefaultQueryReportTextProvider,
    private val clock: Clock = Clock.systemDefaultZone()
) : ViewModel() {
    var uiState by mutableStateOf(initialQueryReportUiState(clock))
        private set
    private val useCases = QueryReportUseCases(
        reportGateway = reportGateway,
        queryGateway = queryGateway,
        textProvider = textProvider
    )
    private val inputValidator = QueryInputValidator(textProvider)

    private sealed interface QueryReportIntent {
        data object ReportDay : QueryReportIntent
        data object ReportMonth : QueryReportIntent
        data object ReportYear : QueryReportIntent
        data object ReportWeek : QueryReportIntent
        data object ReportRecent : QueryReportIntent
        data object ReportRange : QueryReportIntent
        data class LoadTree(val period: DataTreePeriod, val level: Int) : QueryReportIntent
        data object LoadChart : QueryReportIntent
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
        if (uiState.parameterSection == ReportParameterSection.TREE) {
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
        reportCurrentSelection()
    }

    fun onResultDisplayModeChange(mode: ReportResultDisplayMode) {
        val normalizedState = if (mode == ReportResultDisplayMode.CHART) {
            uiState.copy(
                resultDisplayMode = mode,
                chartSemanticMode = uiState.preferredChartSemanticMode
                    .normalizeForReportMode(uiState.reportMode)
            )
        } else {
            uiState.copy(resultDisplayMode = mode)
        }
        uiState = normalizedState
        if (mode == ReportResultDisplayMode.CHART &&
            !normalizedState.isChartLoading() &&
            !normalizedState.hasChartData()
        ) {
            loadChart()
        }
    }

    fun onChartRootChange(root: String) {
        uiState = uiState.copy(trendChartSelectedRoot = root)
        if (uiState.resultDisplayMode == ReportResultDisplayMode.CHART &&
            uiState.chartSemanticMode == ReportChartSemanticMode.TREND &&
            !uiState.trendChartLoading
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
        uiState = uiState.copy(
            chartSemanticMode = normalizedMode,
            preferredChartSemanticMode = mode
        )
        if (uiState.resultDisplayMode == ReportResultDisplayMode.CHART &&
            !uiState.isChartLoading() &&
            !uiState.hasChartData()
        ) {
            loadChart()
        }
    }

    fun onParameterSectionChange(section: ReportParameterSection) {
        if (uiState.reportMode != ReportMode.DAY && section == ReportParameterSection.TIMELINE) {
            return
        }
        val sectionChanged = uiState.parameterSection != section
        uiState = uiState.copy(parameterSection = section)
        if (section == ReportParameterSection.TREE) {
            if (sectionChanged) {
                loadTree(uiState.reportMode.toDataTreePeriod(), uiState.treeLevel)
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
                section != ReportParameterSection.TREE
            if (!hasCurrentReport || needsReportRefresh) {
                reportCurrentSelection()
            }
        }
    }

    fun onPersistedChartSemanticModeChange(mode: ReportChartSemanticMode) {
        val normalizedMode = mode.normalizeForReportMode(uiState.reportMode)
        if (uiState.preferredChartSemanticMode == mode &&
            uiState.chartSemanticMode == normalizedMode
        ) {
            return
        }
        uiState = uiState.copy(
            chartSemanticMode = normalizedMode,
            preferredChartSemanticMode = mode
        )
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
            uiState.parameterSection == ReportParameterSection.TREE
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
        useCases.updateReportLocale(locale)
        reportCurrentSelection()
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
        dispatchIntent(QueryReportIntent.LoadChart)
    }

    private fun updateReportParams(
        transform: QueryReportUiState.() -> QueryReportUiState,
        autoReport: Boolean = false
    ) {
        val nextState = uiState.transform().invalidateChartState()
        val shouldReloadChart = nextState.resultDisplayMode == ReportResultDisplayMode.CHART &&
            !nextState.isChartLoading()
        uiState = nextState
        // Report parameters define the chart query window, so any parameter change must
        // invalidate the current chart instead of leaving a stale series on screen.
        if (shouldReloadChart) {
            loadChart()
        }
        if (autoReport && shouldAutoReport(nextState)) {
            if (nextState.parameterSection == ReportParameterSection.TREE) {
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

    private fun dispatchIntent(intent: QueryReportIntent) {
        viewModelScope.launch {
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

                QueryReportIntent.LoadChart -> {
                    useCases.loadChart(
                        currentState = uiState,
                        emit = { state -> uiState = state }
                    )
                }
            }
        }
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
