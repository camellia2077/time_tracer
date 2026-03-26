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
    private val nativeValidateStructure: (inputPath: String) -> String,
    private val nativeValidateLogic: (inputPath: String, dateCheckMode: Int) -> String,
    private val nativeIngest: (
        inputPath: String,
        dateCheckMode: Int,
        saveProcessedOutput: Boolean
    ) -> String,
    private val syncLiveOperation: (RuntimePaths) -> String
) {
    // Design intent – TXT month file creation:
    //
    // 1. Release builds do not bundle test TXT data. New users arrive with empty
    //    input/full/ and input/live_raw/ directories, leaving the TXT Editor tab
    //    in a "Select File" empty state. This create-month flow lets them
    //    bootstrap their first TXT without leaving the editor.
    //
    // 2. The generated file contains mandatory header lines (yYYYY, mMM).
    //    Business logic resolves a TXT's logical date from these header lines,
    //    not from the filename. Omitting them would break month identification.
    //
    // 3. Day markers (e.g. 0322) are intentionally NOT pre-generated.
    //    - The Record Input flow auto-creates day blocks on demand via
    //      LiveRawRecordPersistence.appendNewDayBlock().
    //    - Pre-generating all 28-31 day markers would produce empty noise
    //      blocks for days with no activity.
    //    - DAY editor mode is designed to edit existing blocks only; it does
    //      not need empty placeholders.

    suspend fun createCurrentMonthTxt(): RecordActionResult = withContext(Dispatchers.IO) {
        try {
            val paths = ensureRuntimePaths()
            val result = rawRecordStore.ensureCurrentMonthFile(paths.liveRawInputPath)
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
            val result = rawRecordStore.ensureMonthFile(paths.liveRawInputPath, year, monthValue)
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
                    nativeValidateStructure(targetInputPath)
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
                    nativeValidateLogic(targetInputPath, NativeBridge.DATE_CHECK_CONTINUITY)
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
                    nativeIngest(targetInputPath, NativeBridge.DATE_CHECK_CONTINUITY, false)
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
            nativeIngest(inputPath, NativeBridge.DATE_CHECK_NONE, false)
        }
    }

    private fun extractNativeStageFailure(result: NativeCallResult, stage: String): String? {
        // Android surfaces native validation failures directly in UI and should
        // not depend on any sidecar error-report files being readable locally.
        return recordTranslator.extractStageFailure(result, stage)
    }
}
