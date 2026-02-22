package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File

internal class RuntimeRecordDelegate(
    private val ensureRuntimePaths: () -> RuntimePaths,
    private val ensureTextStorage: () -> TextStorage,
    private val rawRecordStore: LiveRawRecordStore,
    private val responseCodec: NativeResponseCodec,
    private val executeAfterInit: ((RuntimePaths) -> String) -> NativeCallResult,
    private val syncLiveOperation: (RuntimePaths) -> String
) {
    suspend fun createCurrentMonthTxt(): RecordActionResult = withContext(Dispatchers.IO) {
        RecordActionResult(
            ok = false,
            message = "TXT create entry is removed. Select existing imported TXT files from full/ or smoke/."
        )
    }

    suspend fun createMonthTxt(month: String): RecordActionResult = withContext(Dispatchers.IO) {
        RecordActionResult(
            ok = false,
            message = "TXT create entry is removed for $month. Select existing imported TXT files from full/ or smoke/."
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
            executeAfterInit { paths ->
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

                val structureCheckResult = executeAfterInit {
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
                        message = "save ok, $failureMessage"
                    )
                }

                val logicCheckResult = executeAfterInit {
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
                        message = "save ok, $failureMessage"
                    )
                }

                val syncResult = executeAfterInit {
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
                        message = "save ok, $failureMessage"
                    )
                }

                RecordActionResult(
                    ok = true,
                    message = "save+re-import -> ${saveResult.filePath}"
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
            message = "$recordSummary\nsync: ok"
        )
    }

    private fun syncRecordInput(inputPath: String): NativeCallResult {
        return executeAfterInit {
            NativeBridge.nativeIngest(
                inputPath = inputPath,
                dateCheckMode = NativeBridge.DATE_CHECK_NONE,
                saveProcessedOutput = false
            )
        }
    }

    private fun extractNativeStageFailure(result: NativeCallResult, stage: String): String? {
        if (!result.initialized) {
            return "$stage failed: native init failed."
        }
        if (result.operationOk) {
            return null
        }

        val payload = responseCodec.parse(result.rawResponse)
        val error = if (payload.errorMessage.isNotEmpty()) {
            payload.errorMessage
        } else {
            result.rawResponse
        }
        val detailedError = enrichFailureWithFullReport(error)
        return "$stage failed: $detailedError"
    }

    private fun enrichFailureWithFullReport(errorMessage: String): String {
        val reportPath = parseFullErrorReportPath(errorMessage) ?: return errorMessage
        val reportFile = File(reportPath)
        if (!reportFile.exists() || !reportFile.isFile) {
            return "$errorMessage\nFull report read failed: file not found at $reportPath"
        }

        val reportText = try {
            reportFile.readText()
        } catch (error: Exception) {
            return "$errorMessage\nFull report read failed: ${error.message ?: "unknown read error"}"
        }

        val numberedLines = reportText
            .lineSequence()
            .mapIndexed { index, line -> "${index + 1}: $line" }
            .toList()
        val lineCount = numberedLines.size
        val fullBody = if (numberedLines.isEmpty()) {
            "(empty report)"
        } else {
            numberedLines.joinToString(separator = "\n")
        }

        return buildString {
            append(errorMessage)
            append("\n\n--- Full Validate Report ($lineCount lines) ---\n")
            append(fullBody)
        }
    }

    private fun parseFullErrorReportPath(errorMessage: String): String? {
        val marker = "Full error report:"
        val pathLine = errorMessage
            .lineSequence()
            .firstOrNull { it.contains(marker) }
            ?: return null
        return pathLine
            .substringAfter(marker, "")
            .trim()
            .takeIf { it.isNotEmpty() }
    }
}
