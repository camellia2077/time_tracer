package com.example.tracer

internal class NativeCallExecutor(
    private val initializeRuntime: () -> NativeCallResult,
    private val runtimePathsProvider: () -> RuntimePaths?,
    private val responseCodec: NativeResponseCodec,
    private val reportTranslator: NativeReportTranslator,
    private val diagnosticsRecorder: RuntimeDiagnosticsRecorder,
    private val nextOperationId: (String) -> String,
    private val formatFailure: (String, Exception) -> String
) {
    fun executeAfterInit(
        operationName: String = "runtime_operation",
        action: (RuntimePaths) -> String
    ): NativeCallResult {
        val operationId = nextOperationId(operationName)
        val initResult = try {
            initializeRuntime()
        } catch (error: Exception) {
            val message = formatFailure("nativeInit failed", error)
            val response = buildNativeErrorResponseJson(
                errorMessage = message,
                operationId = operationId
            )
            val failure = NativeCallResult(
                initialized = false,
                operationOk = false,
                rawResponse = response,
                operationId = operationId
            )
            diagnosticsRecorder.record(
                RuntimeDiagnosticRecord(
                    timestampEpochMs = System.currentTimeMillis(),
                    operationId = operationId,
                    stage = "$operationName.init",
                    ok = false,
                    initialized = false,
                    message = appendFailureContext(message, operationId = operationId),
                    errorLogPath = ""
                )
            )
            return failure
        }
        if (!initResult.initialized) {
            val initMessage = responseCodec.parse(initResult.rawResponse)
                .errorMessage
                .ifEmpty { "native init failed." }
            diagnosticsRecorder.record(
                RuntimeDiagnosticRecord(
                    timestampEpochMs = System.currentTimeMillis(),
                    operationId = operationId,
                    stage = "$operationName.init",
                    ok = false,
                    initialized = false,
                    message = appendFailureContext(
                        message = initMessage,
                        operationId = operationId,
                        errorLogPath = initResult.errorLogPath
                    ),
                    errorLogPath = initResult.errorLogPath
                )
            )
            return initResult.copy(operationId = operationId)
        }

        val paths = runtimePathsProvider()
            ?: return NativeCallResult(
                initialized = false,
                operationOk = false,
                rawResponse = buildNativeErrorResponseJson(
                    errorMessage = "Runtime paths are not initialized.",
                    operationId = operationId
                ),
                operationId = operationId
            ).also { result ->
                diagnosticsRecorder.record(
                    RuntimeDiagnosticRecord(
                        timestampEpochMs = System.currentTimeMillis(),
                        operationId = operationId,
                        stage = operationName,
                        ok = false,
                        initialized = false,
                        message = appendFailureContext(
                            message = "Runtime paths are not initialized.",
                            operationId = operationId
                        ),
                        errorLogPath = result.errorLogPath
                    )
                )
            }

        val response = try {
            action(paths)
        } catch (error: Exception) {
            val message = formatFailure("runtime operation failed", error)
            val failure = NativeCallResult(
                initialized = true,
                operationOk = false,
                rawResponse = buildNativeErrorResponseJson(
                    errorMessage = message,
                    operationId = operationId
                ),
                operationId = operationId
            )
            diagnosticsRecorder.record(
                RuntimeDiagnosticRecord(
                    timestampEpochMs = System.currentTimeMillis(),
                    operationId = operationId,
                    stage = operationName,
                    ok = false,
                    initialized = true,
                    message = appendFailureContext(message, operationId = operationId),
                    errorLogPath = ""
                )
            )
            return failure
        }

        val payload = responseCodec.parse(response)
        val errorLogPath = if (payload.ok) {
            ""
        } else {
            extractErrorLogPath(payload.content)
        }
        val result = NativeCallResult(
            initialized = true,
            operationOk = payload.ok,
            rawResponse = response,
            errorLogPath = errorLogPath,
            operationId = operationId
        )
        val diagnosticMessage = if (payload.ok) {
            "ok"
        } else {
            appendFailureContext(
                message = payload.errorMessage.ifEmpty { "runtime operation failed." },
                operationId = operationId,
                errorLogPath = errorLogPath
            )
        }
        diagnosticsRecorder.record(
            RuntimeDiagnosticRecord(
                timestampEpochMs = System.currentTimeMillis(),
                operationId = operationId,
                stage = operationName,
                ok = payload.ok,
                initialized = true,
                message = diagnosticMessage,
                errorLogPath = errorLogPath
            )
        )
        return result
    }

    fun executeReportAfterInit(
        operationName: String = "runtime_report",
        action: (RuntimePaths) -> String
    ): ReportCallResult {
        val operationId = nextOperationId(operationName)
        val initResult = try {
            initializeRuntime()
        } catch (error: Exception) {
            val message = formatFailure("nativeInit failed", error)
            diagnosticsRecorder.record(
                RuntimeDiagnosticRecord(
                    timestampEpochMs = System.currentTimeMillis(),
                    operationId = operationId,
                    stage = "$operationName.init",
                    ok = false,
                    initialized = false,
                    message = appendFailureContext(message, operationId = operationId),
                    errorLogPath = ""
                )
            )
            return reportTranslator.fromInitFailure(
                NativeCallResult(
                    initialized = false,
                    operationOk = false,
                    rawResponse = buildNativeErrorResponseJson(
                        errorMessage = message,
                        operationId = operationId
                    ),
                    operationId = operationId
                )
            )
        }
        if (!initResult.initialized) {
            val initMessage = responseCodec.parse(initResult.rawResponse)
                .errorMessage
                .ifEmpty { "native init failed." }
            diagnosticsRecorder.record(
                RuntimeDiagnosticRecord(
                    timestampEpochMs = System.currentTimeMillis(),
                    operationId = operationId,
                    stage = "$operationName.init",
                    ok = false,
                    initialized = false,
                    message = appendFailureContext(
                        message = initMessage,
                        operationId = operationId,
                        errorLogPath = initResult.errorLogPath
                    ),
                    errorLogPath = initResult.errorLogPath
                )
            )
            return reportTranslator.fromInitFailure(initResult.copy(operationId = operationId))
        }

        val paths = runtimePathsProvider()
            ?: return reportTranslator.fromRuntimePathsMissing(operationId)
                .also {
                    diagnosticsRecorder.record(
                        RuntimeDiagnosticRecord(
                            timestampEpochMs = System.currentTimeMillis(),
                            operationId = operationId,
                            stage = operationName,
                            ok = false,
                            initialized = false,
                            message = appendFailureContext(
                                message = "Runtime paths are not initialized.",
                                operationId = operationId
                            ),
                            errorLogPath = ""
                        )
                    )
                }

        val response = try {
            action(paths)
        } catch (error: Exception) {
            val message = formatFailure("runtime report failed", error)
            diagnosticsRecorder.record(
                RuntimeDiagnosticRecord(
                    timestampEpochMs = System.currentTimeMillis(),
                    operationId = operationId,
                    stage = operationName,
                    ok = false,
                    initialized = true,
                    message = appendFailureContext(message, operationId = operationId),
                    errorLogPath = ""
                )
            )
            return reportTranslator.fromExecutionFailure(
                message = message,
                rawResponse = buildNativeErrorResponseJson(
                    errorMessage = message,
                    operationId = operationId
                ),
                operationId = operationId
            )
        }

        val result = reportTranslator.fromNativeResponse(
            response = response,
            operationId = operationId
        )
        val diagnosticMessage = if (result.operationOk) {
            "ok"
        } else {
            appendFailureContext(
                message = result.outputText.ifBlank { "runtime report failed." },
                operationId = operationId,
                errorLogPath = result.errorLogPath
            )
        }
        diagnosticsRecorder.record(
            RuntimeDiagnosticRecord(
                timestampEpochMs = System.currentTimeMillis(),
                operationId = operationId,
                stage = operationName,
                ok = result.operationOk,
                initialized = result.initialized,
                message = diagnosticMessage,
                errorLogPath = result.errorLogPath
            )
        )
        return result
    }

    private fun extractErrorLogPath(content: String): String {
        if (content.isBlank()) {
            return ""
        }
        return runCatching {
            org.json.JSONObject(content).optString("error_log_path", "")
        }.getOrDefault("")
    }
}
