package com.example.tracer

import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import java.time.Clock

class QueryInsightsViewModelFactory(
    private val insightsGateway: InsightsGateway,
    private val queryGateway: QueryGateway,
    private val recordGateway: RecordGateway? = null,
    private val textProvider: QueryInsightsTextProvider = DefaultQueryInsightsTextProvider,
    private val clock: Clock = Clock.systemDefaultZone()
) : ViewModelProvider.Factory {
    override fun <T : ViewModel> create(modelClass: Class<T>): T {
        if (modelClass.isAssignableFrom(QueryInsightsViewModel::class.java)) {
            @Suppress("UNCHECKED_CAST")
            return QueryInsightsViewModel(
                insightsGateway = insightsGateway,
                queryGateway = queryGateway,
                recordGateway = recordGateway,
                textProvider = textProvider,
                clock = clock
            ) as T
        }
        throw IllegalArgumentException(textProvider.unknownViewModelClass(modelClass.name))
    }
}
