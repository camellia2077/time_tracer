package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.time.Instant
import java.time.ZoneOffset
import java.time.format.DateTimeFormatter

internal class RuntimeDiagnosticsService(
    private val ensureRuntimePaths: () -> RuntimePaths,
    private val bundleStatusProvider: () -> RuntimeConfigBundleStatus,
    private val diagnosticsRecorder: RuntimeDiagnosticsRecorder,
    private val nextOperationId: (String) -> String,
    private val errorMapper: RuntimeErrorMapper
) {
    suspend fun listRecentDiagnostics(limit: Int): RuntimeDiagnosticsListResult =
        withContext(Dispatchers.IO) {
            if (limit <= 0) {
                return@withContext RuntimeDiagnosticsListResult(
                    ok = false,
                    entries = emptyList(),
                    message = "diagnostics limit must be greater than 0.",
                    diagnosticsLogPath = diagnosticsRecorder.diagnosticsLogPath()
                )
            }
            val records = diagnosticsRecorder.recent(limit)
            RuntimeDiagnosticsListResult(
                ok = true,
                entries = records.map { it.toContractEntry() },
                message = "Loaded ${records.size} diagnostic record(s).",
                diagnosticsLogPath = diagnosticsRecorder.diagnosticsLogPath()
            )
        }

    suspend fun buildDiagnosticsPayload(maxEntries: Int): RuntimeDiagnosticsPayloadResult =
        withContext(Dispatchers.IO) {
            if (maxEntries <= 0) {
                return@withContext RuntimeDiagnosticsPayloadResult(
                    ok = false,
                    payload = "",
                    message = "maxEntries must be greater than 0.",
                    entryCount = 0,
                    diagnosticsLogPath = diagnosticsRecorder.diagnosticsLogPath()
                )
            }

            runCatching {
                ensureRuntimePaths()
            }.onFailure { error ->
                val asException = if (error is Exception) {
                    error
                } else {
                    IllegalStateException(error.message ?: "unknown failure", error)
                }
                val operationId = nextOperationId("diagnostics_prepare_paths")
                diagnosticsRecorder.record(
                    RuntimeDiagnosticRecord(
                        timestampEpochMs = System.currentTimeMillis(),
                        operationId = operationId,
                        stage = "diagnostics.prepare_runtime_paths",
                        ok = false,
                        initialized = null,
                        message = errorMapper.appendContext(
                            message = errorMapper.formatFailure(
                                "prepare runtime paths failed",
                                asException
                            ),
                            operationId = operationId
                        ),
                        errorLogPath = ""
                    )
                )
            }

            val records = diagnosticsRecorder.recent(maxEntries)
            val bundleStatus = bundleStatusProvider()
            val payload = buildDiagnosticsPayloadText(
                records = records,
                bundleStatus = bundleStatus
            )
            RuntimeDiagnosticsPayloadResult(
                ok = true,
                payload = payload,
                message = "Prepared diagnostics payload (${records.size} entries).",
                entryCount = records.size,
                diagnosticsLogPath = diagnosticsRecorder.diagnosticsLogPath()
            )
        }

    private fun buildDiagnosticsPayloadText(
        records: List<RuntimeDiagnosticRecord>,
        bundleStatus: RuntimeConfigBundleStatus
    ): String {
        val generatedAtIso = DateTimeFormatter.ISO_INSTANT
            .withZone(ZoneOffset.UTC)
            .format(Instant.now())
        val bundleSchema = bundleStatus.schemaVersion?.toString() ?: "unknown"
        val bundleProfile = bundleStatus.profile.ifBlank { "unknown" }
        val bundleName = bundleStatus.bundleName.ifBlank { "unknown" }
        val bundleMissing = if (bundleStatus.missingFiles.isEmpty()) {
            "-"
        } else {
            bundleStatus.missingFiles.joinToString(",")
        }
        val logPath = diagnosticsRecorder.diagnosticsLogPath().ifBlank { "-" }

        return buildString {
            appendLine("time_tracer_android_support_payload_v1")
            appendLine("generated_at_utc=$generatedAtIso")
            appendLine("bundle.ok=${bundleStatus.ok}")
            appendLine("bundle.schema_version=$bundleSchema")
            appendLine("bundle.profile=$bundleProfile")
            appendLine("bundle.name=$bundleName")
            appendLine("bundle.required_count=${bundleStatus.requiredFiles.size}")
            appendLine("bundle.missing=$bundleMissing")
            appendLine("bundle.message=${bundleStatus.message}")
            appendLine("diagnostics.log_path=$logPath")
            appendLine("diagnostics.count=${records.size}")
            appendLine("[diagnostics]")
            for (record in records) {
                val timestampIso = DateTimeFormatter.ISO_INSTANT
                    .withZone(ZoneOffset.UTC)
                    .format(Instant.ofEpochMilli(record.timestampEpochMs))
                val initFlag = record.initialized?.toString() ?: "null"
                val logSuffix = if (record.errorLogPath.isBlank()) {
                    ""
                } else {
                    " log=${record.errorLogPath}"
                }
                appendLine(
                    "$timestampIso op=${record.operationId} stage=${record.stage} ok=${record.ok} " +
                        "initialized=$initFlag$logSuffix msg=${record.message}"
                )
            }
        }
    }
}
