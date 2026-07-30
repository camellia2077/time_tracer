package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

internal class RuntimeInitService(
    private val initializeRuntimeInternal: () -> NativeCallResult,
    private val clearAllData: () -> String,
    private val clearDatabaseData: () -> ClearDatabaseResult,
    private val resetRuntimeCaches: () -> Unit,
    private val executeAfterInit: (
        operationName: String,
        action: (RuntimePaths) -> String
    ) -> NativeCallResult = { _, _ ->
        buildNativeCallFailure(
            prefix = "rebuild database failed",
            error = IllegalStateException("executeAfterInit is unavailable.")
        )
    },
    private val nativeIngest: (
        inputPath: String,
        dateCheckMode: Int,
        saveProcessedOutput: Boolean
    ) -> String = { _, _, _ ->
        throw IllegalStateException("nativeIngest is unavailable.")
    }
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
            val clearMessage = clearAllData()
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

    suspend fun clearDatabase(): ClearDatabaseResult = withContext(Dispatchers.IO) {
        try {
            val clearResult = clearDatabaseData()
            resetRuntimeCaches()
            clearResult
        } catch (error: Exception) {
            resetRuntimeCaches()
            ClearDatabaseResult(
                ok = false,
                message = formatNativeFailure("clear database failed", error)
            )
        }
    }

    suspend fun rebuildDatabase(): NativeCallResult = withContext(Dispatchers.IO) {
        try {
            val clearResult = clearDatabaseData()
            resetRuntimeCaches()
            if (!clearResult.ok) {
                return@withContext NativeCallResult(
                    initialized = false,
                    operationOk = false,
                    rawResponse = buildNativeErrorResponseJson(clearResult.message)
                )
            }
            executeAfterInit("native_ingest_rebuild_database") { paths ->
                nativeIngest(
                    paths.inputRootPath,
                    NativeBridge.DATE_CHECK_CONTINUITY,
                    false
                )
            }
        } catch (error: Exception) {
            resetRuntimeCaches()
            buildNativeCallFailure(
                prefix = "rebuild database failed",
                error = error
            )
        }
    }
}
