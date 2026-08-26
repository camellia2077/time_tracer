@file:Suppress("TooManyFunctions")

package com.example.tracer

interface QueryGateway {
    suspend fun queryPreviousActivityTail(targetDateIso: String): PreviousActivityTailResult =
        PreviousActivityTailResult(
            ok = false,
            found = false,
            message = "previous activity tail query is unavailable."
        )

    suspend fun queryLatestActivityRecord(targetDateIso: String): LatestActivityRecordResult =
        LatestActivityRecordResult(
            ok = false,
            found = false,
            message = "latest activity record query is unavailable."
        )

    suspend fun queryFrequentActivities(
        lookbackDays: Int = 7,
        topN: Int = 5,
        anchorDateIso: String? = null
    ): ActivityFrequentResult
    suspend fun queryDayDurations(params: DataDurationQueryParams): DataQueryTextResult
    suspend fun queryDayDurationStats(params: DataDurationQueryParams): DataQueryTextResult
    suspend fun queryProjectTree(params: DataTreeQueryParams): TreeQueryResult
    suspend fun queryInsightsCalendarAvailability(): InsightsCalendarAvailabilityResult =
        InsightsCalendarAvailabilityResult(
            ok = false,
            message = "insights calendar availability query not implemented."
        )
    suspend fun queryInsightsChart(params: InsightsChartQueryParams): InsightsChartQueryResult
    suspend fun queryInsightsComposition(
        params: InsightsCompositionQueryParams
    ): InsightsCompositionQueryResult =
        InsightsCompositionQueryResult(
            ok = false,
            data = null,
            message = "insights composition query not implemented."
        )
    suspend fun listActivityMappingNames(): ActivityMappingNamesResult
    suspend fun listActivityHierarchyLeafMappings(): ActivityHierarchyLeafMappingListResult =
        ActivityHierarchyLeafMappingListResult(
            ok = false,
            entries = emptyList(),
            message = "Activity alias mappings query not implemented."
        )
    suspend fun listCanonicalCatalog(searchQuery: String): CanonicalCatalogResult =
        CanonicalCatalogResult(
            ok = false,
            roots = emptyList(),
            entries = emptyList(),
            message = "Canonical catalog query not implemented."
        )

    // Keep this API alias-only so callers never have to infer left keys from mixed name sets.
    suspend fun listActivityHierarchyLeafKeys(): ActivityMappingNamesResult =
        listActivityMappingNames()

    // Wake semantics are config-driven. Authoring/runtime callers should not hardcode wake tokens.
    suspend fun listWakeKeywords(): ActivityMappingNamesResult =
        ActivityMappingNamesResult(
            ok = false,
            names = emptyList(),
            message = "Wake keywords query not implemented."
        )

    // Authorable event tokens include configured activity-hierarchy tokens and wake_keywords.
    suspend fun listAuthorableEventTokens(): ActivityMappingNamesResult =
        listActivityHierarchyLeafKeys()
}
