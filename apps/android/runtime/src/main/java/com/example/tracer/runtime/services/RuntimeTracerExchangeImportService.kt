package com.example.tracer

import java.io.File
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

internal class RuntimeTracerExchangeImportService(
    private val responseCodec: NativeResponseCodec,
    private val nativeImportTracerExchange: (
        inputPath: String,
        workRoot: String,
        passphrase: String
    ) -> String,
) {
    suspend fun importTracerExchange(
        inputPath: String,
        workRoot: String,
        passphrase: String
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
        if (safePassphrase.isBlank() && !File(safeInput).isDirectory) {
            return@withContext RuntimeTracerExchangeResults.importFailure(
                message = "import tracer exchange failed: passphrase must not be empty."
            )
        }

        runCatching {
            val rawResponse = nativeImportTracerExchange(
                safeInput,
                safeWorkRoot,
                safePassphrase
            )
            mapImportResponse(rawResponse)
        }.getOrElse { error ->
            RuntimeTracerExchangeResults.importFailure(
                message = formatNativeFailure(
                    "import tracer exchange failed",
                    error as? Exception ?: Exception(error)
                )
            )
        }
    }

    private fun mapImportResponse(rawResponse: String): TracerExchangeImportResult {
        val payload = responseCodec.parse(rawResponse)
        val content = RuntimeTracerExchangeResults.parseContentObject(payload.content)
        val sourceRootName = content.optString("source_root_name")
        return TracerExchangeImportResult(
            ok = payload.ok,
            message = if (payload.ok) {
                "import tracer exchange completed: $sourceRootName"
            } else {
                payload.errorMessage.ifBlank { "import tracer exchange failed." }
            },
            sourceRootName = sourceRootName,
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
}
