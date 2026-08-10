package com.example.tracer

import java.util.Locale

internal class QueryInsightsUseCases(
    private val insightsGateway: InsightsGateway,
    private val queryGateway: QueryGateway,
    private val textProvider: QueryInsightsTextProvider = DefaultQueryInsightsTextProvider,
    private val inputValidator: QueryInputValidator = QueryInputValidator(textProvider),
    private val periodArgumentResolver: QueryPeriodArgumentResolver =
        QueryPeriodArgumentResolver(textProvider),
    private val chartUseCase: QueryInsightsChartUseCase = QueryInsightsChartUseCase(
        queryGateway = queryGateway,
        inputValidator = inputValidator,
        textProvider = textProvider
    ),
    private val compositionUseCase: QueryInsightsCompositionUseCase = QueryInsightsCompositionUseCase(
        queryGateway = queryGateway,
        inputValidator = inputValidator,
        textProvider = textProvider
    ),
    private var insightsLocale: String = Locale.getDefault().language
) {
    fun updateInsightsLocale(locale: String) {
        insightsLocale = locale
    }

    fun currentInsightsLocale(): String = insightsLocale

    fun invalidateChartCache() {
        chartUseCase.invalidateCache()
        compositionUseCase.invalidateCache()
    }

    suspend fun insightsDay(
        currentState: QueryInsightsUiState,
        emit: (QueryInsightsUiState) -> Unit
    ): QueryInsightsUiState = runDayInsightsAction(
        currentState = currentState,
        inputValidator = inputValidator,
        textProvider = textProvider,
        insightsGateway = insightsGateway,
        locale = insightsLocale,
        emit = emit
    )

    suspend fun insightsMonth(
        currentState: QueryInsightsUiState,
        emit: (QueryInsightsUiState) -> Unit
    ): QueryInsightsUiState = runMonthInsightsAction(
        currentState = currentState,
        inputValidator = inputValidator,
        textProvider = textProvider,
        insightsGateway = insightsGateway,
        locale = insightsLocale,
        emit = emit
    )

    suspend fun insightsYear(
        currentState: QueryInsightsUiState,
        emit: (QueryInsightsUiState) -> Unit
    ): QueryInsightsUiState = runYearInsightsAction(
        currentState = currentState,
        inputValidator = inputValidator,
        textProvider = textProvider,
        insightsGateway = insightsGateway,
        locale = insightsLocale,
        emit = emit
    )

    suspend fun insightsWeek(
        currentState: QueryInsightsUiState,
        emit: (QueryInsightsUiState) -> Unit
    ): QueryInsightsUiState = runWeekInsightsAction(
        currentState = currentState,
        inputValidator = inputValidator,
        textProvider = textProvider,
        insightsGateway = insightsGateway,
        locale = insightsLocale,
        emit = emit
    )

    suspend fun insightsRecent(
        currentState: QueryInsightsUiState,
        emit: (QueryInsightsUiState) -> Unit
    ): QueryInsightsUiState = runRecentInsightsAction(
        currentState = currentState,
        inputValidator = inputValidator,
        textProvider = textProvider,
        insightsGateway = insightsGateway,
        locale = insightsLocale,
        emit = emit
    )

    suspend fun insightsRange(
        currentState: QueryInsightsUiState,
        emit: (QueryInsightsUiState) -> Unit
    ): QueryInsightsUiState = runRangeInsightsAction(
        currentState = currentState,
        inputValidator = inputValidator,
        textProvider = textProvider,
        insightsGateway = insightsGateway,
        locale = insightsLocale,
        emit = emit
    )

    suspend fun loadTree(
        currentState: QueryInsightsUiState,
        period: DataTreePeriod,
        level: Int,
        source: QueryPeriodSource
    ): QueryInsightsUiState = runTreeAnalysisAction(
        currentState = currentState,
        period = period,
        level = level,
        source = source,
        periodArgumentResolver = periodArgumentResolver,
        textProvider = textProvider,
        queryGateway = queryGateway
    )

    suspend fun loadChart(
        currentState: QueryInsightsUiState,
        emit: (QueryInsightsUiState) -> Unit
    ): QueryInsightsUiState {
        val semanticMode = currentState.chartSemanticMode
            .normalizeForInsightsMode(currentState.insightsMode)
        return if (semanticMode == InsightsChartSemanticMode.COMPOSITION) {
            compositionUseCase.execute(
                currentState = currentState,
                emit = emit
            )
        } else {
            chartUseCase.execute(
                currentState = currentState,
                emit = emit
            )
        }
    }
}
