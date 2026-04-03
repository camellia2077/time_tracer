package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

internal class RuntimeRecordDelegate(
    private val ensureRuntimePaths: () -> RuntimePaths,
    private val ensureTextStorage: () -> TextStorage,
    private val rawRecordStore: InputRecordStore,
    private val loadWakeKeywords: suspend () -> ActivityMappingNamesResult,
    private val responseCodec: NativeResponseCodec,
    private val atomicRecordCodec: NativeAtomicRecordCodec,
    private val recordTranslator: NativeRecordTranslator,
    private val inspectTxtFilesInternal: () -> TxtInspectionResult,
    private val executeAfterInit: (
        operationName: String,
        action: (RuntimePaths) -> String
    ) -> NativeCallResult,
    private val nativeValidateStructure: (inputPath: String) -> String,
    private val nativeValidateLogic: (inputPath: String, dateCheckMode: Int) -> String,
    private val nativeRecordActivityAtomically: (
        targetDateIso: String,
        rawActivityName: String,
        remark: String,
        preferredTxtPath: String?,
        dateCheckMode: Int,
        timeOrderMode: RecordTimeOrderMode
    ) -> String,
    private val nativeIngestSingleTxtReplaceMonth: (
        inputPath: String,
        dateCheckMode: Int,
        saveProcessedOutput: Boolean
    ) -> String,
    private val nativeClearTxtIngestSyncStatus: () -> String
) {
    private val atomicFlow = RuntimeRecordAtomicFlow(
        responseCodec = responseCodec,
        atomicRecordCodec = atomicRecordCodec,
        executeAfterInit = executeAfterInit,
        nativeRecordActivityAtomically = nativeRecordActivityAtomically
    )
    private val syncFlow = RuntimeRecordSyncFlow(
        ensureRuntimePaths = ensureRuntimePaths,
        inspectTxtFilesInternal = inspectTxtFilesInternal,
        executeAfterInit = executeAfterInit,
        nativeIngestSingleTxtReplaceMonth = nativeIngestSingleTxtReplaceMonth,
        nativeClearTxtIngestSyncStatus = nativeClearTxtIngestSyncStatus
    )
    private val txtSaveAndSyncFlow = RuntimeTxtSaveAndSyncFlow(
        ensureRuntimePaths = ensureRuntimePaths,
        ensureTextStorage = ensureTextStorage,
        rawRecordStore = rawRecordStore,
        loadWakeKeywords = loadWakeKeywords,
        recordTranslator = recordTranslator,
        executeAfterInit = executeAfterInit,
        nativeValidateStructure = nativeValidateStructure,
        nativeValidateLogic = nativeValidateLogic,
        nativeIngestSingleTxtReplaceMonth = nativeIngestSingleTxtReplaceMonth
    )

    suspend fun createCurrentMonthTxt(): RecordActionResult = withContext(Dispatchers.IO) {
        try {
            val paths = ensureRuntimePaths()
            val result = rawRecordStore.ensureCurrentMonthFile(paths.inputRootPath)
            RecordActionResult(
                ok = true,
                message = "Created month TXT -> ${result.monthFile.name}" +
                    if (result.created) " (new file)" else " (already exists)"
            )
        } catch (error: Exception) {
            buildRecordActionFailure(prefix = "Create current month TXT failed", error = error)
        }
    }

    suspend fun createMonthTxt(month: String): RecordActionResult = withContext(Dispatchers.IO) {
        try {
            val parsed = month.trim().split("-")
            if (parsed.size != 2) {
                return@withContext RecordActionResult(
                    ok = false,
                    message = "Invalid month format: $month. Expected YYYY-MM."
                )
            }
            val year = parsed[0].toIntOrNull()
            val monthValue = parsed[1].toIntOrNull()
            if (year == null || monthValue == null || monthValue !in 1..12) {
                return@withContext RecordActionResult(
                    ok = false,
                    message = "Invalid month format: $month. Expected YYYY-MM with month 01-12."
                )
            }
            val paths = ensureRuntimePaths()
            val result = rawRecordStore.ensureMonthFile(paths.inputRootPath, year, monthValue)
            RecordActionResult(
                ok = true,
                message = "Created month TXT -> ${result.monthFile.name}" +
                    if (result.created) " (new file)" else " (already exists)"
            )
        } catch (error: Exception) {
            buildRecordActionFailure(prefix = "Create month TXT failed for $month", error = error)
        }
    }

    suspend fun recordNow(
        activityName: String,
        remark: String,
        targetDateIso: String?,
        preferredTxtPath: String?,
        timeOrderMode: RecordTimeOrderMode
    ): RecordActionResult = withContext(Dispatchers.IO) {
        try {
            atomicFlow.recordNow(
                activityName = activityName,
                remark = remark,
                targetDateIso = targetDateIso,
                preferredTxtPath = preferredTxtPath,
                timeOrderMode = timeOrderMode
            )
        } catch (error: Exception) {
            buildRecordActionFailure(prefix = "Record failed", error = error)
        }
    }

    suspend fun syncLiveToDatabase(): NativeCallResult = withContext(Dispatchers.IO) {
        try {
            syncFlow.syncLiveToDatabase()
        } catch (error: Exception) {
            buildNativeCallFailure(prefix = "syncLiveToDatabase failed", error = error)
        }
    }

    suspend fun saveTxtFileAndSync(relativePath: String, content: String): RecordActionResult =
        txtSaveAndSyncFlow.saveTxtFileAndSync(
            relativePath = relativePath,
            content = content
        )
}

