@file:Suppress("TooManyFunctions")

package com.example.tracer

internal class RuntimeQueryService(
    private val queryDelegate: RuntimeQueryDelegate
) {
    suspend fun queryPreviousActivityTail(targetDateIso: String): PreviousActivityTailResult =
        queryDelegate.queryPreviousActivityTail(targetDateIso)

    suspend fun queryLatestActivityRecord(targetDateIso: String): LatestActivityRecordResult =
        queryDelegate.queryLatestActivityRecord(targetDateIso)

    suspend fun queryFrequentActivities(
        lookbackDays: Int,
        topN: Int,
        anchorDateIso: String? = null
    ): ActivityFrequentResult =
        queryDelegate.queryFrequentActivities(lookbackDays, topN, anchorDateIso)

    suspend fun queryDayDurations(params: DataDurationQueryParams): DataQueryTextResult =
        queryDelegate.queryDayDurations(params)

    suspend fun queryDayDurationStats(params: DataDurationQueryParams): DataQueryTextResult =
        queryDelegate.queryDayDurationStats(params)

    suspend fun queryProjectTree(params: DataTreeQueryParams): TreeQueryResult =
        queryDelegate.queryProjectTree(params)

    suspend fun queryInsightsCalendarAvailability(): InsightsCalendarAvailabilityResult =
        queryDelegate.queryInsightsCalendarAvailability()

    suspend fun queryInsightsChart(params: InsightsChartQueryParams): InsightsChartQueryResult =
        queryDelegate.queryInsightsChart(params)

    suspend fun queryInsightsComposition(
        params: InsightsCompositionQueryParams
    ): InsightsCompositionQueryResult =
        queryDelegate.queryInsightsComposition(params)

    suspend fun listActivityMappingNames(): ActivityMappingNamesResult =
        queryDelegate.listActivityMappingNames()

    suspend fun listActivityHierarchyLeafMappings(): ActivityHierarchyLeafMappingListResult =
        queryDelegate.listActivityHierarchyLeafMappings()

    suspend fun listCanonicalCatalog(): CanonicalCatalogResult =
        queryDelegate.listCanonicalCatalog()

    suspend fun listActivityHierarchyLeafKeys(): ActivityMappingNamesResult =
        queryDelegate.listActivityHierarchyLeafKeys()

    suspend fun listWakeKeywords(): ActivityMappingNamesResult =
        queryDelegate.listWakeKeywords()

    suspend fun listAuthorableEventTokens(): ActivityMappingNamesResult =
        queryDelegate.listAuthorableEventTokens()
}
