package com.example.tracer

internal class RuntimeCoreAdapter(
    private val ensureRuntimePaths: () -> RuntimePaths,
    private val runtimePathsProvider: () -> RuntimePaths?,
    private val nativeInit: (RuntimePaths) -> String,
    private val nativeQuery: (DataQueryRequest) -> String,
    private val nativeTree: (DataTreeQueryParams) -> String,
    private val responseCodec: NativeResponseCodec,
    private val reportTranslator: NativeReportTranslator,
    private val diagnosticsRecorder: RuntimeDiagnosticsRecorder,
    private val nextOperationId: (String) -> String,
    private val errorMapper: RuntimeErrorMapper
) {
    private val callExecutor = NativeCallExecutor(
        initializeRuntime = { initializeRuntimeInternal() },
        runtimePathsProvider = runtimePathsProvider,
        responseCodec = responseCodec,
        reportTranslator = reportTranslator,
        diagnosticsRecorder = diagnosticsRecorder,
        nextOperationId = nextOperationId,
        formatFailure = errorMapper::formatFailure
    )

    fun initializeRuntimeInternal(): NativeCallResult {
        val operationId = nextOperationId("native_init")
        try {
            val paths = ensureRuntimePaths()
            val response = nativeInit(paths)
            val payload = responseCodec.parse(response)
            val errorLogPath = runCatching {
                org.json.JSONObject(payload.content).optString("error_log_path", "")
            }.getOrDefault("")
            val result = NativeCallResult(
                initialized = payload.ok,
                operationOk = payload.ok,
                rawResponse = response,
                errorLogPath = errorLogPath,
                operationId = operationId
            )
            val message = if (payload.ok) {
                "ok"
            } else {
                errorMapper.appendContext(
                    message = payload.errorMessage.ifEmpty { "native init failed." },
                    operationId = operationId,
                    errorLogPath = errorLogPath
                )
            }
            diagnosticsRecorder.record(
                RuntimeDiagnosticRecord(
                    timestampEpochMs = System.currentTimeMillis(),
                    operationId = operationId,
                    stage = "native_init",
                    ok = payload.ok,
                    initialized = payload.ok,
                    message = message,
                    errorLogPath = errorLogPath
                )
            )
            return result
        } catch (error: Exception) {
            val failureMessage = errorMapper.formatFailure("nativeInit failed", error)
            diagnosticsRecorder.record(
                RuntimeDiagnosticRecord(
                    timestampEpochMs = System.currentTimeMillis(),
                    operationId = operationId,
                    stage = "native_init",
                    ok = false,
                    initialized = false,
                    message = errorMapper.appendContext(
                        failureMessage,
                        operationId = operationId
                    ),
                    errorLogPath = ""
                )
            )
            throw error
        }
    }

    fun executeAfterInit(
        operationName: String,
        action: (RuntimePaths) -> String
    ): NativeCallResult {
        return callExecutor.executeAfterInit(
            operationName = operationName,
            action = action
        )
    }

    fun executeReportAfterInit(
        operationName: String,
        action: (RuntimePaths) -> String
    ): ReportCallResult {
        return callExecutor.executeReportAfterInit(
            operationName = operationName,
            action = action
        )
    }

    fun executeNativeDataQuery(
        request: DataQueryRequest,
        onRuntimePaths: ((RuntimePaths) -> Unit)? = null
    ): NativeCallResult {
        val operationName = "native_query_${request.action}"
        return executeAfterInit(operationName = operationName) { paths ->
            onRuntimePaths?.invoke(paths)
            nativeQuery(request)
        }
    }

    fun executeNativeTreeQuery(
        params: DataTreeQueryParams
    ): NativeCallResult {
        return executeAfterInit(operationName = "native_tree") {
            nativeTree(params)
        }
    }
}
