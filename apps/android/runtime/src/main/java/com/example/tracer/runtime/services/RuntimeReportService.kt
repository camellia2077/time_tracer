package com.example.tracer

internal class RuntimeReportService(
    private val reportDelegate: RuntimeReportDelegate
) {
    suspend fun reportMarkdown(request: TemporalReportQueryRequest): ReportCallResult =
        reportDelegate.reportMarkdown(request)
}
