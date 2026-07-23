package com.example.tracer

interface ReportGateway {
    suspend fun reportMarkdown(request: TemporalReportQueryRequest): ReportCallResult

    suspend fun reportStructured(
        request: TemporalReportQueryRequest
    ): StructuredReportCallResult = StructuredReportCallResult(
        initialized = false,
        operationOk = false,
        report = null,
        rawResponse = ""
    )
}
