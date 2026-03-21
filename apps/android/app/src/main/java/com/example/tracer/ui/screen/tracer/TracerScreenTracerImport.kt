package com.example.tracer

import android.content.Context
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import kotlinx.coroutines.Dispatchers
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
internal fun rememberTracerSingleTracerImportAction(
    context: Context,
    coroutineScope: kotlinx.coroutines.CoroutineScope,
    dataViewModel: DataViewModel,
    recordViewModel: RecordViewModel,
    tracerExchangeGateway: TracerExchangeGateway
): () -> Unit {
    val transferCoordinator = rememberTracerScreenTransferCoordinator(
        context = context,
        coroutineScope = coroutineScope
    )
    val transferUiCallbacks = remember(dataViewModel, recordViewModel) {
        TracerTransferUiCallbacks(
            setStatusText = dataViewModel::setStatusText,
            clearCryptoProgress = recordViewModel::clearCryptoProgress,
            startCryptoProgress = recordViewModel::startCryptoProgress,
            finishCryptoProgress = { statusText, detailsText ->
                recordViewModel.finishCryptoProgress(
                    statusText = statusText,
                    keepVisible = true,
                    detailsTextOverride = detailsText
                )
            }
        )
    }
    val canceledStatusText = context.getString(R.string.tracer_import_single_tracer_canceled)
    val progressOperationText = context.getString(R.string.tracer_progress_operation_import_tracer)
    val completedText = context.getString(R.string.tracer_progress_status_completed)
    val failedText = context.getString(R.string.tracer_progress_status_failed)
    val partialText = context.getString(R.string.tracer_progress_status_partial)
    val passphraseRequest = remember(context) {
        TracerTransferPassphraseRequest(
            title = context.getString(R.string.tracer_crypto_passphrase_decrypt_title),
            firstHint = context.getString(R.string.tracer_crypto_passphrase_hint),
            secondHint = null,
            requiredMessage = context.getString(R.string.tracer_crypto_passphrase_required),
            mismatchMessage = context.getString(R.string.tracer_crypto_passphrase_mismatch),
            canceledStatusText = context.getString(R.string.tracer_import_single_tracer_canceled)
        )
    }

    val importSingleTracerLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.OpenDocument()
    ) { documentUri ->
        if (documentUri == null) {
            transferCoordinator.handleSelectionCanceled(
                uiCallbacks = transferUiCallbacks,
                canceledStatusText = canceledStatusText
            )
            return@rememberLauncherForActivityResult
        }

        transferCoordinator.launchCryptoTransfer(
            uiCallbacks = transferUiCallbacks,
            passphraseRequest = passphraseRequest,
            progressOperationText = progressOperationText,
            prepareInput = {
                stageSelectedDocument(
                    context = context,
                    documentUri = documentUri,
                    expectedExtension = ".tracer"
                )
            },
            formatPrepareFailure = { error ->
                context.getString(
                    R.string.tracer_import_single_tracer_failed,
                    error.message ?: context.getString(R.string.tracer_export_unknown_error)
                )
            },
            runTransfer = { stagedTracerPath, passphrase ->
                val sourceFileName = File(stagedTracerPath).name
                val importResult = importTracerExchangeTransaction(
                    context = context,
                    tracerExchangeGateway = tracerExchangeGateway,
                    inputTracerPath = stagedTracerPath,
                    passphrase = passphrase,
                    sourceLabel = sourceFileName,
                    stagingPrefix = "single_tracer_exchange_import",
                    onProgress = { event ->
                        val overallProgress = event.currentFileProgressFraction.coerceIn(0f, 1f)
                        runBlocking(Dispatchers.Main) {
                            recordViewModel.updateCryptoProgress(
                                event = event,
                                operationTextOverride = progressOperationText,
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
                val finalStatusText = when {
                    importResult.ok -> completedText
                    importResult.hasPartialSuccess -> partialText
                    else -> failedText
                }
                TracerCryptoTransferResult(
                    progressStatusText = finalStatusText,
                    message = importResult.message
                )
            },
            formatTransferFailure = { error ->
                TracerCryptoTransferResult(
                    progressStatusText = failedText,
                    message = context.getString(
                        R.string.tracer_import_single_tracer_failed,
                        error.message ?: context.getString(R.string.tracer_export_unknown_error)
                    )
                )
            },
            afterTransfer = { _, _ ->
                // Keep Data tab export availability in sync with TXT store state.
                recordViewModel.refreshHistory()
            }
        )
    }

    return {
        dataViewModel.setStatusText(context.getString(R.string.tracer_import_select_single_tracer))
        importSingleTracerLauncher.launch(arrayOf("*/*"))
    }
}

private suspend fun importTracerExchangeTransaction(
    context: Context,
    tracerExchangeGateway: TracerExchangeGateway,
    inputTracerPath: String,
    passphrase: String,
    sourceLabel: String,
    stagingPrefix: String,
    onProgress: ((FileCryptoProgressEvent) -> Unit)? = null
): TracerExchangePackageImportResult {
    val workRoot = withContext(Dispatchers.IO) {
        buildTracerExchangeImportWorkRoot(context, stagingPrefix, sourceLabel)
    }
    val importResult = tracerExchangeGateway.importTracerExchange(
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
