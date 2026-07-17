package com.example.tracer

interface QueryGateway {
    suspend fun queryActivitySuggestions(
        lookbackDays: Int = 7,
        topN: Int = 5,
        anchorDateIso: String? = null
    ): ActivitySuggestionResult
    suspend fun queryDayDurations(params: DataDurationQueryParams): DataQueryTextResult
    suspend fun queryDayDurationStats(params: DataDurationQueryParams): DataQueryTextResult
    suspend fun queryProjectTree(params: DataTreeQueryParams): TreeQueryResult
    suspend fun queryReportChart(params: ReportChartQueryParams): ReportChartQueryResult
    suspend fun queryReportComposition(
        params: ReportCompositionQueryParams
    ): ReportCompositionQueryResult =
        ReportCompositionQueryResult(
            ok = false,
            data = null,
            message = "report composition query not implemented."
        )
    suspend fun listActivityMappingNames(): ActivityMappingNamesResult
    suspend fun listActivityAliasMappings(): ActivityAliasMappingListResult =
        ActivityAliasMappingListResult(
            ok = false,
            entries = emptyList(),
            message = "Activity alias mappings query not implemented."
        )
    suspend fun listCanonicalCatalog(): CanonicalCatalogResult =
        CanonicalCatalogResult(
            ok = false,
            roots = emptyList(),
            entries = emptyList(),
            message = "Canonical catalog query not implemented."
        )

    // Keep this API alias-only so callers never have to infer left keys from mixed name sets.
    suspend fun listActivityAliasKeys(): ActivityMappingNamesResult =
        listActivityMappingNames()

    // Wake semantics are config-driven. Authoring/runtime callers should not hardcode wake tokens.
    suspend fun listWakeKeywords(): ActivityMappingNamesResult =
        ActivityMappingNamesResult(
            ok = false,
            names = emptyList(),
            message = "Wake keywords query not implemented."
        )

    // Authorable event tokens are alias_mapping keys union wake_keywords.
    suspend fun listAuthorableEventTokens(): ActivityMappingNamesResult =
        listActivityAliasKeys()
}
