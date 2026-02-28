package com.example.tracer

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch

class QueryReportViewModel(
    reportGateway: ReportGateway,
    queryGateway: QueryGateway,
    textProvider: QueryReportTextProvider = DefaultQueryReportTextProvider
) : ViewModel() {
    var uiState by mutableStateOf(QueryReportUiState())
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
            uiState.chartRenderModel == null &&
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
