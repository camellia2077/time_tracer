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
    val onExportAllMonthsTxt: () -> Unit,
    val onExportAllMonthsTracer: () -> Unit,
    val isTxtExportInProgress: Boolean,
    val isTracerExportInProgress: Boolean,
    val selectedTracerSecurityLevel: FileCryptoSecurityLevel,
    val onTracerSecurityLevelChange: (FileCryptoSecurityLevel) -> Unit,
    val onExportAndroidConfigBundle: () -> Unit
)

@Composable
internal fun rememberTracerExportActions(
    context: android.content.Context,
    coroutineScope: kotlinx.coroutines.CoroutineScope,
    recordUiState: RecordUiState,
    txtStorageGateway: TxtStorageGateway,
    cryptoGateway: FileCryptoGateway,
    recordViewModel: RecordViewModel,
    configViewModel: ConfigViewModel
): TracerExportActions {
    var isTxtExportInProgress by remember { mutableStateOf(false) }
    var isTracerExportInProgress by remember { mutableStateOf(false) }
    var tracerSecurityLevel by remember { mutableStateOf(FileCryptoSecurityLevel.INTERACTIVE) }
    val onExportAndroidConfigBundle = rememberTracerAndroidConfigExportAction(
        context = context,
        coroutineScope = coroutineScope,
        configViewModel = configViewModel
    )

    val exportAllTxtLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.OpenDocumentTree()
    ) { treeUri ->
        if (treeUri == null) {
            recordViewModel.setStatusText(context.getString(R.string.tracer_export_all_canceled))
            isTxtExportInProgress = false
            return@rememberLauncherForActivityResult
        }

        coroutineScope.launch {
            val message = runCatching {
                withContext(Dispatchers.IO) {
                    exportAllMonthsTxtToTree(
                        context = context,
                        treeUri = treeUri,
                        recordUiState = recordUiState,
                        txtStorageGateway = txtStorageGateway
                    )
                }
            }.getOrElse { error ->
                context.getString(
                    R.string.tracer_export_all_failed,
                    error.message ?: context.getString(R.string.tracer_export_unknown_error)
                )
            }
            recordViewModel.setStatusText(message)
            isTxtExportInProgress = false
        }
    }

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

            recordViewModel.startCryptoProgress("导出 TRACER")
            val exportResult = withContext(Dispatchers.IO) {
                exportAllMonthsTracerToTree(
                    context = context,
                    treeUri = treeUri,
                    recordUiState = recordUiState,
                    txtStorageGateway = txtStorageGateway,
                    cryptoGateway = cryptoGateway,
                    recordViewModel = recordViewModel,
                    passphrase = passphrase,
                    tracerSecurityLevel = tracerSecurityLevel
                )
            }
            recordViewModel.finishCryptoProgress(
                statusText = exportResult.progressStatusText,
                keepVisible = true
            )
            recordViewModel.setStatusText(exportResult.message)
            isTracerExportInProgress = false
        }
    }
    val onExportAllMonthsTxt: () -> Unit = {
        if (isTxtExportInProgress || isTracerExportInProgress) {
            recordViewModel.setStatusText(context.getString(R.string.tracer_export_already_in_progress))
        } else if (recordUiState.availableMonths.isEmpty()) {
            recordViewModel.setStatusText(context.getString(R.string.tracer_export_all_failed_no_months))
        } else {
            isTxtExportInProgress = true
            recordViewModel.setStatusText(context.getString(R.string.tracer_export_select_txt_destination))
            exportAllTxtLauncher.launch(null)
        }
    }
    val onExportAllMonthsTracer: () -> Unit = {
        if (isTxtExportInProgress || isTracerExportInProgress) {
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
        onExportAllMonthsTxt = onExportAllMonthsTxt,
        onExportAllMonthsTracer = onExportAllMonthsTracer,
        isTxtExportInProgress = isTxtExportInProgress,
        isTracerExportInProgress = isTracerExportInProgress,
        selectedTracerSecurityLevel = tracerSecurityLevel,
        onTracerSecurityLevelChange = { nextLevel ->
            tracerSecurityLevel = nextLevel
        },
        onExportAndroidConfigBundle = onExportAndroidConfigBundle
    )
}
