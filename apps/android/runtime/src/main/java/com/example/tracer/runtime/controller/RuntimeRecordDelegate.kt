package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File

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
            runRecordNowInternal(
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
            runSyncLiveToDatabaseInternal()
        } catch (error: Exception) {
            buildNativeCallFailure(prefix = "syncLiveToDatabase failed", error = error)
        }
    }

    suspend fun saveTxtFileAndSync(relativePath: String, content: String): RecordActionResult =
        withContext(Dispatchers.IO) {
            var validationCandidateFile: File? = null
            try {
                val paths = ensureRuntimePaths()
                val storage = ensureTextStorage()
                val canonicalRelativePath = resolveCanonicalTxtRelativePath(paths.inputRootPath, relativePath)
                    ?: return@withContext RecordActionResult(
                        ok = false,
                        message = "save blocked: cannot resolve txt path inside input root."
                    )

                val canonicalContent = CanonicalTextCodec.canonicalizeText(content)
                val header = parseTxtMonthHeader(canonicalContent)
                    ?: return@withContext RecordActionResult(
                        ok = false,
                        message = "save blocked: TXT is missing valid yYYYY + mMM headers."
                    )
                if (header.canonicalRelativePath != canonicalRelativePath) {
                    return@withContext RecordActionResult(
                        ok = false,
                        message = "save blocked: TXT header month resolves to ${header.canonicalRelativePath}, but target path is $canonicalRelativePath."
                    )
                }

                validationCandidateFile = File.createTempFile(
                    "txt_edit_validate_",
                    ".txt",
                    File(paths.cacheRootPath).apply { mkdirs() }
                )
                CanonicalTextCodec.writeFile(validationCandidateFile, canonicalContent)

                val structureCheckResult = executeAfterInit("native_validate_structure") {
                    nativeValidateStructure(validationCandidateFile.absolutePath)
                }
                extractNativeStageFailure(
                    result = structureCheckResult,
                    stage = "validate"
                )?.let { failureMessage ->
                    return@withContext RecordActionResult(
                        ok = false,
                        message = "save blocked, $failureMessage",
                        operationId = structureCheckResult.operationId
                    )
                }

                val logicCheckResult = executeAfterInit("native_validate_logic") {
                    nativeValidateLogic(validationCandidateFile.absolutePath, NativeBridge.DATE_CHECK_CONTINUITY)
                }
                extractNativeStageFailure(
                    result = logicCheckResult,
                    stage = "validate"
                )?.let { failureMessage ->
                    return@withContext RecordActionResult(
                        ok = false,
                        message = "save blocked, $failureMessage",
                        operationId = logicCheckResult.operationId
                    )
                }

                val saveResult = storage.writeTxtFile(
                    relativePath = canonicalRelativePath,
                    content = canonicalContent
                )
                if (!saveResult.ok) {
                    return@withContext RecordActionResult(
                        ok = false,
                        message = "save failed: ${saveResult.message}"
                    )
                }

                val targetInputPath = File(paths.inputRootPath, canonicalRelativePath).absolutePath
                val syncResult = executeAfterInit("native_reimport_txt") {
                    nativeIngestSingleTxtReplaceMonth(
                        targetInputPath,
                        NativeBridge.DATE_CHECK_CONTINUITY,
                        false
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
                    message = buildSaveSuccessMessage(
                        baseMessage = "save+re-import -> ${saveResult.filePath}",
                        canonicalContent = canonicalContent
                    ),
                    operationId = syncResult.operationId
                )
            } catch (error: Exception) {
                buildRecordActionFailure(prefix = "save txt failed", error = error)
            } finally {
                validationCandidateFile?.delete()
            }
        }

    private fun runRecordNowInternal(
        activityName: String,
        remark: String,
        targetDateIso: String?,
        preferredTxtPath: String?,
        timeOrderMode: RecordTimeOrderMode
    ): RecordActionResult {
        val logicalDateResult = parseLogicalDate(targetDateIso)
        if (!logicalDateResult.ok || logicalDateResult.date == null) {
            return RecordActionResult(
                ok = false,
                message = logicalDateResult.message
            )
        }
        val logicalDate = logicalDateResult.date

        // Record Activity authority now lives in libs so TXT candidate validation, canonical
        // alias resolution, ingest, and rollback all share one atomic decision path. Android must
        // not rebuild the old "write TXT first, then sync DB" flow locally.
        val atomicResult = executeAfterInit("native_record_activity_atomically") {
            nativeRecordActivityAtomically(
                logicalDate,
                activityName,
                remark,
                preferredTxtPath,
                NativeBridge.DATE_CHECK_NONE,
                timeOrderMode
            )
        }
        val payload = responseCodec.parse(atomicResult.rawResponse)
        val atomicPayload = atomicRecordCodec.parse(payload.content)
        val baseMessage = atomicPayload?.message
            ?.takeIf { it.isNotBlank() }
            ?: payload.errorMessage.takeIf { it.isNotBlank() }
            ?: if (atomicResult.operationOk) "record: ok\nsync: ok" else "Record failed."

        if (!atomicResult.initialized) {
            return RecordActionResult(
                ok = false,
                message = appendFailureContext(
                    message = baseMessage,
                    operationId = atomicResult.operationId,
                    errorLogPath = atomicResult.errorLogPath
                ),
                operationId = atomicResult.operationId
            )
        }

        if (!atomicResult.operationOk) {
            return RecordActionResult(
                ok = false,
                message = appendFailureContext(
                    message = baseMessage,
                    operationId = atomicResult.operationId,
                    errorLogPath = atomicResult.errorLogPath
                ),
                operationId = atomicResult.operationId
            )
        }

        return RecordActionResult(
            ok = true,
            message = baseMessage,
            operationId = atomicResult.operationId
        )
    }

    private fun runSyncLiveToDatabaseInternal(): NativeCallResult {
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

    private fun syncRecordInput(inputPath: String): NativeCallResult {
        return executeAfterInit("native_record_sync_ingest") {
            nativeIngestSingleTxtReplaceMonth(inputPath, NativeBridge.DATE_CHECK_NONE, false)
        }
    }

    private fun validateRecordTarget(
        paths: RuntimePaths,
        logicalMonth: String,
        preferredTxtPath: String?,
        inspection: TxtInspectionResult
    ): ValidatedRecordTarget {
        val inspectionByPath = inspection.entries.associateBy { it.relativePath.replace('\\', '/') }
        if (!preferredTxtPath.isNullOrBlank()) {
            val preferredRelativePath = resolveCanonicalTxtRelativePath(paths.inputRootPath, preferredTxtPath)
                ?: return ValidatedRecordTarget(
                    preferredRelativePath = null,
                    errorMessage = "Selected TXT path is outside input root. Refresh TXT list and try again."
                )
            val entry = inspectionByPath[preferredRelativePath]
                ?: return ValidatedRecordTarget(
                    preferredRelativePath = null,
                    errorMessage = "Selected TXT file no longer exists. Refresh TXT list and try again."
                )
            if (!entry.canOpen) {
                return ValidatedRecordTarget(
                    preferredRelativePath = null,
                    errorMessage = "Selected TXT cannot be opened: ${entry.message} Refresh TXT list and try again."
                )
            }
            if (entry.headerMonth != logicalMonth) {
                return ValidatedRecordTarget(
                    preferredRelativePath = null,
                    errorMessage = "Selected TXT month ${entry.headerMonth ?: "unknown"} does not match logical date month $logicalMonth. Refresh TXT list and try again."
                )
            }
            return ValidatedRecordTarget(
                preferredRelativePath = preferredRelativePath,
                errorMessage = null
            )
        }

        val defaultRelativePath = buildMonthRelativePath(logicalMonth)
        val monthEntries = inspection.entries.filter { it.headerMonth == logicalMonth }
        val blockingMonthEntry = monthEntries.firstOrNull { !it.canOpen }
        if (blockingMonthEntry != null) {
            return ValidatedRecordTarget(
                preferredRelativePath = null,
                errorMessage = "Target month $logicalMonth is blocked: ${blockingMonthEntry.message}"
            )
        }

        val entry = inspectionByPath[defaultRelativePath]
        if (entry != null) {
            if (!entry.canOpen) {
                return ValidatedRecordTarget(
                    preferredRelativePath = null,
                    errorMessage = "Target TXT cannot be opened: ${entry.message}"
                )
            }
            if (entry.headerMonth != logicalMonth) {
                return ValidatedRecordTarget(
                    preferredRelativePath = null,
                    errorMessage = "Target TXT month ${entry.headerMonth ?: "unknown"} does not match logical date month $logicalMonth."
                )
            }
        }

        return ValidatedRecordTarget(
            preferredRelativePath = null,
            errorMessage = null
        )
    }

    private fun extractNativeStageFailure(result: NativeCallResult, stage: String): String? {
        return recordTranslator.extractStageFailure(result, stage)
    }

    private suspend fun buildSaveSuccessMessage(baseMessage: String, canonicalContent: String): String {
        val wakeKeywordsResult = try {
            loadWakeKeywords()
        } catch (_: Exception) {
            null
        }
        val normalizedWakeKeywords = if (wakeKeywordsResult?.ok == true) {
            wakeKeywordsResult.names.toSet()
        } else {
            emptySet()
        }
        val warning = rawRecordStore.resolveCompletenessWarningForMonthContent(
            content = canonicalContent,
            wakeKeywords = normalizedWakeKeywords
        )
        return if (warning.isNullOrBlank()) {
            baseMessage
        } else {
            "$baseMessage\n$warning"
        }
    }

    private data class ValidatedRecordTarget(
        val preferredRelativePath: String?,
        val errorMessage: String?
    )
}
