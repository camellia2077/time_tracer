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
    internal val queryGateway: QueryGateway,
    internal val recordGateway: RecordGateway? = null,
    textProvider: QueryReportTextProvider = DefaultQueryReportTextProvider,
    internal val clock: Clock = Clock.systemDefaultZone()
) : ViewModel() {
    private companion object {
        const val REPORT_CHART_LOG_TAG = "TracerReportChart"
    }

    var uiState by mutableStateOf(initialQueryReportUiState(clock))
        internal set
    internal val useCases = QueryReportUseCases(
        reportGateway = reportGateway,
        queryGateway = queryGateway,
        textProvider = textProvider
    )
    internal val inputValidator = QueryInputValidator(textProvider)
    internal var reportPresentationPreferencesApplied = false
    // Chart queries can overlap while DataStore restores the persisted chart selection. Keep
    // only the newest query's state updates so a delayed request for the transient selection
    // cannot replace the chart selected by the user (or by the restored preferences).
    internal var chartRequestGeneration = 0L

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

    internal fun refreshCurrentResult() {
        if (uiState.resultDisplayMode == ReportResultDisplayMode.CHART) {
            refreshCurrentChart()
        } else {
            reportCurrentSelection()
        }
    }

    internal fun refreshCurrentChart() {
        if (uiState.isChartLoading()) {
            logChart("chart refresh skipped because current semantic is loading; ${chartSelection()}")
            return
        }
        // Re-entering the Report/Chart page must reflect any records edited while it was
        // hidden. Do not reuse an in-memory chart model for that navigation refresh.
        useCases.invalidateChartCache()
        loadChart()
    }

    internal fun updateReportParams(
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

    internal fun invalidateInFlightChartRequests(reason: String) {
        val previousGeneration = chartRequestGeneration
        chartRequestGeneration += 1L
        logChart(
            "chart request invalidated; reason=$reason generation=$previousGeneration->$chartRequestGeneration " +
                chartSelection()
        )
    }

    internal fun chartSelection(state: QueryReportUiState = uiState): String =
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

    internal fun logChart(message: String) {
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
