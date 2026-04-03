package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.json.JSONArray
import org.json.JSONObject

internal class RuntimeTracerExchangeExportService(
    private val responseCodec: NativeResponseCodec,
    private val nativeExportTracerExchange: (
        inputPath: String,
        outputPath: String,
        passphrase: String,
        securityLevel: FileCryptoSecurityLevel,
        dateCheckMode: Int
    ) -> String,
    private val nativeExportTracerExchangeFromPayloadJson: (
        requestJson: String,
        outputFd: Int
    ) -> String,
    private val setProgressListener: (((String) -> Unit)?) -> Unit
) {
    suspend fun exportTracerExchange(
        inputPath: String,
        outputPath: String,
        passphrase: String,
        securityLevel: FileCryptoSecurityLevel,
        dateCheckMode: Int,
        onProgress: ((FileCryptoProgressEvent) -> Unit)?
    ): TracerExchangeExportResult = withContext(Dispatchers.IO) {
        val safeInput = inputPath.trim()
        val safeOutput = outputPath.trim()
        val safePassphrase = passphrase

        if (safeInput.isBlank()) {
            return@withContext RuntimeTracerExchangeResults.exportFailure(
                message = "complete exchange package export failed: inputPath must not be empty."
            )
        }
        if (safeOutput.isBlank()) {
            return@withContext RuntimeTracerExchangeResults.exportFailure(
                message = "complete exchange package export failed: outputPath must not be empty."
            )
        }
        if (safePassphrase.isBlank()) {
            return@withContext RuntimeTracerExchangeResults.exportFailure(
                message = "complete exchange package export failed: passphrase must not be empty."
            )
        }

        runCatching {
            val rawResponse = executeWithCryptoProgressListener(
                onProgress = onProgress,
                setProgressListener = setProgressListener
            ) {
                nativeExportTracerExchange(
                    safeInput,
                    safeOutput,
                    safePassphrase,
                    securityLevel,
                    dateCheckMode
                )
            }
            val payload = responseCodec.parse(rawResponse)
            if (!payload.ok) {
                return@runCatching RuntimeTracerExchangeResults.exportFailure(
                    message = payload.errorMessage.ifBlank {
                        "complete exchange package export failed."
                    }
                )
            }

            val content = RuntimeTracerExchangeResults.parseContentObject(payload.content)
            val resolvedOutput = content.optString("output_path", safeOutput)
            TracerExchangeExportResult(
                ok = true,
                message = "complete exchange package export completed: $resolvedOutput",
                outputPath = resolvedOutput,
                sourceRootName = content.optString("source_root_name"),
                payloadFileCount = content.optInt("payload_file_count", 0),
                converterFileCount = content.optInt("converter_file_count", 0),
                manifestIncluded = content.optBoolean("manifest_included", false)
            )
        }.getOrElse { error ->
            RuntimeTracerExchangeResults.exportFailure(
                message = formatNativeFailure(
                    "complete exchange package export failed",
                    error as? Exception ?: Exception(error)
                )
            )
        }
    }

    suspend fun exportTracerExchangeFromPayload(
        payloads: List<TracerExchangePayloadItem>,
        outputFd: Int,
        passphrase: String,
        securityLevel: FileCryptoSecurityLevel,
        dateCheckMode: Int,
        logicalSourceRootName: String,
        outputDisplayName: String,
        onProgress: ((FileCryptoProgressEvent) -> Unit)?
    ): TracerExchangeExportResult = withContext(Dispatchers.IO) {
        val safePassphrase = passphrase
        val safeSourceRootName = logicalSourceRootName.trim().ifBlank { "data" }
        val safeOutputDisplayName = outputDisplayName.trim().ifBlank { "data.tracer" }

        if (payloads.isEmpty()) {
            return@withContext RuntimeTracerExchangeResults.exportFailure(
                message = "complete exchange package export failed: payloads must not be empty."
            )
        }
        if (outputFd < 0) {
            return@withContext RuntimeTracerExchangeResults.exportFailure(
                message = "complete exchange package export failed: outputFd must be a valid detached file descriptor."
            )
        }
        if (safePassphrase.isBlank()) {
            return@withContext RuntimeTracerExchangeResults.exportFailure(
                message = "complete exchange package export failed: passphrase must not be empty."
            )
        }

        val requestJson = JSONObject()
            .put("logical_source_root_name", safeSourceRootName)
            .put("output_display_name", safeOutputDisplayName)
            .put("passphrase", safePassphrase)
            .put("security_level", securityLevel.wireValue)
            .put("date_check_mode", dateCheckMode)
            .put(
                "payload_items",
                JSONArray().apply {
                    payloads.forEach { payload ->
                        put(
                            JSONObject()
                                .put("relative_path_hint", payload.relativePathHint)
                                .put("content", payload.content)
                        )
                    }
                }
            )
            .toString()

        runCatching {
            val rawResponse = executeWithCryptoProgressListener(
                onProgress = onProgress,
                setProgressListener = setProgressListener
            ) {
                nativeExportTracerExchangeFromPayloadJson(requestJson, outputFd)
            }
            val payload = responseCodec.parse(rawResponse)
            if (!payload.ok) {
                return@runCatching RuntimeTracerExchangeResults.exportFailure(
                    message = payload.errorMessage.ifBlank {
                        "complete exchange package export failed."
                    }
                )
            }

            val content = RuntimeTracerExchangeResults.parseContentObject(payload.content)
            val resolvedOutput = content.optString("output_path", safeOutputDisplayName)
            TracerExchangeExportResult(
                ok = true,
                message = "complete exchange package export completed: $resolvedOutput",
                outputPath = resolvedOutput,
                sourceRootName = content.optString("source_root_name", safeSourceRootName),
                payloadFileCount = content.optInt("payload_file_count", 0),
                converterFileCount = content.optInt("converter_file_count", 0),
                manifestIncluded = content.optBoolean("manifest_included", false)
            )
        }.getOrElse { error ->
            RuntimeTracerExchangeResults.exportFailure(
                message = formatNativeFailure(
                    "complete exchange package export failed",
                    error as? Exception ?: Exception(error)
                )
            )
        }
    }
}

