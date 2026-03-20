package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.json.JSONObject

internal class RuntimeTracerExchangeService(
    private val responseCodec: NativeResponseCodec,
    private val nativeExportTracerExchange: (
        inputPath: String,
        outputPath: String,
        passphrase: String,
        securityLevel: FileCryptoSecurityLevel,
        dateCheckMode: Int
    ) -> String,
    private val nativeImportTracerExchange: (
        inputPath: String,
        workRoot: String,
        passphrase: String
    ) -> String,
    private val nativeInspectTracerExchange: (inputPath: String, passphrase: String) -> String,
    private val setProgressListener: (((String) -> Unit)?) -> Unit = NativeBridge::setCryptoProgressListener
) {
    suspend fun exportTracerExchange(
        inputPath: String,
        outputPath: String,
        passphrase: String,
        securityLevel: FileCryptoSecurityLevel = FileCryptoSecurityLevel.INTERACTIVE,
        dateCheckMode: Int = NativeBridge.DATE_CHECK_NONE,
        onProgress: ((FileCryptoProgressEvent) -> Unit)? = null
    ): TracerExchangeExportResult = withContext(Dispatchers.IO) {
        val safeInput = inputPath.trim()
        val safeOutput = outputPath.trim()
        val safePassphrase = passphrase

        if (safeInput.isBlank()) {
            return@withContext TracerExchangeExportResult(
                ok = false,
                message = "complete exchange package export failed: inputPath must not be empty.",
                outputPath = "",
                sourceRootName = "",
                payloadFileCount = 0,
                converterFileCount = 0,
                manifestIncluded = false
            )
        }
        if (safeOutput.isBlank()) {
            return@withContext TracerExchangeExportResult(
                ok = false,
                message = "complete exchange package export failed: outputPath must not be empty.",
                outputPath = "",
                sourceRootName = "",
                payloadFileCount = 0,
                converterFileCount = 0,
                manifestIncluded = false
            )
        }
        if (safePassphrase.isBlank()) {
            return@withContext TracerExchangeExportResult(
                ok = false,
                message = "complete exchange package export failed: passphrase must not be empty.",
                outputPath = "",
                sourceRootName = "",
                payloadFileCount = 0,
                converterFileCount = 0,
                manifestIncluded = false
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
                return@runCatching TracerExchangeExportResult(
                    ok = false,
                    message = payload.errorMessage.ifBlank {
                        "complete exchange package export failed."
                    },
                    outputPath = "",
                    sourceRootName = "",
                    payloadFileCount = 0,
                    converterFileCount = 0,
                    manifestIncluded = false
                )
            }

            val content = parseContentObject(payload.content)
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
            TracerExchangeExportResult(
                ok = false,
                message = formatNativeFailure(
                    "complete exchange package export failed",
                    error as? Exception ?: Exception(error)
                ),
                outputPath = "",
                sourceRootName = "",
                payloadFileCount = 0,
                converterFileCount = 0,
                manifestIncluded = false
            )
        }
    }

    suspend fun importTracerExchange(
        inputPath: String,
        workRoot: String,
        passphrase: String,
        onProgress: ((FileCryptoProgressEvent) -> Unit)? = null
    ): TracerExchangeImportResult = withContext(Dispatchers.IO) {
        val safeInput = inputPath.trim()
        val safeWorkRoot = workRoot.trim()
        val safePassphrase = passphrase

        if (safeInput.isBlank()) {
            return@withContext TracerExchangeImportResult(
                ok = false,
                message = "import tracer exchange failed: inputPath must not be empty.",
                sourceRootName = "",
                payloadFileCount = 0
            )
        }
        if (safeWorkRoot.isBlank()) {
            return@withContext TracerExchangeImportResult(
                ok = false,
                message = "import tracer exchange failed: workRoot must not be empty.",
                sourceRootName = "",
                payloadFileCount = 0
            )
        }
        if (safePassphrase.isBlank()) {
            return@withContext TracerExchangeImportResult(
                ok = false,
                message = "import tracer exchange failed: passphrase must not be empty.",
                sourceRootName = "",
                payloadFileCount = 0
            )
        }

        runCatching {
            val rawResponse = executeWithCryptoProgressListener(
                onProgress = onProgress,
                setProgressListener = setProgressListener
            ) {
                nativeImportTracerExchange(
                    safeInput,
                    safeWorkRoot,
                    safePassphrase
                )
            }
            val payload = responseCodec.parse(rawResponse)
            if (!payload.ok) {
                val content = parseContentObject(payload.content)
                return@runCatching TracerExchangeImportResult(
                    ok = false,
                    message = payload.errorMessage.ifBlank {
                        "import tracer exchange failed."
                    },
                    sourceRootName = content.optString("source_root_name"),
                    payloadFileCount = content.optInt("payload_file_count", 0),
                    replacedMonthCount = content.optInt("replaced_month_count", 0),
                    preservedMonthCount = content.optInt("preserved_month_count", 0),
                    rebuiltMonthCount = content.optInt("rebuilt_month_count", 0),
                    textRootUpdated = content.optBoolean("text_root_updated", false),
                    configApplied = content.optBoolean("config_applied", false),
                    databaseRebuilt = content.optBoolean("database_rebuilt", false),
                    retainedFailureRoot = content.optString("retained_failure_root"),
                    backupRetainedRoot = content.optString("backup_retained_root"),
                    backupCleanupError = content.optString("backup_cleanup_error")
                )
            }

            val content = parseContentObject(payload.content)
            TracerExchangeImportResult(
                ok = true,
                message = "import tracer exchange completed: ${content.optString("source_root_name")}",
                sourceRootName = content.optString("source_root_name"),
                payloadFileCount = content.optInt("payload_file_count", 0),
                replacedMonthCount = content.optInt("replaced_month_count", 0),
                preservedMonthCount = content.optInt("preserved_month_count", 0),
                rebuiltMonthCount = content.optInt("rebuilt_month_count", 0),
                textRootUpdated = content.optBoolean("text_root_updated", false),
                configApplied = content.optBoolean("config_applied", false),
                databaseRebuilt = content.optBoolean("database_rebuilt", false),
                retainedFailureRoot = content.optString("retained_failure_root"),
                backupRetainedRoot = content.optString("backup_retained_root"),
                backupCleanupError = content.optString("backup_cleanup_error")
            )
        }.getOrElse { error ->
            TracerExchangeImportResult(
                ok = false,
                message = formatNativeFailure(
                    "import tracer exchange failed",
                    error as? Exception ?: Exception(error)
                ),
                sourceRootName = "",
                payloadFileCount = 0
            )
        }
    }

    suspend fun inspectTracerExchange(
        inputPath: String,
        passphrase: String,
        onProgress: ((FileCryptoProgressEvent) -> Unit)? = null
    ): TracerExchangeInspectResult = withContext(Dispatchers.IO) {
        val safeInput = inputPath.trim()
        val safePassphrase = passphrase

        if (safeInput.isBlank()) {
            return@withContext TracerExchangeInspectResult(
                ok = false,
                message = "inspect tracer exchange failed: inputPath must not be empty.",
                renderedText = "",
                inputPath = "",
                sourceRootName = "",
                payloadFileCount = 0,
                producerPlatform = "",
                producerApp = "",
                createdAtUtc = ""
            )
        }
        if (safePassphrase.isBlank()) {
            return@withContext TracerExchangeInspectResult(
                ok = false,
                message = "inspect tracer exchange failed: passphrase must not be empty.",
                renderedText = "",
                inputPath = "",
                sourceRootName = "",
                payloadFileCount = 0,
                producerPlatform = "",
                producerApp = "",
                createdAtUtc = ""
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
                return@runCatching TracerExchangeInspectResult(
                    ok = false,
                    message = payload.errorMessage.ifBlank {
                        "inspect tracer exchange failed."
                    },
                    renderedText = "",
                    inputPath = "",
                    sourceRootName = "",
                    payloadFileCount = 0,
                    producerPlatform = "",
                    producerApp = "",
                    createdAtUtc = ""
                )
            }

            val content = parseContentObject(payload.content)
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
            TracerExchangeInspectResult(
                ok = false,
                message = formatNativeFailure(
                    "inspect tracer exchange failed",
                    error as? Exception ?: Exception(error)
                ),
                renderedText = "",
                inputPath = "",
                sourceRootName = "",
                payloadFileCount = 0,
                producerPlatform = "",
                producerApp = "",
                createdAtUtc = ""
            )
        }
    }

    private fun parseContentObject(content: String): JSONObject {
        if (content.isBlank()) {
            return JSONObject()
        }
        return runCatching { JSONObject(content) }.getOrDefault(JSONObject())
    }
}
