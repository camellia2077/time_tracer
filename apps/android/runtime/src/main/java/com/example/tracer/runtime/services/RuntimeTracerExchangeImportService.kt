package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

internal class RuntimeTracerExchangeImportService(
    private val responseCodec: NativeResponseCodec,
    private val nativeImportTracerExchange: (
        inputPath: String,
        workRoot: String,
        passphrase: String
    ) -> String,
    private val setProgressListener: (((String) -> Unit)?) -> Unit
) {
    suspend fun importTracerExchange(
        inputPath: String,
        workRoot: String,
        passphrase: String,
        onProgress: ((FileCryptoProgressEvent) -> Unit)?
    ): TracerExchangeImportResult = withContext(Dispatchers.IO) {
        val safeInput = inputPath.trim()
        val safeWorkRoot = workRoot.trim()
        val safePassphrase = passphrase

        if (safeInput.isBlank()) {
            return@withContext RuntimeTracerExchangeResults.importFailure(
                message = "import tracer exchange failed: inputPath must not be empty."
            )
        }
        if (safeWorkRoot.isBlank()) {
            return@withContext RuntimeTracerExchangeResults.importFailure(
                message = "import tracer exchange failed: workRoot must not be empty."
            )
        }
        if (safePassphrase.isBlank()) {
            return@withContext RuntimeTracerExchangeResults.importFailure(
                message = "import tracer exchange failed: passphrase must not be empty."
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
            val content = RuntimeTracerExchangeResults.parseContentObject(payload.content)
            if (!payload.ok) {
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
            RuntimeTracerExchangeResults.importFailure(
                message = formatNativeFailure(
                    "import tracer exchange failed",
                    error as? Exception ?: Exception(error)
                )
            )
        }
    }
}

