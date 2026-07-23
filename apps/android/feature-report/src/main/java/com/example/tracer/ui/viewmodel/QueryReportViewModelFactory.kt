package com.example.tracer

import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import java.time.Clock

class QueryReportViewModelFactory(
    private val reportGateway: ReportGateway,
    private val queryGateway: QueryGateway,
    private val recordGateway: RecordGateway? = null,
    private val textProvider: QueryReportTextProvider = DefaultQueryReportTextProvider,
    private val clock: Clock = Clock.systemDefaultZone()
) : ViewModelProvider.Factory {
    override fun <T : ViewModel> create(modelClass: Class<T>): T {
        if (modelClass.isAssignableFrom(QueryReportViewModel::class.java)) {
            @Suppress("UNCHECKED_CAST")
            return QueryReportViewModel(
                reportGateway = reportGateway,
                queryGateway = queryGateway,
                recordGateway = recordGateway,
                textProvider = textProvider,
                clock = clock
            ) as T
        }
        throw IllegalArgumentException(textProvider.unknownViewModelClass(modelClass.name))
    }
}
