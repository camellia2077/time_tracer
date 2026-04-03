package com.example.tracer

import java.io.File

internal class RuntimeRecordSyncFlow(
    private val ensureRuntimePaths: () -> RuntimePaths,
    private val inspectTxtFilesInternal: () -> TxtInspectionResult,
    private val executeAfterInit: (
        operationName: String,
        action: (RuntimePaths) -> String
    ) -> NativeCallResult,
    private val nativeIngestSingleTxtReplaceMonth: (
        inputPath: String,
        dateCheckMode: Int,
        saveProcessedOutput: Boolean
    ) -> String,
    private val nativeClearTxtIngestSyncStatus: () -> String
) {
    fun syncLiveToDatabase(): NativeCallResult {
        val inspection = inspectTxtFilesInternal()
        if (!inspection.ok) {
            return NativeCallResult(
                initialized = false,
                operationOk = false,
                rawResponse = buildNativeErrorResponseJson(inspection.message)
            )
        }

        val blockedEntries = inspection.entries.filterNot { it.canOpen }
        if (blockedEntries.isNotEmpty()) {
            val details = blockedEntries.joinToString(separator = " | ") {
                "${it.relativePath}: ${it.message}"
            }
            return NativeCallResult(
                initialized = true,
                operationOk = false,
                rawResponse = buildNativeErrorResponseJson(
                    "syncLiveToDatabase blocked by TXT conflicts: $details"
                )
            )
        }

        val clearResult = executeAfterInit("native_clear_txt_ingest_sync_status") {
            nativeClearTxtIngestSyncStatus()
        }
        if (!clearResult.initialized || !clearResult.operationOk) {
            return clearResult
        }

        val paths = ensureRuntimePaths()
        var lastResult = clearResult
        for (entry in inspection.entries.filter { it.canOpen }.sortedBy { it.relativePath }) {
            val targetInputPath = File(paths.inputRootPath, entry.relativePath).absolutePath
            lastResult = executeAfterInit("native_ingest_single_txt_replace_month") {
                nativeIngestSingleTxtReplaceMonth(
                    targetInputPath,
                    NativeBridge.DATE_CHECK_CONTINUITY,
                    false
                )
            }
            if (!lastResult.initialized || !lastResult.operationOk) {
                return lastResult
            }
        }

        return lastResult
    }
}

