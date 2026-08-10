package com.example.tracer

import java.time.LocalDate
import java.time.temporal.ChronoUnit

internal data class ResolvedChartQueryParams(
    val insightsMode: InsightsMode,
    val lookbackDays: Int,
    val fromDateIso: String?,
    val toDateIso: String?,
    val validationError: String
)

internal class QueryInsightsChartParamResolver(
    private val inputValidator: QueryInputValidator,
    private val textProvider: QueryInsightsTextProvider
) {
    private val selectionResolver = InsightsTemporalSelectionResolver(
        inputValidator = inputValidator,
        textProvider = textProvider
    )

    fun resolve(currentState: QueryInsightsUiState): ResolvedChartQueryParams {
        return when (val result = selectionResolver.resolve(currentState)) {
            is InsightsTemporalSelectionResolveResult.Failure -> invalid(
                insightsMode = result.insightsMode,
                validationError = result.validationError
            )
            is InsightsTemporalSelectionResolveResult.Success -> fromSelection(result.selection)
        }
    }

    private fun fromSelection(selection: InsightsTemporalSelection): ResolvedChartQueryParams {
        return when (selection) {
            is InsightsTemporalSelection.SingleDay -> ResolvedChartQueryParams(
                insightsMode = selection.insightsMode,
                lookbackDays = 1,
                fromDateIso = selection.date.toString(),
                toDateIso = selection.date.toString(),
                validationError = ""
            )
            is InsightsTemporalSelection.DateRange -> resolvedRange(
                insightsMode = selection.insightsMode,
                start = selection.start,
                end = selection.end
            )
            is InsightsTemporalSelection.RecentDays -> ResolvedChartQueryParams(
                insightsMode = selection.insightsMode,
                lookbackDays = selection.days,
                fromDateIso = null,
                toDateIso = null,
                validationError = ""
            )
        }
    }

    private fun resolvedRange(
        insightsMode: InsightsMode,
        start: LocalDate,
        end: LocalDate
    ): ResolvedChartQueryParams = ResolvedChartQueryParams(
        insightsMode = insightsMode,
        lookbackDays = inclusiveDayCount(start, end),
        fromDateIso = start.toString(),
        toDateIso = end.toString(),
        validationError = ""
    )

    private fun invalid(
        insightsMode: InsightsMode,
        validationError: String
    ): ResolvedChartQueryParams = ResolvedChartQueryParams(
        insightsMode = insightsMode,
        lookbackDays = 0,
        fromDateIso = null,
        toDateIso = null,
        validationError = validationError
    )

    private fun inclusiveDayCount(
        start: LocalDate,
        end: LocalDate
    ): Int = ChronoUnit.DAYS.between(start, end).toInt() + 1
}
