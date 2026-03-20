package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.json.JSONObject

internal class RuntimeCryptoService(
    private val responseCodec: NativeResponseCodec,
    private val nativeEncryptFile: (inputPath: String, outputPath: String, passphrase: String, securityLevel: FileCryptoSecurityLevel) -> String,
    private val nativeDecryptFile: (inputPath: String, outputPath: String, passphrase: String) -> String,
    private val setProgressListener: (((String) -> Unit)?) -> Unit = NativeBridge::setCryptoProgressListener
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
            val rawResponse = executeWithCryptoProgressListener(
                onProgress = onProgress,
                setProgressListener = setProgressListener
            ) {
                    nativeCall(safeInput, safeOutput, safePassphrase, securityLevel)
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
}
