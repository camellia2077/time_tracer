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
        securityLevel: TracerExchangeSecurityLevel,
        dateCheckMode: Int
    ) -> String,
    private val nativeExportTracerExchangeFromPayloadJson: (
        requestJson: String,
        outputFd: Int
    ) -> String,
    private val nativeBuildTracerExchangeContentFromPayloadJson: (
        requestJson: String
    ) -> String,
    private val setProgressListener: (((String) -> Unit)?) -> Unit
) {
    suspend fun exportTracerExchange(
        inputPath: String,
        outputPath: String,
        passphrase: String,
        securityLevel: TracerExchangeSecurityLevel,
        dateCheckMode: Int,
        onProgress: ((TracerExchangeProgressEvent) -> Unit)?
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
            mapExportResponse(
                rawResponse = rawResponse,
                fallbackOutputPath = safeOutput,
                fallbackSourceRootName = ""
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
        securityLevel: TracerExchangeSecurityLevel,
        dateCheckMode: Int,
        logicalSourceRootName: String,
        outputDisplayName: String,
        onProgress: ((TracerExchangeProgressEvent) -> Unit)?
    ): TracerExchangeExportResult = withContext(Dispatchers.IO) {
        val safePassphrase = passphrase
        val safeSourceRootName = logicalSourceRootName.trim().ifBlank { "data" }
        val safeOutputDisplayName = outputDisplayName.trim().ifBlank { "data.zip" }

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
            .put("payload_items", buildPayloadItemsJson(payloads))
            .toString()

        runCatching {
            val rawResponse = executeWithCryptoProgressListener(
                onProgress = onProgress,
                setProgressListener = setProgressListener
            ) {
                nativeExportTracerExchangeFromPayloadJson(requestJson, outputFd)
            }
            mapExportResponse(
                rawResponse = rawResponse,
                fallbackOutputPath = safeOutputDisplayName,
                fallbackSourceRootName = safeSourceRootName
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

    suspend fun buildTracerExchangeContentFromPayload(
        payloads: List<TracerExchangePayloadItem>,
        logicalSourceRootName: String,
        dateCheckMode: Int
    ): TracerExchangeContentResult = withContext(Dispatchers.IO) {
        val sourceRootName = logicalSourceRootName.trim().ifBlank { "data" }
        if (payloads.isEmpty()) {
            return@withContext TracerExchangeContentResult(
                ok = false,
                message = "build exchange content failed: payloads must not be empty.",
                manifestText = "",
                entries = emptyList()
            )
        }
        val requestJson = JSONObject()
            .put("logical_source_root_name", sourceRootName)
            .put("date_check_mode", dateCheckMode)
            .put("payload_items", buildPayloadItemsJson(payloads))
            .toString()

        runCatching {
            val payload = responseCodec.parse(
                nativeBuildTracerExchangeContentFromPayloadJson(requestJson)
            )
            val content = RuntimeTracerExchangeResults.parseContentObject(payload.content)
            if (!payload.ok) {
                return@runCatching TracerExchangeContentResult(
                    ok = false,
                    message = payload.errorMessage.ifBlank {
                        "build exchange content failed."
                    },
                    manifestText = "",
                    entries = emptyList()
                )
            }
            val entriesJson = content.optJSONArray("entries") ?: JSONArray()
            val entries = (0 until entriesJson.length()).mapNotNull { index ->
                entriesJson.optJSONObject(index)?.let { entry ->
                    TracerExchangeContentEntry(
                        relativePath = entry.optString("relative_path"),
                        content = entry.optString("content"),
                        contentBase64 = entry.optString("content_base64")
                    )
                }
            }
            TracerExchangeContentResult(
                ok = true,
                message = "",
                manifestText = content.optString("manifest_text"),
                entries = entries
            )
        }.getOrElse { error ->
            TracerExchangeContentResult(
                ok = false,
                message = formatNativeFailure(
                    "build exchange content failed",
                    error as? Exception ?: Exception(error)
                ),
                manifestText = "",
                entries = emptyList()
            )
        }
    }

    private fun buildPayloadItemsJson(
        payloads: List<TracerExchangePayloadItem>
    ): JSONArray = JSONArray().apply {
        payloads.forEach { payload ->
            put(
                JSONObject()
                    .put("relative_path_hint", payload.relativePathHint)
                    .put("content", payload.content)
            )
        }
    }

    private fun mapExportResponse(
        rawResponse: String,
        fallbackOutputPath: String,
        fallbackSourceRootName: String
    ): TracerExchangeExportResult {
        val payload = responseCodec.parse(rawResponse)
        if (!payload.ok) {
            return RuntimeTracerExchangeResults.exportFailure(
                message = payload.errorMessage.ifBlank {
                    "complete exchange package export failed."
                }
            )
        }

        val content = RuntimeTracerExchangeResults.parseContentObject(payload.content)
        val resolvedOutput = content.optString("output_path", fallbackOutputPath)
        return TracerExchangeExportResult(
            ok = true,
            message = "complete exchange package export completed: $resolvedOutput",
            outputPath = resolvedOutput,
            sourceRootName = content.optString(
                "source_root_name",
                fallbackSourceRootName
            ),
            payloadFileCount = content.optInt("payload_file_count", 0),
            converterFileCount = content.optInt("converter_file_count", 0),
            manifestIncluded = content.optBoolean("manifest_included", false)
        )
    }

}
