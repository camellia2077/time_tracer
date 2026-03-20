package com.example.tracer

import android.content.Context
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.runtime.Composable
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import kotlinx.coroutines.withContext
import java.io.File

private data class TracerExchangePackageImportResult(
    val importedCount: Int,
    val totalCount: Int,
    val errors: List<String>,
    val message: String
) {
    val ok: Boolean
        get() = totalCount > 0 && errors.isEmpty()

    val hasPartialSuccess: Boolean
        get() = false
}

@Composable
internal fun rememberTracerSingleTxtImportAction(
    context: Context,
    coroutineScope: kotlinx.coroutines.CoroutineScope,
    dataViewModel: DataViewModel,
    recordViewModel: RecordViewModel
): () -> Unit {
    val importSingleTxtLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.OpenDocument()
    ) { documentUri ->
        if (documentUri == null) {
            dataViewModel.setStatusText("single TXT import canceled.")
            return@rememberLauncherForActivityResult
        }

        coroutineScope.launch {
            val stagedInputPath = runCatching {
                withContext(Dispatchers.IO) {
                    stageSelectedTxtDocument(context = context, documentUri = documentUri)
                }
            }.getOrElse { error ->
                dataViewModel.setStatusText(
                    "single TXT import failed: ${error.message ?: "cannot read selected document."}"
                )
                return@launch
            }

            val ingestOk = dataViewModel.ingestSingleTxtReplaceMonthAndGetResult(stagedInputPath)
            // Keep Data tab export availability in sync with TXT store state,
            // even when ingest returns non-ok (e.g. partial/diagnostic response).
            recordViewModel.refreshHistory()
            if (!ingestOk) {
                return@launch
            }
        }
    }

    return {
        dataViewModel.setStatusText("selecting single TXT...")
        importSingleTxtLauncher.launch(arrayOf("text/plain", "text/*", "*/*"))
    }
}

@Composable
internal fun rememberTracerSingleTracerImportAction(
    context: Context,
    coroutineScope: kotlinx.coroutines.CoroutineScope,
    dataViewModel: DataViewModel,
    recordViewModel: RecordViewModel,
    runtimeGateway: RuntimeGateway
): () -> Unit {
    val importSingleTracerLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.OpenDocument()
    ) { documentUri ->
        if (documentUri == null) {
            recordViewModel.clearCryptoProgress()
            dataViewModel.setStatusText(context.getString(R.string.tracer_import_single_tracer_canceled))
            return@rememberLauncherForActivityResult
        }

        coroutineScope.launch {
            val stagedTracerPath = runCatching {
                withContext(Dispatchers.IO) {
                    stageSelectedDocument(
                        context = context,
                        documentUri = documentUri,
                        expectedExtension = ".tracer"
                    )
                }
            }.getOrElse { error ->
                dataViewModel.setStatusText(
                    context.getString(
                        R.string.tracer_import_single_tracer_failed,
                        error.message ?: context.getString(R.string.tracer_export_unknown_error)
                    )
                )
                return@launch
            }

            val passphrase = promptPassphrase(
                context = context,
                title = context.getString(R.string.tracer_crypto_passphrase_decrypt_title),
                firstHint = context.getString(R.string.tracer_crypto_passphrase_hint),
                secondHint = null,
                requiredMessage = context.getString(R.string.tracer_crypto_passphrase_required),
                mismatchMessage = context.getString(R.string.tracer_crypto_passphrase_mismatch)
            )
            if (passphrase.isNullOrBlank()) {
                recordViewModel.clearCryptoProgress()
                dataViewModel.setStatusText(context.getString(R.string.tracer_import_single_tracer_canceled))
                return@launch
            }

            val completedText = context.getString(R.string.tracer_progress_status_completed)
            val failedText = context.getString(R.string.tracer_progress_status_failed)
            val partialText = context.getString(R.string.tracer_progress_status_partial)
            recordViewModel.startCryptoProgress(
                context.getString(R.string.tracer_progress_operation_import_tracer)
            )
            val sourceFileName = File(stagedTracerPath).name
            val importResult = importTracerExchangeTransaction(
                context = context,
                runtimeGateway = runtimeGateway,
                inputTracerPath = stagedTracerPath,
                passphrase = passphrase,
                sourceLabel = sourceFileName,
                stagingPrefix = "single_tracer_exchange_import",
                onProgress = { event ->
                    val overallProgress = event.currentFileProgressFraction.coerceIn(0f, 1f)
                    runBlocking(Dispatchers.Main) {
                        recordViewModel.updateCryptoProgress(
                            event = event,
                            operationTextOverride = context.getString(
                                R.string.tracer_progress_operation_import_tracer
                            ),
                            overallProgressOverride = overallProgress,
                            overallTextOverride = buildSingleImportOverallText(
                                overallProgress = overallProgress,
                                isTerminal = event.phase.isTerminal
                            ),
                            currentTextOverride = buildSingleImportCurrentText(
                                sourceFileName = sourceFileName,
                                progress = overallProgress
                            )
                        )
                    }
                }
            )
            // Keep Data tab export availability in sync with TXT store state,
            // even when ingest returns non-ok (e.g. partial/diagnostic response).
            recordViewModel.refreshHistory()
            val finalStatusText = when {
                importResult.ok -> completedText
                importResult.hasPartialSuccess -> partialText
                else -> failedText
            }
            recordViewModel.finishCryptoProgress(
                statusText = finalStatusText,
                keepVisible = true,
                detailsTextOverride = importResult.message
            )
            dataViewModel.setStatusText(importResult.message)
        }
    }

    return {
        dataViewModel.setStatusText(context.getString(R.string.tracer_import_select_single_tracer))
        importSingleTracerLauncher.launch(arrayOf("*/*"))
    }
}

private suspend fun importTracerExchangeTransaction(
    context: Context,
    runtimeGateway: RuntimeGateway,
    inputTracerPath: String,
    passphrase: String,
    sourceLabel: String,
    stagingPrefix: String,
    onProgress: ((FileCryptoProgressEvent) -> Unit)? = null
): TracerExchangePackageImportResult {
    val workRoot = withContext(Dispatchers.IO) {
        buildTracerExchangeImportWorkRoot(context, stagingPrefix, sourceLabel)
    }
    val importResult = runtimeGateway.importTracerExchange(
        inputPath = inputTracerPath,
        workRoot = workRoot.absolutePath,
        passphrase = passphrase,
        onProgress = onProgress
    )
    if (!importResult.ok) {
        return TracerExchangePackageImportResult(
            importedCount = 0,
            totalCount = 0,
            errors = listOf(importResult.message),
            message = importResult.message
        )
    }

    return TracerExchangePackageImportResult(
        importedCount = importResult.payloadFileCount,
        totalCount = importResult.payloadFileCount,
        errors = emptyList(),
        message = buildSingleTracerImportSummary(
            context = context,
            successCount = importResult.payloadFileCount,
            totalCount = importResult.payloadFileCount,
            errors = emptyList()
        )
    )
}

private fun buildTracerExchangeImportWorkRoot(
    context: Context,
    stagingPrefix: String,
    sourceLabel: String
): File {
    val sanitizedLabel = sourceLabel
        .replace('\\', '_')
        .replace('/', '_')
        .replace(Regex("[^A-Za-z0-9._-]"), "_")
        .ifBlank { "tracer" }
    val root = File(context.cacheDir, "time_tracer/$stagingPrefix")
    if (!root.exists()) {
        require(root.mkdirs()) {
            "cannot create tracer exchange import staging directory: ${root.absolutePath}"
        }
    }
    return File(root, "${System.currentTimeMillis()}_$sanitizedLabel")
}
