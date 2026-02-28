package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

internal class RuntimeInitService(
    private val initializeRuntimeInternal: () -> NativeCallResult,
    private val clearRuntimeData: () -> String,
    private val resetRuntimeCaches: () -> Unit
) {
    suspend fun initializeRuntime(): NativeCallResult = withContext(Dispatchers.IO) {
        try {
            initializeRuntimeInternal()
        } catch (error: Exception) {
            buildNativeCallFailure(prefix = "nativeInit failed", error = error)
        }
    }

    suspend fun clearAndReinitialize(): ClearAndInitResult = withContext(Dispatchers.IO) {
        try {
            val clearMessage = clearRuntimeData()
            resetRuntimeCaches()
            val initResult = initializeRuntimeInternal()
            ClearAndInitResult(
                initialized = initResult.initialized,
                operationOk = initResult.operationOk,
                clearMessage = clearMessage,
                initResponse = initResult.rawResponse,
                operationId = initResult.operationId
            )
        } catch (error: Exception) {
            ClearAndInitResult(
                initialized = false,
                operationOk = false,
                clearMessage = "clear -> failed",
                initResponse = buildNativeErrorResponseJson(
                    formatNativeFailure("clear and reinitialize failed", error)
                )
            )
        }
    }
}
