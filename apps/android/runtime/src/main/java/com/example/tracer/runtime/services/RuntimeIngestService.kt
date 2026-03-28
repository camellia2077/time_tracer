package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File

internal class RuntimeIngestService(
    private val ensureRuntimePaths: () -> RuntimePaths,
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
    suspend fun ingestSingleTxtReplaceMonth(inputPath: String): NativeCallResult =
        withContext(Dispatchers.IO) {
            var validationCandidateFile: File? = null
            try {
                val paths = ensureRuntimePaths()
                val preparedInput = prepareSingleTxtCandidate(
                    sourceInputPath = inputPath
                )
                validationCandidateFile = writeValidationCandidate(
                    paths = paths,
                    sourceInputPath = inputPath,
                    normalizedContent = preparedInput.normalizedContent
                )

                val structureCheckResult = executeAfterInit("native_validate_structure") {
                    nativeValidateStructure(validationCandidateFile.absolutePath)
                }
                if (!structureCheckResult.initialized || !structureCheckResult.operationOk) {
                    return@withContext structureCheckResult
                }

                val logicCheckResult = executeAfterInit("native_validate_logic") {
                    nativeValidateLogic(
                        validationCandidateFile.absolutePath,
                        NativeBridge.DATE_CHECK_CONTINUITY
                    )
                }
                if (!logicCheckResult.initialized || !logicCheckResult.operationOk) {
                    return@withContext logicCheckResult
                }

                val managedInputPath = promoteValidatedSingleTxtInput(
                    paths = paths,
                    preparedInput = preparedInput
                )
                executeAfterInit("native_ingest_single_txt_replace_month") {
                    nativeIngestSingleTxtReplaceMonth(
                        managedInputPath,
                        NativeBridge.DATE_CHECK_CONTINUITY,
                        // Android build disables processed JSON output in core
                        // (TT_ENABLE_PROCESSED_JSON_IO=OFF).
                        false
                    )
                }
            } catch (error: Exception) {
                buildNativeCallFailure(
                    prefix = "nativeIngest(single_txt_replace_month) failed",
                    error = error
                )
            } finally {
                validationCandidateFile?.delete()
            }
        }

    private data class PreparedSingleTxtInput(
        val monthHeader: TxtMonthHeader,
        val normalizedContent: String
    )

    private fun prepareSingleTxtCandidate(
        sourceInputPath: String
    ): PreparedSingleTxtInput {
        val sourceFile = File(sourceInputPath.trim())
        require(sourceFile.exists() && sourceFile.isFile) {
            "inputPath must point to an existing TXT file."
        }

        val normalizedContent = CanonicalTextCodec.readFile(sourceFile)
        val monthHeader = parseTxtMonthHeader(normalizedContent)
            ?: throw IllegalArgumentException(
                "TXT import requires headers yYYYY + mMM in file content."
            )
        return PreparedSingleTxtInput(
            monthHeader = monthHeader,
            normalizedContent = normalizedContent
        )
    }

    private fun writeValidationCandidate(
        paths: RuntimePaths,
        sourceInputPath: String,
        normalizedContent: String
    ): File {
        val candidateRoot = File(paths.cacheRootPath).apply { mkdirs() }
        val validationCandidateFile = File.createTempFile(
            "single_txt_validate_",
            ".txt",
            candidateRoot
        )
        CanonicalTextCodec.writeFile(validationCandidateFile, normalizedContent)
        return validationCandidateFile
    }

    private fun promoteValidatedSingleTxtInput(
        paths: RuntimePaths,
        preparedInput: PreparedSingleTxtInput
    ): String {
        val monthHeader = preparedInput.monthHeader
        val targetFile = File(paths.inputRootPath, monthHeader.canonicalRelativePath)
        targetFile.parentFile?.mkdirs()
        CanonicalTextCodec.writeFile(targetFile, preparedInput.normalizedContent)
        return targetFile.absolutePath
    }
}
