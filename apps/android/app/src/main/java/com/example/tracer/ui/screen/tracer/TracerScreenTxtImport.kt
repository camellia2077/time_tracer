package com.example.tracer

import android.content.Context
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember

@Composable
internal fun rememberTracerSingleTxtImportAction(
    context: Context,
    coroutineScope: kotlinx.coroutines.CoroutineScope,
    dataViewModel: DataViewModel,
    recordViewModel: RecordViewModel
): () -> Unit {
    val transferCoordinator = rememberTracerScreenTransferCoordinator(
        context = context,
        coroutineScope = coroutineScope
    )
    val transferUiCallbacks = remember(dataViewModel) {
        TracerTransferUiCallbacks(
            setStatusText = dataViewModel::setStatusText,
            clearCryptoProgress = {},
            startCryptoProgress = {},
            finishCryptoProgress = { _, _ -> }
        )
    }
    val canceledStatusText = context.getString(R.string.tracer_import_single_txt_canceled)
    val readSelectedDocumentText =
        context.getString(R.string.tracer_import_error_read_selected_document)

    val importSingleTxtLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.OpenDocument()
    ) { documentUri ->
        if (documentUri == null) {
            transferCoordinator.handleSelectionCanceled(
                uiCallbacks = transferUiCallbacks,
                canceledStatusText = canceledStatusText
            )
            return@rememberLauncherForActivityResult
        }

        transferCoordinator.launchPreparedTransfer(
            uiCallbacks = transferUiCallbacks,
            prepareInput = {
                stageSelectedTxtDocument(context = context, documentUri = documentUri)
            },
            formatPrepareFailure = { error ->
                context.getString(
                    R.string.tracer_import_single_txt_failed,
                    error.message ?: readSelectedDocumentText
                )
            },
            runTransfer = { stagedInputPath ->
                dataViewModel.ingestSingleTxtReplaceMonthAndGetResult(stagedInputPath)
                TracerPreparedTransferResult()
            },
            formatTransferFailure = { error ->
                TracerPreparedTransferResult(
                    statusText = context.getString(
                        R.string.tracer_import_single_txt_failed,
                        error.message ?: readSelectedDocumentText
                    )
                )
            },
            afterTransfer = { _, _ ->
                // Keep Data tab export availability in sync with TXT store state,
                // even when ingest returns non-ok (e.g. partial/diagnostic response).
                recordViewModel.refreshHistory()
            }
        )
    }

    return {
        dataViewModel.setStatusText(context.getString(R.string.tracer_import_select_single_txt))
        importSingleTxtLauncher.launch(arrayOf("text/plain", "text/*", "*/*"))
    }
}
