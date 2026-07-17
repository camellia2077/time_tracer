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

    private sealed interface QueryReportIntent {
        data object ReportDay : QueryReportIntent
        data object ReportMonth : QueryReportIntent
        data object ReportYear : QueryReportIntent
        data object ReportWeek : QueryReportIntent
        data object ReportRecent : QueryReportIntent
        data object ReportRange : QueryReportIntent
        data class LoadStats(val period: DataTreePeriod) : QueryReportIntent
        data class LoadTree(val period: DataTreePeriod, val level: Int) : QueryReportIntent
        data object LoadChart : QueryReportIntent
    }

    private fun digitsOnly(value: String, maxLength: Int): String =
        value.filter { it.isDigit() }.take(maxLength)

    fun onReportDateChange(value: String) {
        updateReportParams {
            copy(reportDate = digitsOnly(value, 8))
        }
    }

    fun onReportModeChange(mode: ReportMode) {
        updateReportParams {
            val hidesStaleAnalysis = activeResult.isAnalysisResultForDifferentPeriod(
                mode.toDataTreePeriod()
            )
            copy(
                reportMode = mode,
                // Stats and Project Tree cards are scoped to the period used to query them.
                // Do not retain an analysis card after changing to a different report window.
                activeResult = if (hidesStaleAnalysis) null else activeResult,
                analysisError = if (hidesStaleAnalysis) "" else analysisError,
                chartSemanticMode = preferredChartSemanticMode.normalizeForReportMode(mode)
            )
        }
    }

    fun onReportMonthChange(value: String) {
        updateReportParams {
            copy(reportMonth = digitsOnly(value, 6))
        }
    }

    fun onReportYearChange(value: String) {
        updateReportParams {
            copy(reportYear = digitsOnly(value, 4))
        }
    }

    fun onReportWeekChange(value: String) {
        updateReportParams {
            copy(reportWeek = digitsOnly(value, 6))
        }
    }

    fun onReportRecentDaysChange(value: String) {
        updateReportParams {
            copy(reportRecentDays = value.filter { it.isDigit() })
        }
    }

    fun refreshReportDayDefault() {
        // Report day parameters are intentionally session-local. When users return to the
        // Report tab, the day field should reflect the current logical log day instead of
        // preserving a previously inspected date from an earlier tab visit.
        val currentLogicalDayDigits = currentDateDigits(clock)
        if (uiState.reportDate == currentLogicalDayDigits) {
            return
        }
        uiState = uiState.copy(reportDate = currentLogicalDayDigits)
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
        if (uiState.parameterSection == section) {
            return
        }
        uiState = uiState.copy(parameterSection = section)
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
        updateReportParams {
            copy(reportRangeStartDate = digitsOnly(value, 8))
        }
    }

    fun onReportRangeEndDateChange(value: String) {
        updateReportParams {
            copy(reportRangeEndDate = digitsOnly(value, 8))
        }
    }

    fun reportDay() {
        dispatchIntent(QueryReportIntent.ReportDay)
    }

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

    fun loadDayStats(period: DataTreePeriod) {
        dispatchIntent(QueryReportIntent.LoadStats(period))
    }

    fun loadTree(
        period: DataTreePeriod,
        level: Int = -1
    ) {
        dispatchIntent(QueryReportIntent.LoadTree(period, level))
    }

    fun loadChart() {
        dispatchIntent(QueryReportIntent.LoadChart)
    }

    private fun updateReportParams(
        transform: QueryReportUiState.() -> QueryReportUiState
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
        is QueryResult.Stats -> period != selectedPeriod
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

                is QueryReportIntent.LoadStats -> {
                    useCases.loadStats(
                        currentState = uiState,
                        period = intent.period,
                        source = currentPeriodSource()
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
