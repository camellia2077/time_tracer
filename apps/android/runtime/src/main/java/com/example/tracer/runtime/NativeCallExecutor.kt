package com.example.tracer

import android.util.Log

private const val INSIGHTS_LOG_TAG = "TimeTracerInsights"

// Insights failures are logged with the native error code/message so a logcat
// capture can distinguish request construction problems from JNI startup or
// runtime initialization failures.

internal class NativeCallExecutor(
    private val initializeRuntime: () -> NativeCallResult,
    private val runtimePathsProvider: () -> RuntimePaths?,
    private val responseCodec: NativeResponseCodec,
    private val insightsTranslator: NativeInsightsTranslator,
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

    fun executeInsightsAfterInit(
        operationName: String = "runtime_insights",
        action: (RuntimePaths) -> String
    ): InsightsCallResult {
        val operationId = nextOperationId(operationName)
        val initResult = try {
            initializeRuntime()
        } catch (error: Exception) {
            val message = formatFailure("nativeInit failed", error)
            Log.e(INSIGHTS_LOG_TAG, "insights init threw operation=$operationName id=$operationId message=$message", error)
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
            return insightsTranslator.fromInitFailure(
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
            Log.e(
                INSIGHTS_LOG_TAG,
                "insights init failed operation=$operationName id=$operationId " +
                    "message=$initMessage errorLogPath=${initResult.errorLogPath}"
            )
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
            return insightsTranslator.fromInitFailure(initResult.copy(operationId = operationId))
        }

        val paths = runtimePathsProvider()
            ?: return insightsTranslator.fromRuntimePathsMissing(operationId)
                .also {
                    Log.e(
                        INSIGHTS_LOG_TAG,
                        "insights runtime paths missing operation=$operationName id=$operationId"
                    )
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
            val message = formatFailure("runtime insights failed", error)
            Log.e(
                INSIGHTS_LOG_TAG,
                "insights native call threw operation=$operationName id=$operationId message=$message",
                error
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
            return insightsTranslator.fromExecutionFailure(
                message = message,
                rawResponse = buildNativeErrorResponseJson(
                    errorMessage = message,
                    operationId = operationId
                ),
                operationId = operationId
            )
        }

        val result = insightsTranslator.fromNativeResponse(
            response = response,
            operationId = operationId
        )
        Log.i(
            INSIGHTS_LOG_TAG,
            "insights result operation=$operationName id=$operationId " +
                "initialized=${result.initialized} ok=${result.operationOk} " +
                "errorCode=${result.errorContract?.errorCode.orEmpty()} " +
                "errorCategory=${result.errorContract?.errorCategory.orEmpty()} " +
                "errorLogPath=${result.errorLogPath} " +
                "window=${result.insightsWindowMetadata?.toLogSummary().orEmpty()} " +
                "outputPreview=${result.outputText.logPreview()}"
        )
        val diagnosticMessage = if (result.operationOk) {
            "ok"
        } else {
            appendFailureContext(
                message = result.outputText.ifBlank { "runtime insights failed." },
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

    private fun InsightsWindowMetadata.toLogSummary(): String =
        "${startDate}..${endDate}, requestedDays=$requestedDays, " +
            "hasRecords=$hasRecords, matchedDays=$matchedDayCount, " +
            "matchedRecords=$matchedRecordCount"

    private fun String.logPreview(maxLength: Int = 500): String =
        replace(Regex("\\s+"), " ").take(maxLength)

    private fun extractErrorLogPath(content: String): String {
        if (content.isBlank()) {
            return ""
        }
        return runCatching {
            org.json.JSONObject(content).optString("error_log_path", "")
        }.getOrDefault("")
    }
}
