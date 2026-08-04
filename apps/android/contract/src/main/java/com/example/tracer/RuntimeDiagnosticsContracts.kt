package com.example.tracer

data class RuntimeDiagnosticEntry(
    val timestampIso: String,
    val operationId: String,
    val stage: String,
    val ok: Boolean,
    val initialized: Boolean?,
    val message: String,
    val errorLogPath: String = ""
)

data class RuntimeDiagnosticsListResult(
    val ok: Boolean,
    val entries: List<RuntimeDiagnosticEntry>,
    val message: String,
    val diagnosticsLogPath: String = ""
)

data class RuntimeDiagnosticsPayloadResult(
    val ok: Boolean,
    val payload: String,
    val message: String,
    val entryCount: Int = 0,
    val diagnosticsLogPath: String = ""
)
