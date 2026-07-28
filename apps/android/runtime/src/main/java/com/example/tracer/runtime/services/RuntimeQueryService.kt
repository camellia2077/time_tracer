package com.example.tracer

internal class RuntimeQueryService(
    private val queryDelegate: RuntimeQueryDelegate
) {
    suspend fun queryActivitySuggestions(
        lookbackDays: Int,
        topN: Int,
        anchorDateIso: String? = null
    ): ActivitySuggestionResult =
        queryDelegate.queryActivitySuggestions(lookbackDays, topN, anchorDateIso)

    suspend fun queryDayDurations(params: DataDurationQueryParams): DataQueryTextResult =
        queryDelegate.queryDayDurations(params)

    suspend fun queryDayDurationStats(params: DataDurationQueryParams): DataQueryTextResult =
        queryDelegate.queryDayDurationStats(params)

    suspend fun queryProjectTree(params: DataTreeQueryParams): TreeQueryResult =
        queryDelegate.queryProjectTree(params)

    suspend fun queryReportCalendarAvailability(): ReportCalendarAvailabilityResult =
        queryDelegate.queryReportCalendarAvailability()

    suspend fun queryReportChart(params: ReportChartQueryParams): ReportChartQueryResult =
        queryDelegate.queryReportChart(params)

    suspend fun queryReportComposition(
        params: ReportCompositionQueryParams
    ): ReportCompositionQueryResult =
        queryDelegate.queryReportComposition(params)

    suspend fun listActivityMappingNames(): ActivityMappingNamesResult =
        queryDelegate.listActivityMappingNames()

    suspend fun listActivityAliasMappings(): ActivityAliasMappingListResult =
        queryDelegate.listActivityAliasMappings()

    suspend fun listCanonicalCatalog(): CanonicalCatalogResult =
        queryDelegate.listCanonicalCatalog()

    suspend fun listActivityAliasKeys(): ActivityMappingNamesResult =
        queryDelegate.listActivityAliasKeys()

    suspend fun listWakeKeywords(): ActivityMappingNamesResult =
        queryDelegate.listWakeKeywords()

    suspend fun listAuthorableEventTokens(): ActivityMappingNamesResult =
        queryDelegate.listAuthorableEventTokens()
}
