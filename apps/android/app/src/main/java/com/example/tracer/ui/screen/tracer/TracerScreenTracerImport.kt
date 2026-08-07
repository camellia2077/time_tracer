package com.example.tracer

import android.content.Context
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.File

@Composable
internal fun rememberTracerSingleTracerImportAction(
    context: Context,
    coroutineScope: kotlinx.coroutines.CoroutineScope,
    dataViewModel: DataViewModel,
    recordViewModel: RecordViewModel,
    tracerExchangeGateway: TracerExchangeGateway
): () -> Unit {
    val canceledStatusText = context.getString(R.string.tracer_import_single_tracer_canceled)
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
            dataViewModel.setStatusText(canceledStatusText)
            return@rememberLauncherForActivityResult
        }

        coroutineScope.launch {
            val stagedTracerPath = runCatching {
                withContext(Dispatchers.IO) {
                    stageSelectedDocument(
                        context = context,
                        documentUri = documentUri,
                        expectedExtension = ".zip"
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
                title = passphraseRequest.title,
                firstHint = passphraseRequest.firstHint,
                secondHint = passphraseRequest.secondHint,
                requiredMessage = passphraseRequest.requiredMessage,
                mismatchMessage = passphraseRequest.mismatchMessage
            )
            if (passphrase.isNullOrBlank()) {
                dataViewModel.setStatusText(canceledStatusText)
                return@launch
            }

            // Inspect before import so the user can confirm this is the
            // intended backup package before we replace managed TXT months and
            // rebuild the database. The main UX goal is to reduce accidental
            // imports of stale `data.zip` files.
            val inspectResult = tracerExchangeGateway.inspectTracerExchange(
                inputPath = stagedTracerPath,
                passphrase = passphrase,
            )
            if (!inspectResult.ok) {
                dataViewModel.setStatusText(
                    context.getString(
                        R.string.tracer_import_single_tracer_failed,
                        inspectResult.message
                    )
                )
                return@launch
            }

            val confirmed = confirmTracerExchangeImport(
                context = context,
                inspectResult = inspectResult,
            )
            if (!confirmed) {
                dataViewModel.setStatusText(canceledStatusText)
                return@launch
            }

            val sourceFileName = File(stagedTracerPath).name
            val transferMessage = runCatching {
                importTracerExchangeTransaction(
                    context = context,
                    tracerExchangeGateway = tracerExchangeGateway,
                    inputTracerPath = stagedTracerPath,
                    passphrase = passphrase,
                    sourceLabel = sourceFileName,
                    stagingPrefix = "single_tracer_exchange_import"
                )
            }.getOrElse { error ->
                context.getString(
                    R.string.tracer_import_single_tracer_failed,
                    error.message ?: context.getString(R.string.tracer_export_unknown_error)
                )
            }

            recordViewModel.refreshHistory()
            dataViewModel.setStatusText(transferMessage)
        }
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
    stagingPrefix: String
): String {
    val workRoot = withContext(Dispatchers.IO) {
        buildTracerExchangeImportWorkRoot(context, stagingPrefix, sourceLabel)
    }
    val importResult = tracerExchangeGateway.importTracerExchange(
        inputPath = inputTracerPath,
        workRoot = workRoot.absolutePath,
        passphrase = passphrase
    )
    if (!importResult.ok) {
        return importResult.message
    }

    return buildSingleTracerImportSummary(
        context = context,
        successCount = importResult.payloadFileCount,
        totalCount = importResult.payloadFileCount,
        errors = emptyList()
    )
}
