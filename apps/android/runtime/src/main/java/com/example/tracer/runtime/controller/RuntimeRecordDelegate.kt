package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File

internal class RuntimeRecordDelegate(
    private val ensureRuntimePaths: () -> RuntimePaths,
    private val ensureTextStorage: () -> TextStorage,
    private val rawRecordStore: LiveRawRecordStore,
    private val responseCodec: NativeResponseCodec,
    private val recordTranslator: NativeRecordTranslator,
    private val executeAfterInit: (
        operationName: String,
        action: (RuntimePaths) -> String
    ) -> NativeCallResult,
    private val syncLiveOperation: (RuntimePaths) -> String
) {
    suspend fun createCurrentMonthTxt(): RecordActionResult = withContext(Dispatchers.IO) {
        RecordActionResult(
            ok = false,
            message = "TXT create entry is removed. Select existing imported TXT files from full/ or live_raw/."
        )
    }

    suspend fun createMonthTxt(month: String): RecordActionResult = withContext(Dispatchers.IO) {
        RecordActionResult(
            ok = false,
            message = "TXT create entry is removed for $month. Select existing imported TXT files from full/ or live_raw/."
        )
    }

    suspend fun recordNow(
        activityName: String,
        remark: String,
        targetDateIso: String?,
        preferredTxtPath: String?
    ): RecordActionResult = withContext(Dispatchers.IO) {
        try {
            runRecordNowInternal(
                activityName = activityName,
                remark = remark,
                targetDateIso = targetDateIso,
                preferredTxtPath = preferredTxtPath
            )
        } catch (error: Exception) {
            buildRecordActionFailure(prefix = "Record failed", error = error)
        }
    }

    suspend fun syncLiveToDatabase(): NativeCallResult = withContext(Dispatchers.IO) {
        try {
            executeAfterInit("native_sync_live_to_database") { paths ->
                syncLiveOperation(paths)
            }
        } catch (error: Exception) {
            buildNativeCallFailure(prefix = "syncLiveToDatabase failed", error = error)
        }
    }

    suspend fun saveTxtFileAndSync(relativePath: String, content: String): RecordActionResult =
        withContext(Dispatchers.IO) {
            try {
                ensureRuntimePaths()
                val storage = ensureTextStorage()
                val saveResult = storage.writeTxtFile(relativePath = relativePath, content = content)
                if (!saveResult.ok) {
                    return@withContext RecordActionResult(
                        ok = false,
                        message = "save failed: ${saveResult.message}"
                    )
                }

                val sourceTarget = storage.resolveSourceTarget(relativePath)
                    ?: return@withContext RecordActionResult(
                        ok = false,
                        message = "save failed: cannot resolve txt source path."
                    )
                val targetInputPath = File(sourceTarget.first, sourceTarget.second).absolutePath

                val structureCheckResult = executeAfterInit("native_validate_structure") {
                    NativeBridge.nativeValidateStructure(
                        inputPath = targetInputPath
                    )
                }
                extractNativeStageFailure(
                    result = structureCheckResult,
                    stage = "validate"
                )?.let { failureMessage ->
                    return@withContext RecordActionResult(
                        ok = false,
                        message = "save ok, $failureMessage",
                        operationId = structureCheckResult.operationId
                    )
                }

                val logicCheckResult = executeAfterInit("native_validate_logic") {
                    NativeBridge.nativeValidateLogic(
                        inputPath = targetInputPath,
                        dateCheckMode = NativeBridge.DATE_CHECK_CONTINUITY
                    )
                }
                extractNativeStageFailure(
                    result = logicCheckResult,
                    stage = "validate"
                )?.let { failureMessage ->
                    return@withContext RecordActionResult(
                        ok = false,
                        message = "save ok, $failureMessage",
                        operationId = logicCheckResult.operationId
                    )
                }

                val syncResult = executeAfterInit("native_reimport_txt") {
                    NativeBridge.nativeIngest(
                        inputPath = targetInputPath,
                        dateCheckMode = NativeBridge.DATE_CHECK_CONTINUITY,
                        saveProcessedOutput = false
                    )
                }
                extractNativeStageFailure(
                    result = syncResult,
                    stage = "re-import"
                )?.let { failureMessage ->
                    return@withContext RecordActionResult(
                        ok = false,
                        message = "save ok, $failureMessage",
                        operationId = syncResult.operationId
                    )
                }

                RecordActionResult(
                    ok = true,
                    message = "save+re-import -> ${saveResult.filePath}",
                    operationId = syncResult.operationId
                )
            } catch (error: Exception) {
                buildRecordActionFailure(prefix = "save txt failed", error = error)
            }
        }

    private fun runRecordNowInternal(
        activityName: String,
        remark: String,
        targetDateIso: String?,
        preferredTxtPath: String?
    ): RecordActionResult {
        val paths = ensureRuntimePaths()
        val storage = ensureTextStorage()
        val logicalDateResult = parseLogicalDate(targetDateIso)
        if (!logicalDateResult.ok || logicalDateResult.date == null) {
            return RecordActionResult(
                ok = false,
                message = logicalDateResult.message
            )
        }
        val logicalDate = logicalDateResult.date

        val normalizedActivity = rawRecordStore.normalizeActivityName(activityName)
        if (normalizedActivity.isEmpty()) {
            return RecordActionResult(
                ok = false,
                message = "Activity name is empty."
            )
        }
        val normalizedRemark = rawRecordStore.normalizeRemark(remark)
        val target = resolveRecordTarget(
            paths = paths,
            storage = storage,
            logicalDate = logicalDate,
            preferredTxtPath = preferredTxtPath
        )
        val createdMonthFile = !target.targetFile.exists()

        val snapshot = try {
            rawRecordStore.appendRecord(
                liveRawInputPath = target.inputPath,
                logicalDateString = logicalDate,
                activityName = normalizedActivity,
                remark = normalizedRemark,
                preferredRelativePath = target.preferredInnerPath
            )
        } catch (error: Exception) {
            return buildRecordActionFailure(
                prefix = "Record failed: cannot write TXT",
                error = error
            )
        }
        val recordSummary = buildRecordSummary(
            snapshot = snapshot,
            monthFileCreated = createdMonthFile
        )

        val syncResult = syncRecordInput(target.inputPath)
        val failure = buildRecordSyncFailureResult(
            recordSummary = recordSummary,
            syncResult = syncResult,
            responseCodec = responseCodec
        )
        if (failure != null) {
            return failure
        }

        return RecordActionResult(
            ok = true,
            message = "$recordSummary\nsync: ok",
            operationId = syncResult.operationId
        )
    }

    private fun syncRecordInput(inputPath: String): NativeCallResult {
        return executeAfterInit("native_record_sync_ingest") {
            NativeBridge.nativeIngest(
                inputPath = inputPath,
                dateCheckMode = NativeBridge.DATE_CHECK_NONE,
                saveProcessedOutput = false
            )
        }
    }

    private fun extractNativeStageFailure(result: NativeCallResult, stage: String): String? {
        // Android surfaces native validation failures directly in UI and should
        // not depend on any sidecar error-report files being readable locally.
        return recordTranslator.extractStageFailure(result, stage)
    }
}
