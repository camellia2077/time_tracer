package com.example.tracer

import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import kotlinx.coroutines.launch

internal data class TracerExportActions(
    val onExportAllMonthsTracer: () -> Unit,
    val isTracerExportInProgress: Boolean,
    val selectedTracerSecurityLevel: FileCryptoSecurityLevel,
    val onTracerSecurityLevelChange: (FileCryptoSecurityLevel) -> Unit
)

@Composable
internal fun rememberTracerExportActions(
    context: android.content.Context,
    coroutineScope: kotlinx.coroutines.CoroutineScope,
    recordUiState: RecordUiState,
    txtStorageGateway: TxtStorageGateway,
    tracerExchangeGateway: TracerExchangeGateway,
    recordViewModel: RecordViewModel
): TracerExportActions {
    var isTracerExportInProgress by remember { mutableStateOf(false) }
    var tracerSecurityLevel by remember { mutableStateOf(FileCryptoSecurityLevel.INTERACTIVE) }

    val exportAllTracerLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.OpenDocumentTree()
    ) { treeUri ->
        if (treeUri == null) {
            recordViewModel.clearCryptoProgress()
            recordViewModel.setStatusText(context.getString(R.string.tracer_export_all_tracer_canceled))
            isTracerExportInProgress = false
            return@rememberLauncherForActivityResult
        }

        coroutineScope.launch {
            val passphrase = promptPassphrase(
                context = context,
                title = context.getString(R.string.tracer_crypto_passphrase_encrypt_title),
                firstHint = context.getString(R.string.tracer_crypto_passphrase_hint),
                secondHint = context.getString(R.string.tracer_crypto_passphrase_confirm_hint),
                requiredMessage = context.getString(R.string.tracer_crypto_passphrase_required),
                mismatchMessage = context.getString(R.string.tracer_crypto_passphrase_mismatch)
            )
            if (passphrase.isNullOrBlank()) {
                recordViewModel.clearCryptoProgress()
                recordViewModel.setStatusText(context.getString(R.string.tracer_export_all_tracer_canceled))
                isTracerExportInProgress = false
                return@launch
            }

            recordViewModel.startCryptoProgress(
                context.getString(R.string.tracer_progress_operation_export_tracer)
            )
            val exportResult = withContext(Dispatchers.IO) {
                exportAllMonthsTracerToTree(
                    context = context,
                    treeUri = treeUri,
                    recordUiState = recordUiState,
                    txtStorageGateway = txtStorageGateway,
                    tracerExchangeGateway = tracerExchangeGateway,
                    recordViewModel = recordViewModel,
                    passphrase = passphrase,
                    tracerSecurityLevel = tracerSecurityLevel
                )
            }
            recordViewModel.finishCryptoProgress(
                statusText = exportResult.progressStatusText,
                keepVisible = true,
                detailsTextOverride = exportResult.message
            )
            recordViewModel.setStatusText(exportResult.message)
            isTracerExportInProgress = false
        }
    }
    val onExportAllMonthsTracer: () -> Unit = {
        if (isTracerExportInProgress) {
            recordViewModel.setStatusText(context.getString(R.string.tracer_export_already_in_progress))
        } else if (recordUiState.availableMonths.isEmpty()) {
            recordViewModel.setStatusText(context.getString(R.string.tracer_export_all_failed_no_months))
        } else {
            isTracerExportInProgress = true
            recordViewModel.setStatusText(context.getString(R.string.tracer_export_select_tracer_destination))
            exportAllTracerLauncher.launch(null)
        }
    }

    return TracerExportActions(
        onExportAllMonthsTracer = onExportAllMonthsTracer,
        isTracerExportInProgress = isTracerExportInProgress,
        selectedTracerSecurityLevel = tracerSecurityLevel,
        onTracerSecurityLevelChange = { nextLevel ->
            tracerSecurityLevel = nextLevel
        }
    )
}
