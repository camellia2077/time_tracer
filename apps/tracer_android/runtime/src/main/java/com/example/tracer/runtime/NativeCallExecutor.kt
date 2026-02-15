package com.example.tracer

internal class NativeCallExecutor(
    private val initializeRuntime: () -> NativeCallResult,
    private val runtimePathsProvider: () -> RuntimePaths?,
    private val responseCodec: NativeResponseCodec,
    private val buildNativeErrorResponse: (String) -> String,
    private val formatFailure: (String, Exception) -> String
) {
    fun executeAfterInit(action: (RuntimePaths) -> String): NativeCallResult {
        val initResult = try {
            initializeRuntime()
        } catch (error: Exception) {
            val message = formatFailure("nativeInit failed", error)
            return NativeCallResult(
                initialized = false,
                operationOk = false,
                rawResponse = buildNativeErrorResponse(message)
            )
        }
        if (!initResult.initialized) {
            return initResult
        }

        val paths = runtimePathsProvider()
            ?: return NativeCallResult(
                initialized = false,
                operationOk = false,
                rawResponse = buildNativeErrorResponse("Runtime paths are not initialized.")
            )

        val response = try {
            action(paths)
        } catch (error: Exception) {
            val message = formatFailure("runtime operation failed", error)
            return NativeCallResult(
                initialized = true,
                operationOk = false,
                rawResponse = buildNativeErrorResponse(message)
            )
        }

        val payload = responseCodec.parse(response)
        return NativeCallResult(
            initialized = true,
            operationOk = payload.ok,
            rawResponse = response
        )
    }

    fun executeReportAfterInit(action: (RuntimePaths) -> String): ReportCallResult {
        val initResult = try {
            initializeRuntime()
        } catch (error: Exception) {
            val message = formatFailure("nativeInit failed", error)
            return ReportCallResult(
                initialized = false,
                operationOk = false,
                outputText = message,
                rawResponse = buildNativeErrorResponse(message)
            )
        }
        if (!initResult.initialized) {
            val initPayload = responseCodec.parse(initResult.rawResponse)
            return ReportCallResult(
                initialized = false,
                operationOk = false,
                outputText = initPayload.errorMessage,
                rawResponse = initResult.rawResponse
            )
        }

        val paths = runtimePathsProvider()
            ?: return ReportCallResult(
                initialized = false,
                operationOk = false,
                outputText = "Runtime paths are not initialized.",
                rawResponse = ""
            )

        val response = try {
            action(paths)
        } catch (error: Exception) {
            val message = formatFailure("runtime report failed", error)
            return ReportCallResult(
                initialized = true,
                operationOk = false,
                outputText = message,
                rawResponse = buildNativeErrorResponse(message)
            )
        }

        val payload = responseCodec.parse(response)
        return ReportCallResult(
            initialized = true,
            operationOk = payload.ok,
            outputText = if (payload.ok) payload.content else payload.errorMessage,
            rawResponse = response
        )
    }
}
