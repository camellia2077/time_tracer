package com.example.tracer

import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider

class QueryReportViewModelFactory(
    private val reportGateway: ReportGateway,
    private val queryGateway: QueryGateway,
    private val textProvider: QueryReportTextProvider = DefaultQueryReportTextProvider
) : ViewModelProvider.Factory {
    override fun <T : ViewModel> create(modelClass: Class<T>): T {
        if (modelClass.isAssignableFrom(QueryReportViewModel::class.java)) {
            @Suppress("UNCHECKED_CAST")
            return QueryReportViewModel(reportGateway, queryGateway, textProvider) as T
        }
        throw IllegalArgumentException(textProvider.unknownViewModelClass(modelClass.name))
    }
}
