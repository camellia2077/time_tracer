package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

internal class RuntimeTracerExchangeInspectService(
    private val responseCodec: NativeResponseCodec,
    private val nativeInspectTracerExchange: (inputPath: String, passphrase: String) -> String,
    private val setProgressListener: (((String) -> Unit)?) -> Unit
) {
    suspend fun inspectTracerExchange(
        inputPath: String,
        passphrase: String,
        onProgress: ((FileCryptoProgressEvent) -> Unit)?
    ): TracerExchangeInspectResult = withContext(Dispatchers.IO) {
        val safeInput = inputPath.trim()
        val safePassphrase = passphrase

        if (safeInput.isBlank()) {
            return@withContext RuntimeTracerExchangeResults.inspectFailure(
                message = "inspect tracer exchange failed: inputPath must not be empty."
            )
        }
        if (safePassphrase.isBlank()) {
            return@withContext RuntimeTracerExchangeResults.inspectFailure(
                message = "inspect tracer exchange failed: passphrase must not be empty."
            )
        }

        runCatching {
            val rawResponse = executeWithCryptoProgressListener(
                onProgress = onProgress,
                setProgressListener = setProgressListener
            ) {
                nativeInspectTracerExchange(safeInput, safePassphrase)
            }
            val payload = responseCodec.parse(rawResponse)
            if (!payload.ok) {
                return@runCatching RuntimeTracerExchangeResults.inspectFailure(
                    message = payload.errorMessage.ifBlank {
                        "inspect tracer exchange failed."
                    }
                )
            }

            val content = RuntimeTracerExchangeResults.parseContentObject(payload.content)
            TracerExchangeInspectResult(
                ok = true,
                message = "inspect tracer exchange completed: $safeInput",
                renderedText = content.optString("rendered_text"),
                inputPath = content.optString("input_path", safeInput),
                sourceRootName = content.optString("source_root_name"),
                payloadFileCount = content.optInt("payload_file_count", 0),
                producerPlatform = content.optString("producer_platform"),
                producerApp = content.optString("producer_app"),
                createdAtUtc = content.optString("created_at_utc")
            )
        }.getOrElse { error ->
            RuntimeTracerExchangeResults.inspectFailure(
                message = formatNativeFailure(
                    "inspect tracer exchange failed",
                    error as? Exception ?: Exception(error)
                )
            )
        }
    }
}

