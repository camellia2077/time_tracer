package com.example.tracer

import java.time.LocalDate
import java.time.temporal.ChronoUnit

internal data class ResolvedChartQueryParams(
    val reportMode: ReportMode,
    val lookbackDays: Int,
    val fromDateIso: String?,
    val toDateIso: String?,
    val validationError: String
)

internal class QueryReportChartParamResolver(
    private val inputValidator: QueryInputValidator,
    private val textProvider: QueryReportTextProvider
) {
    private val selectionResolver = ReportTemporalSelectionResolver(
        inputValidator = inputValidator,
        textProvider = textProvider
    )

    fun resolve(currentState: QueryReportUiState): ResolvedChartQueryParams {
        return when (val result = selectionResolver.resolve(currentState)) {
            is ReportTemporalSelectionResolveResult.Failure -> invalid(
                reportMode = result.reportMode,
                validationError = result.validationError
            )
            is ReportTemporalSelectionResolveResult.Success -> fromSelection(result.selection)
        }
    }

    private fun fromSelection(selection: ReportTemporalSelection): ResolvedChartQueryParams {
        return when (selection) {
            is ReportTemporalSelection.SingleDay -> ResolvedChartQueryParams(
                reportMode = selection.reportMode,
                lookbackDays = 1,
                fromDateIso = selection.date.toString(),
                toDateIso = selection.date.toString(),
                validationError = ""
            )
            is ReportTemporalSelection.DateRange -> resolvedRange(
                reportMode = selection.reportMode,
                start = selection.start,
                end = selection.end
            )
            is ReportTemporalSelection.RecentDays -> ResolvedChartQueryParams(
                reportMode = selection.reportMode,
                lookbackDays = selection.days,
                fromDateIso = null,
                toDateIso = null,
                validationError = ""
            )
        }
    }

    private fun resolvedRange(
        reportMode: ReportMode,
        start: LocalDate,
        end: LocalDate
    ): ResolvedChartQueryParams = ResolvedChartQueryParams(
        reportMode = reportMode,
        lookbackDays = inclusiveDayCount(start, end),
        fromDateIso = start.toString(),
        toDateIso = end.toString(),
        validationError = ""
    )

    private fun invalid(
        reportMode: ReportMode,
        validationError: String
    ): ResolvedChartQueryParams = ResolvedChartQueryParams(
        reportMode = reportMode,
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
