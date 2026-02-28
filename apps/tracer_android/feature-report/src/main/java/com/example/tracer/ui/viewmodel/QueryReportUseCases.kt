package com.example.tracer

internal class QueryReportUseCases(
    private val reportGateway: ReportGateway,
    private val queryGateway: QueryGateway,
    private val textProvider: QueryReportTextProvider = DefaultQueryReportTextProvider,
    private val inputValidator: QueryInputValidator = QueryInputValidator(textProvider),
    private val periodArgumentResolver: QueryPeriodArgumentResolver =
        QueryPeriodArgumentResolver(textProvider),
    private val chartUseCase: QueryReportChartUseCase = QueryReportChartUseCase(
        queryGateway = queryGateway,
        inputValidator = inputValidator,
        textProvider = textProvider
    )
) {
    suspend fun reportDay(
        currentState: QueryReportUiState,
        emit: (QueryReportUiState) -> Unit
    ): QueryReportUiState = runDayReportAction(
        currentState = currentState,
        inputValidator = inputValidator,
        textProvider = textProvider,
        reportGateway = reportGateway,
        emit = emit
    )

    suspend fun reportMonth(
        currentState: QueryReportUiState,
        emit: (QueryReportUiState) -> Unit
    ): QueryReportUiState = runMonthReportAction(
        currentState = currentState,
        inputValidator = inputValidator,
        textProvider = textProvider,
        reportGateway = reportGateway,
        emit = emit
    )

    suspend fun reportYear(
        currentState: QueryReportUiState,
        emit: (QueryReportUiState) -> Unit
    ): QueryReportUiState = runYearReportAction(
        currentState = currentState,
        inputValidator = inputValidator,
        textProvider = textProvider,
        reportGateway = reportGateway,
        emit = emit
    )

    suspend fun reportWeek(
        currentState: QueryReportUiState,
        emit: (QueryReportUiState) -> Unit
    ): QueryReportUiState = runWeekReportAction(
        currentState = currentState,
        inputValidator = inputValidator,
        textProvider = textProvider,
        reportGateway = reportGateway,
        emit = emit
    )

    suspend fun reportRecent(
        currentState: QueryReportUiState,
        emit: (QueryReportUiState) -> Unit
    ): QueryReportUiState = runRecentReportAction(
        currentState = currentState,
        inputValidator = inputValidator,
        textProvider = textProvider,
        reportGateway = reportGateway,
        emit = emit
    )

    suspend fun reportRange(
        currentState: QueryReportUiState,
        emit: (QueryReportUiState) -> Unit
    ): QueryReportUiState = runRangeReportAction(
        currentState = currentState,
        inputValidator = inputValidator,
        textProvider = textProvider,
        reportGateway = reportGateway,
        emit = emit
    )

    suspend fun loadStats(
        currentState: QueryReportUiState,
        period: DataTreePeriod,
        source: QueryPeriodSource
    ): QueryReportUiState = runStatsAnalysisAction(
        currentState = currentState,
        period = period,
        source = source,
        periodArgumentResolver = periodArgumentResolver,
        textProvider = textProvider,
        queryGateway = queryGateway
    )

    suspend fun loadTree(
        currentState: QueryReportUiState,
        period: DataTreePeriod,
        level: Int,
        source: QueryPeriodSource
    ): QueryReportUiState = runTreeAnalysisAction(
        currentState = currentState,
        period = period,
        level = level,
        source = source,
        periodArgumentResolver = periodArgumentResolver,
        textProvider = textProvider,
        queryGateway = queryGateway
    )

    suspend fun loadChart(
        currentState: QueryReportUiState,
        emit: (QueryReportUiState) -> Unit
    ): QueryReportUiState = chartUseCase.execute(
        currentState = currentState,
        emit = emit
    )
}
