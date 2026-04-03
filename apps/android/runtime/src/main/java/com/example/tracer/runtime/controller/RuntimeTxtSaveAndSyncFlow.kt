package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File

internal class RuntimeTxtSaveAndSyncFlow(
    private val ensureRuntimePaths: () -> RuntimePaths,
    private val ensureTextStorage: () -> TextStorage,
    private val rawRecordStore: InputRecordStore,
    private val loadWakeKeywords: suspend () -> ActivityMappingNamesResult,
    private val recordTranslator: NativeRecordTranslator,
    private val executeAfterInit: (
        operationName: String,
        action: (RuntimePaths) -> String
    ) -> NativeCallResult,
    private val nativeValidateStructure: (inputPath: String) -> String,
    private val nativeValidateLogic: (inputPath: String, dateCheckMode: Int) -> String,
    private val nativeIngestSingleTxtReplaceMonth: (
        inputPath: String,
        dateCheckMode: Int,
        saveProcessedOutput: Boolean
    ) -> String
) {
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
}

