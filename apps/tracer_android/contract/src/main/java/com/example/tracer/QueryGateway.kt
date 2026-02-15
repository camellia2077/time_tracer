package com.example.tracer

interface QueryGateway {
    suspend fun queryActivitySuggestions(
        lookbackDays: Int = 7,
        topN: Int = 5
    ): ActivitySuggestionResult
    suspend fun queryDayDurations(params: DataDurationQueryParams): DataQueryTextResult
    suspend fun queryDayDurationStats(params: DataDurationQueryParams): DataQueryTextResult
    suspend fun queryProjectTree(params: DataTreeQueryParams): DataQueryTextResult
    suspend fun listActivityMappingNames(): ActivityMappingNamesResult
}
