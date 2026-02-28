package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import kotlinx.coroutines.withContext
import org.json.JSONObject

internal class RuntimeCryptoService(
    private val responseCodec: NativeResponseCodec,
    private val nativeEncryptFile: (inputPath: String, outputPath: String, passphrase: String, securityLevel: FileCryptoSecurityLevel) -> String,
    private val nativeDecryptFile: (inputPath: String, outputPath: String, passphrase: String) -> String
) {
    suspend fun encryptTxtFile(
        inputPath: String,
        outputPath: String,
        passphrase: String,
        securityLevel: FileCryptoSecurityLevel = FileCryptoSecurityLevel.INTERACTIVE,
        onProgress: ((FileCryptoProgressEvent) -> Unit)? = null
    ): RecordActionResult = executeCryptoOperation(
        operationName = "encrypt txt",
        inputPath = inputPath,
        outputPath = outputPath,
        passphrase = passphrase,
        securityLevel = securityLevel,
        nativeCall = nativeEncryptFile,
        onProgress = onProgress
    )

    suspend fun decryptTracerFile(
        inputPath: String,
        outputPath: String,
        passphrase: String,
        onProgress: ((FileCryptoProgressEvent) -> Unit)? = null
    ): RecordActionResult = executeCryptoOperation(
        operationName = "decrypt tracer",
        inputPath = inputPath,
        outputPath = outputPath,
        passphrase = passphrase,
        nativeCall = { safeInput, safeOutput, safePassphrase, _ ->
            nativeDecryptFile(safeInput, safeOutput, safePassphrase)
        },
        onProgress = onProgress
    )

    private suspend fun executeCryptoOperation(
        operationName: String,
        inputPath: String,
        outputPath: String,
        passphrase: String,
        securityLevel: FileCryptoSecurityLevel = FileCryptoSecurityLevel.INTERACTIVE,
        nativeCall: (inputPath: String, outputPath: String, passphrase: String, securityLevel: FileCryptoSecurityLevel) -> String,
        onProgress: ((FileCryptoProgressEvent) -> Unit)?
    ): RecordActionResult = withContext(Dispatchers.IO) {
        val safeInput = inputPath.trim()
        val safeOutput = outputPath.trim()
        val safePassphrase = passphrase

        if (safeInput.isBlank()) {
            return@withContext RecordActionResult(
                ok = false,
                message = "$operationName failed: inputPath must not be empty."
            )
        }
        if (safeOutput.isBlank()) {
            return@withContext RecordActionResult(
                ok = false,
                message = "$operationName failed: outputPath must not be empty."
            )
        }
        if (safePassphrase.isBlank()) {
            return@withContext RecordActionResult(
                ok = false,
                message = "$operationName failed: passphrase must not be empty."
            )
        }

        runCatching {
            val rawResponse = cryptoCallMutex.withLock {
                NativeBridge.setCryptoProgressListener(
                    if (onProgress == null) {
                        null
                    } else {
                        { rawProgress ->
                            parseCryptoProgressEvent(rawProgress)?.let(onProgress)
                        }
                    }
                )
                try {
                    nativeCall(safeInput, safeOutput, safePassphrase, securityLevel)
                } finally {
                    NativeBridge.setCryptoProgressListener(null)
                }
            }
            val payload = responseCodec.parse(rawResponse)
            if (!payload.ok) {
                return@runCatching RecordActionResult(
                    ok = false,
                    message = payload.errorMessage.ifBlank {
                        "$operationName failed."
                    }
                )
            }

            val outputPathInContent = runCatching {
                JSONObject(payload.content).optString("output_path", safeOutput)
            }.getOrDefault(safeOutput)

            RecordActionResult(
                ok = true,
                message = "$operationName completed: $outputPathInContent"
            )
        }.getOrElse { error ->
            buildRecordActionFailure("$operationName failed", error as? Exception ?: Exception(error))
        }
    }

    private fun parseCryptoProgressEvent(rawProgress: String): FileCryptoProgressEvent? {
        return runCatching {
            val json = JSONObject(rawProgress)
            FileCryptoProgressEvent(
                operation = FileCryptoOperation.fromWireValue(json.optString("operation")),
                phase = FileCryptoPhase.fromWireValue(json.optString("phase")),
                currentGroupLabel = json.optString("current_group_label"),
                groupIndex = json.optInt("group_index", 0),
                groupCount = json.optInt("group_count", 0),
                fileIndexInGroup = json.optInt("file_index_in_group", 0),
                fileCountInGroup = json.optInt("file_count_in_group", 0),
                currentFileIndex = json.optInt("current_file_index", 0),
                totalFiles = json.optInt("total_files", 0),
                currentFileDoneBytes = json.optLong("current_file_done_bytes", 0L),
                currentFileTotalBytes = json.optLong("current_file_total_bytes", 0L),
                overallDoneBytes = json.optLong("overall_done_bytes", 0L),
                overallTotalBytes = json.optLong("overall_total_bytes", 0L),
                speedBytesPerSec = json.optLong("speed_bytes_per_sec", 0L),
                remainingBytes = json.optLong("remaining_bytes", 0L),
                etaSeconds = json.optLong("eta_seconds", 0L)
            )
        }.getOrNull()
    }

    companion object {
        private val cryptoCallMutex = Mutex()
    }
}
