package com.example.tracer

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch

class QueryReportViewModel(
    private val reportGateway: ReportGateway,
    private val queryGateway: QueryGateway,
    private val textProvider: QueryReportTextProvider = DefaultQueryReportTextProvider
) : ViewModel() {
    var uiState by mutableStateOf(QueryReportUiState())
        private set

    private val periodArgumentResolver = QueryPeriodArgumentResolver(textProvider)
    private val inputValidator = QueryInputValidator(textProvider)

    private fun digitsOnly(value: String, maxLength: Int): String =
        value.filter { it.isDigit() }.take(maxLength)

    fun onReportDateChange(value: String) {
        uiState = uiState.copy(reportDate = digitsOnly(value, 8))
    }

    fun onReportMonthChange(value: String) {
        uiState = uiState.copy(reportMonth = digitsOnly(value, 6))
    }

    fun onReportYearChange(value: String) {
        uiState = uiState.copy(reportYear = digitsOnly(value, 4))
    }

    fun onReportWeekChange(value: String) {
        uiState = uiState.copy(reportWeek = digitsOnly(value, 6))
    }

    fun onReportRecentDaysChange(value: String) {
        uiState = uiState.copy(reportRecentDays = value.filter { it.isDigit() })
    }

    fun onResultDisplayModeChange(mode: ReportResultDisplayMode) {
        uiState = uiState.copy(resultDisplayMode = mode)
        if (mode == ReportResultDisplayMode.CHART &&
            !uiState.chartLoading &&
            uiState.chartRoots.isEmpty() &&
            uiState.chartPoints.isEmpty()
        ) {
            loadChart()
        }
    }

    fun onChartRootChange(root: String) {
        uiState = uiState.copy(chartSelectedRoot = root)
        if (uiState.resultDisplayMode == ReportResultDisplayMode.CHART &&
            !uiState.chartLoading
        ) {
            loadChart()
        }
    }

    fun onChartDateInputModeChange(mode: ChartDateInputMode) {
        uiState = uiState.copy(chartDateInputMode = mode)
    }

    fun onChartLookbackDaysChange(value: String) {
        uiState = uiState.copy(
            chartDateInputMode = ChartDateInputMode.LOOKBACK,
            chartLookbackDays = value.filter { it.isDigit() }.take(3)
        )
    }

    fun onChartRangeStartDateChange(value: String) {
        uiState = uiState.copy(
            chartDateInputMode = ChartDateInputMode.RANGE,
            chartRangeStartDate = digitsOnly(value, 8)
        )
    }

    fun onChartRangeEndDateChange(value: String) {
        uiState = uiState.copy(
            chartDateInputMode = ChartDateInputMode.RANGE,
            chartRangeEndDate = digitsOnly(value, 8)
        )
    }

    fun onReportRangeStartDateChange(value: String) {
        uiState = uiState.copy(reportRangeStartDate = digitsOnly(value, 8))
    }

    fun onReportRangeEndDateChange(value: String) {
        uiState = uiState.copy(reportRangeEndDate = digitsOnly(value, 8))
    }

    fun reportDay() {
        viewModelScope.launch { triggerDayReport() }
    }

    fun reportMonth() {
        viewModelScope.launch { triggerMonthReport() }
    }

    fun reportYear() {
        viewModelScope.launch { triggerYearReport() }
    }

    fun reportWeek() {
        viewModelScope.launch { triggerWeekReport() }
    }

    fun reportRecent() {
        viewModelScope.launch { triggerRecentReport() }
    }

    fun reportRange() {
        viewModelScope.launch { triggerRangeReport() }
    }

    fun loadDayStats(period: DataTreePeriod) {
        viewModelScope.launch { triggerStatsAnalysis(period) }
    }

    fun loadTree(
        period: DataTreePeriod,
        level: Int = -1
    ) {
        viewModelScope.launch { triggerTreeAnalysis(period, level) }
    }

    fun loadChart() {
        viewModelScope.launch { triggerChartQuery() }
    }

    private suspend fun triggerDayReport() {
        uiState = runDayReportAction(
            currentState = uiState,
            inputValidator = inputValidator,
            textProvider = textProvider,
            reportGateway = reportGateway,
            emit = { state -> uiState = state }
        )
    }

    private suspend fun triggerMonthReport() {
        uiState = runMonthReportAction(
            currentState = uiState,
            inputValidator = inputValidator,
            textProvider = textProvider,
            reportGateway = reportGateway,
            emit = { state -> uiState = state }
        )
    }

    private suspend fun triggerYearReport() {
        uiState = runYearReportAction(
            currentState = uiState,
            inputValidator = inputValidator,
            textProvider = textProvider,
            reportGateway = reportGateway,
            emit = { state -> uiState = state }
        )
    }

    private suspend fun triggerWeekReport() {
        uiState = runWeekReportAction(
            currentState = uiState,
            inputValidator = inputValidator,
            textProvider = textProvider,
            reportGateway = reportGateway,
            emit = { state -> uiState = state }
        )
    }

    private suspend fun triggerRecentReport() {
        uiState = runRecentReportAction(
            currentState = uiState,
            inputValidator = inputValidator,
            textProvider = textProvider,
            reportGateway = reportGateway,
            emit = { state -> uiState = state }
        )
    }

    private suspend fun triggerRangeReport() {
        uiState = runRangeReportAction(
            currentState = uiState,
            inputValidator = inputValidator,
            textProvider = textProvider,
            reportGateway = reportGateway,
            emit = { state -> uiState = state }
        )
    }

    private suspend fun triggerStatsAnalysis(period: DataTreePeriod) {
        uiState = runStatsAnalysisAction(
            currentState = uiState,
            period = period,
            source = currentPeriodSource(),
            periodArgumentResolver = periodArgumentResolver,
            textProvider = textProvider,
            queryGateway = queryGateway
        )
    }

    private suspend fun triggerTreeAnalysis(period: DataTreePeriod, level: Int) {
        uiState = runTreeAnalysisAction(
            currentState = uiState,
            period = period,
            level = level,
            source = currentPeriodSource(),
            periodArgumentResolver = periodArgumentResolver,
            textProvider = textProvider,
            queryGateway = queryGateway
        )
    }

    private suspend fun triggerChartQuery() {
        uiState = runChartQueryAction(
            currentState = uiState,
            inputValidator = inputValidator,
            textProvider = textProvider,
            queryGateway = queryGateway
        )
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
