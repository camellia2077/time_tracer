package com.example.tracer

interface ReportGateway {
    suspend fun reportMarkdown(request: TemporalReportQueryRequest): ReportCallResult
}
