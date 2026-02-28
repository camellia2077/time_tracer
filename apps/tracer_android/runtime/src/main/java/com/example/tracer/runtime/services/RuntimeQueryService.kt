package com.example.tracer

internal class RuntimeQueryService(
    private val queryDelegate: RuntimeQueryDelegate
) {
    suspend fun queryActivitySuggestions(
        lookbackDays: Int,
        topN: Int
    ): ActivitySuggestionResult =
        queryDelegate.queryActivitySuggestions(lookbackDays, topN)

    suspend fun queryDayDurations(params: DataDurationQueryParams): DataQueryTextResult =
        queryDelegate.queryDayDurations(params)

    suspend fun queryDayDurationStats(params: DataDurationQueryParams): DataQueryTextResult =
        queryDelegate.queryDayDurationStats(params)

    suspend fun queryProjectTree(params: DataTreeQueryParams): TreeQueryResult =
        queryDelegate.queryProjectTree(params)

    suspend fun queryProjectTreeText(params: DataTreeQueryParams): DataQueryTextResult =
        queryDelegate.queryProjectTreeText(params)

    suspend fun queryReportChart(params: ReportChartQueryParams): ReportChartQueryResult =
        queryDelegate.queryReportChart(params)

    suspend fun listActivityMappingNames(): ActivityMappingNamesResult =
        queryDelegate.listActivityMappingNames()
}
