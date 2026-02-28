package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File
import java.util.Locale

internal class RuntimeIngestService(
    private val ensureRuntimePaths: () -> RuntimePaths,
    private val executeAfterInit: (
        operationName: String,
        action: (RuntimePaths) -> String
    ) -> NativeCallResult,
    private val nativeIngest: (
        inputPath: String,
        dateCheckMode: Int,
        saveProcessedOutput: Boolean
    ) -> String,
    private val nativeIngestSingleTxtReplaceMonth: (
        inputPath: String,
        dateCheckMode: Int,
        saveProcessedOutput: Boolean
    ) -> String
) {
    suspend fun ingestFull(): NativeCallResult =
        runIngestFlow { paths -> paths.fullInputPath }

    suspend fun ingestSingleTxtReplaceMonth(inputPath: String): NativeCallResult =
        withContext(Dispatchers.IO) {
            try {
                val managedInputPath = prepareManagedSingleTxtInput(
                    paths = ensureRuntimePaths(),
                    sourceInputPath = inputPath
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
            }
        }

    private suspend fun runIngestFlow(
        inputPathSelector: (RuntimePaths) -> String
    ): NativeCallResult = withContext(Dispatchers.IO) {
        try {
            executeAfterInit("native_ingest") { paths ->
                nativeIngest(
                    inputPathSelector(paths),
                    NativeBridge.DATE_CHECK_CONTINUITY,
                    // Android build disables processed JSON output in core
                    // (TT_ENABLE_PROCESSED_JSON_IO=OFF).
                    false
                )
            }
        } catch (error: Exception) {
            buildNativeCallFailure(prefix = "nativeIngest failed", error = error)
        }
    }

    private data class ParsedMonthHeader(
        val year: Int,
        val month: Int
    ) {
        val monthKey: String
            get() = String.format(Locale.US, "%04d-%02d", year, month)
    }

    private fun prepareManagedSingleTxtInput(
        paths: RuntimePaths,
        sourceInputPath: String
    ): String {
        val sourceFile = File(sourceInputPath.trim())
        require(sourceFile.exists() && sourceFile.isFile) {
            "inputPath must point to an existing TXT file."
        }

        val normalizedContent = normalizeImportedText(sourceFile.readText())
        val monthHeader = parseRequiredMonthHeader(normalizedContent)
        val targetFile = File(paths.fullInputPath, "${monthHeader.monthKey}.txt")
        targetFile.parentFile?.mkdirs()
        targetFile.writeText(normalizedContent)
        return targetFile.absolutePath
    }

    private fun normalizeImportedText(rawContent: String): String {
        // Accept UTF-8 BOM imports and normalize line endings before core parse.
        return rawContent
            .removePrefix("\uFEFF")
            .replace("\r\n", "\n")
            .replace('\r', '\n')
    }

    private fun parseRequiredMonthHeader(content: String): ParsedMonthHeader {
        var yearValue: Int? = null
        var monthValue: Int? = null

        val lines = content.lines()
        for (line in lines) {
            val trimmed = line.trim()
            if (trimmed.isEmpty()) {
                continue
            }

            if (
                yearValue == null &&
                trimmed.length == 5 &&
                trimmed[0] == 'y' &&
                trimmed.substring(1).all(Char::isDigit)
            ) {
                yearValue = trimmed.substring(1).toInt()
                continue
            }

            if (
                yearValue != null &&
                monthValue == null &&
                trimmed.length == 3 &&
                trimmed[0] == 'm' &&
                trimmed.substring(1).all(Char::isDigit)
            ) {
                val parsedMonth = trimmed.substring(1).toInt()
                require(parsedMonth in 1..12) {
                    "Invalid month header: m${trimmed.substring(1)}."
                }
                monthValue = parsedMonth
                break
            }
        }

        if (yearValue == null || monthValue == null) {
            throw IllegalArgumentException(
                "Single TXT import requires headers yYYYY + mMM in file content."
            )
        }
        return ParsedMonthHeader(year = yearValue, month = monthValue)
    }
}
