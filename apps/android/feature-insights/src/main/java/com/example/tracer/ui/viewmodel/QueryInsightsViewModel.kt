package com.example.tracer

import android.util.Log
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch
import java.time.Clock



class QueryInsightsViewModel(
    insightsGateway: InsightsGateway,
    internal val queryGateway: QueryGateway,
    internal val recordGateway: RecordGateway? = null,
    textProvider: QueryInsightsTextProvider = DefaultQueryInsightsTextProvider,
    internal val clock: Clock = Clock.systemDefaultZone()
) : ViewModel() {
    private companion object {
        const val INSIGHTS_CHART_LOG_TAG = "TracerInsightsChart"
    }

    var uiState by mutableStateOf(initialQueryInsightsUiState(clock))
        internal set
    internal val useCases = QueryInsightsUseCases(
        insightsGateway = insightsGateway,
        queryGateway = queryGateway,
        textProvider = textProvider
    )
    internal val inputValidator = QueryInputValidator(textProvider)
    internal var insightsPresentationPreferencesApplied = false
    // Chart queries can overlap while DataStore restores the persisted chart selection. Keep
    // only the newest query's state updates so a delayed request for the transient selection
    // cannot replace the chart selected by the user (or by the restored preferences).
    internal var chartRequestGeneration = 0L

    private sealed interface QueryInsightsIntent {
        data object InsightsDay : QueryInsightsIntent
        data object InsightsMonth : QueryInsightsIntent
        data object InsightsYear : QueryInsightsIntent
        data object InsightsWeek : QueryInsightsIntent
        data object InsightsRecent : QueryInsightsIntent
        data object InsightsRange : QueryInsightsIntent
        data class LoadTree(val period: DataTreePeriod, val level: Int) : QueryInsightsIntent
        data class LoadChart(val generation: Long) : QueryInsightsIntent
    }

    fun insightsDay() {
        dispatchIntent(QueryInsightsIntent.InsightsDay)
    }

    suspend fun updateActivityRemark(
        activity: ActivityTimelineItem,
        remark: String
    ): RecordActionResult {
        val gateway = recordGateway ?: return RecordActionResult(
            ok = false,
            message = "Insights Timeline remark editing is not wired."
        )
        val dayDigits = uiState.insightsDate.trim()
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
            message = "Insights Timeline day remark editing is not wired."
        )
        val dayDigits = uiState.insightsDate.trim()
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

    private fun QueryInsightsUiState.updateLocalActivityRemark(
        logicalId: Long,
        remark: String
    ): QueryInsightsUiState {
        val timeline = dayTimeline ?: return copy(dayInsightsNeedsRefresh = true)
        val updatedActivities = timeline.activities.map { activity ->
            if (activity.logicalId == logicalId) activity.copy(remark = remark) else activity
        }
        return copy(
            dayTimeline = timeline.copy(activities = updatedActivities),
            dayInsightsNeedsRefresh = true
        )
    }

    private fun QueryInsightsUiState.updateLocalDayRemark(
        remark: String
    ): QueryInsightsUiState = copy(
        dayTimeline = dayTimeline?.copy(dayRemark = remark),
        dayInsightsNeedsRefresh = true
    )

    fun insightsMonth() {
        dispatchIntent(QueryInsightsIntent.InsightsMonth)
    }

    fun insightsYear() {
        dispatchIntent(QueryInsightsIntent.InsightsYear)
    }

    fun insightsWeek() {
        dispatchIntent(QueryInsightsIntent.InsightsWeek)
    }

    fun insightsRecent() {
        dispatchIntent(QueryInsightsIntent.InsightsRecent)
    }

    fun insightsRange() {
        dispatchIntent(QueryInsightsIntent.InsightsRange)
    }

    fun insightsCurrentSelection() {
        if (uiState.resultDisplayMode != InsightsResultDisplayMode.TEXT ||
            uiState.parameterSection == InsightsParameterSection.ACTIVITY_HIERARCHY
        ) {
            return
        }
        dispatchIntent(
            when (uiState.insightsMode) {
                InsightsMode.DAY -> QueryInsightsIntent.InsightsDay
                InsightsMode.MONTH -> QueryInsightsIntent.InsightsMonth
                InsightsMode.YEAR -> QueryInsightsIntent.InsightsYear
                InsightsMode.WEEK -> QueryInsightsIntent.InsightsWeek
                InsightsMode.RANGE -> QueryInsightsIntent.InsightsRange
                InsightsMode.RECENT -> QueryInsightsIntent.InsightsRecent
            }
        )
    }

    /**
     * Insights text is generated by core, so a UI language change must invalidate the
     * cached Markdown and issue the same query with the new insights locale.
     */
    fun onInsightsLocaleChange(locale: String) {
        if (locale == useCases.currentInsightsLocale()) {
            return
        }
        // Locale is restored before the presentation preferences during cold start. Updating
        // the locale is safe, but querying here is not: wait for applyPersistedInsightsPresentation
        // to select TEXT or CHART before generating its first result.
        useCases.updateInsightsLocale(locale)
        if (insightsPresentationPreferencesApplied) {
            refreshCurrentResult()
        }
    }

    fun loadTree(
        period: DataTreePeriod,
        level: Int = uiState.treeLevel
    ) {
        uiState = uiState.copy(treeLevel = level.coerceAtLeast(-1))
        dispatchIntent(QueryInsightsIntent.LoadTree(period, level))
    }

    fun onTreeLevelChange(level: Int) {
        loadTree(
            period = uiState.insightsMode.toDataTreePeriod(),
            level = level
        )
    }

    fun loadChart() {
        chartRequestGeneration += 1L
        logChart("chart request queued; generation=$chartRequestGeneration ${chartSelection()}")
        dispatchIntent(QueryInsightsIntent.LoadChart(chartRequestGeneration))
    }

    internal fun refreshCurrentResult() {
        if (uiState.resultDisplayMode == InsightsResultDisplayMode.CHART) {
            refreshCurrentChart()
        } else {
            insightsCurrentSelection()
        }
    }

    internal fun refreshCurrentChart() {
        if (uiState.isChartLoading()) {
            logChart("chart refresh skipped because current semantic is loading; ${chartSelection()}")
            return
        }
        // Re-entering the Insights/Chart page must reflect any records edited while it was
        // hidden. Do not reuse an in-memory chart model for that navigation refresh.
        useCases.invalidateChartCache()
        loadChart()
    }

    internal fun updateInsightsParams(
        transform: QueryInsightsUiState.() -> QueryInsightsUiState,
        autoInsights: Boolean = false
    ) {
        invalidateInFlightChartRequests("insights parameters changed")
        val nextState = uiState.transform().invalidateChartState()
        val shouldReloadChart = nextState.resultDisplayMode == InsightsResultDisplayMode.CHART &&
            !nextState.isChartLoading()
        uiState = nextState
        logChart("insights parameters applied; reloadChart=$shouldReloadChart ${chartSelection()}")
        // Insights parameters define the chart query window, so any parameter change must
        // invalidate the current chart instead of leaving a stale series on screen.
        if (shouldReloadChart) {
            loadChart()
        }
        if (autoInsights && shouldAutoInsights(nextState)) {
            if (nextState.parameterSection == InsightsParameterSection.ACTIVITY_HIERARCHY) {
                loadTree(nextState.insightsMode.toDataTreePeriod(), nextState.treeLevel)
            } else {
                insightsCurrentSelection()
            }
        }
    }

    private fun shouldAutoInsights(state: QueryInsightsUiState): Boolean {
        if (state.resultDisplayMode != InsightsResultDisplayMode.TEXT) {
            return false
        }
        return when (state.insightsMode) {
            InsightsMode.DAY -> inputValidator.validateDateDigits(state.insightsDate).isNullOrBlank()
            InsightsMode.MONTH -> inputValidator.validateMonthDigits(state.insightsMonth).isNullOrBlank()
            InsightsMode.YEAR -> inputValidator.validateIsoYear(state.insightsYear).isNullOrBlank()
            InsightsMode.WEEK -> inputValidator.validateWeekDigits(state.insightsWeek).isNullOrBlank()
            InsightsMode.RANGE ->
                inputValidator.validateDateDigits(state.insightsRangeStartDate).isNullOrBlank() &&
                    inputValidator.validateDateDigits(state.insightsRangeEndDate).isNullOrBlank() &&
                    inputValidator.validateRangeOrder(
                        state.insightsRangeStartDate,
                        state.insightsRangeEndDate
                    ).isNullOrBlank()
            InsightsMode.RECENT -> inputValidator.validateRecentDays(state.insightsRecentDays)
                .isNullOrBlank()
        }
    }


    private fun QueryInsightsUiState.isChartLoading(): Boolean =
        when (chartSemanticMode.normalizeForInsightsMode(insightsMode)) {
            InsightsChartSemanticMode.TREND -> trendChartLoading
            InsightsChartSemanticMode.COMPOSITION -> compositionChartLoading
        }

    private fun QueryInsightsUiState.hasChartData(): Boolean =
        when (chartSemanticMode.normalizeForInsightsMode(insightsMode)) {
            InsightsChartSemanticMode.TREND -> trendChartRenderModel != null
            InsightsChartSemanticMode.COMPOSITION ->
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

    internal fun chartSelection(state: QueryInsightsUiState = uiState): String =
        "display=${state.resultDisplayMode} semantic=${state.chartSemanticMode} " +
            "mode=${state.insightsMode} date=${state.insightsDate} month=${state.insightsMonth} " +
            "week=${state.insightsWeek} year=${state.insightsYear} range=${state.insightsRangeStartDate}-" +
            "${state.insightsRangeEndDate} recent=${state.insightsRecentDays} root=${state.trendChartSelectedRoot}"

    private fun chartResultSummary(state: QueryInsightsUiState): String =
        "trendLoading=${state.trendChartLoading} trendPoints=${state.trendChartPoints.size} " +
            "trendError=${state.trendChartError.ifBlank { "<none>" }} " +
            "compositionLoading=${state.compositionChartLoading} " +
            "compositionData=${state.compositionChartRenderModel != null} " +
            "compositionError=${state.compositionChartError.ifBlank { "<none>" }}"

    internal fun logChart(message: String) {
        runCatching { Log.i(INSIGHTS_CHART_LOG_TAG, message) }
    }

    private fun dispatchIntent(intent: QueryInsightsIntent) {
        // Capture the complete query selection before launching. The coroutine may start after
        // another UI event has changed uiState; using the later state would make this request
        // appear to belong to the new selection and defeat the generation guard below.
        val chartRequestState = if (intent is QueryInsightsIntent.LoadChart) uiState else null
        if (intent is QueryInsightsIntent.LoadChart) {
            logChart(
                "chart request started; generation=${intent.generation} " +
                    chartSelection(requireNotNull(chartRequestState))
            )
        } else {
            logChart(
                "insights request started; kind=${intent.logName()} " +
                    "generation=$chartRequestGeneration ${chartSelection()}"
            )
        }
        viewModelScope.launch {
            if (intent is QueryInsightsIntent.LoadChart) {
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
                QueryInsightsIntent.InsightsDay -> {
                    useCases.insightsDay(
                        currentState = uiState,
                        emit = { state -> uiState = state }
                    )
                }

                QueryInsightsIntent.InsightsMonth -> {
                    useCases.insightsMonth(
                        currentState = uiState,
                        emit = { state -> uiState = state }
                    )
                }

                QueryInsightsIntent.InsightsYear -> {
                    useCases.insightsYear(
                        currentState = uiState,
                        emit = { state -> uiState = state }
                    )
                }

                QueryInsightsIntent.InsightsWeek -> {
                    useCases.insightsWeek(
                        currentState = uiState,
                        emit = { state -> uiState = state }
                    )
                }

                QueryInsightsIntent.InsightsRecent -> {
                    useCases.insightsRecent(
                        currentState = uiState,
                        emit = { state -> uiState = state }
                    )
                }

                QueryInsightsIntent.InsightsRange -> {
                    useCases.insightsRange(
                        currentState = uiState,
                        emit = { state -> uiState = state }
                    )
                }

                is QueryInsightsIntent.LoadTree -> {
                    useCases.loadTree(
                        currentState = uiState,
                        period = intent.period,
                        level = intent.level,
                        source = currentPeriodSource()
                    )
                }

                is QueryInsightsIntent.LoadChart -> error("Chart intents are handled before dispatch")
            }
            logChart(
                "insights request committed; kind=${intent.logName()} " +
                    "generation=$chartRequestGeneration ${chartSelection()} " +
                    "status=${uiState.statusText.take(120)}"
            )
        }
    }

    private fun QueryInsightsIntent.logName(): String = when (this) {
        QueryInsightsIntent.InsightsDay -> "DAY"
        QueryInsightsIntent.InsightsMonth -> "MONTH"
        QueryInsightsIntent.InsightsYear -> "YEAR"
        QueryInsightsIntent.InsightsWeek -> "WEEK"
        QueryInsightsIntent.InsightsRecent -> "RECENT"
        QueryInsightsIntent.InsightsRange -> "RANGE"
        is QueryInsightsIntent.LoadTree -> "TREE:${period}"
        is QueryInsightsIntent.LoadChart -> "CHART"
    }

    private fun currentPeriodSource(): QueryPeriodSource = QueryPeriodSource(
        dayDigits = uiState.insightsDate,
        monthDigits = uiState.insightsMonth,
        yearDigits = uiState.insightsYear,
        weekDigits = uiState.insightsWeek,
        rangeStartDigits = uiState.insightsRangeStartDate,
        rangeEndDigits = uiState.insightsRangeEndDate,
        recentDays = uiState.insightsRecentDays
    )
}
