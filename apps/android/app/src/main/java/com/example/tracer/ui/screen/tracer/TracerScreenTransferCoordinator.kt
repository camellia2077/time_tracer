package com.example.tracer

import android.content.Context
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

internal data class TracerTransferUiCallbacks(
    val setStatusText: (String) -> Unit,
    val clearCryptoProgress: () -> Unit,
    val startCryptoProgress: (String) -> Unit,
    val finishCryptoProgress: (String, String) -> Unit
)

internal data class TracerTransferPassphraseRequest(
    val title: String,
    val firstHint: String,
    val secondHint: String? = null,
    val requiredMessage: String,
    val mismatchMessage: String,
    val canceledStatusText: String
)

internal data class TracerCryptoTransferResult(
    val progressStatusText: String,
    val message: String
)

internal data class TracerPreparedTransferResult(
    val statusText: String? = null
)

@Composable
internal fun rememberTracerScreenTransferCoordinator(
    context: Context,
    coroutineScope: CoroutineScope
): TracerScreenTransferCoordinator = remember(context, coroutineScope) {
    TracerScreenTransferCoordinator(
        context = context,
        coroutineScope = coroutineScope
    )
}

internal class TracerScreenTransferCoordinator(
    private val context: Context,
    private val coroutineScope: CoroutineScope
) {
    fun handleSelectionCanceled(
        uiCallbacks: TracerTransferUiCallbacks,
        canceledStatusText: String,
        onFinally: () -> Unit = {}
    ) {
        cancelTransfer(uiCallbacks, canceledStatusText)
        onFinally()
    }

    fun <TPrepared> launchCryptoTransfer(
        uiCallbacks: TracerTransferUiCallbacks,
        passphraseRequest: TracerTransferPassphraseRequest,
        progressOperationText: String,
        prepareInput: suspend () -> TPrepared,
        formatPrepareFailure: (Throwable) -> String,
        runTransfer: suspend (TPrepared, String) -> TracerCryptoTransferResult,
        formatTransferFailure: (Throwable) -> TracerCryptoTransferResult,
        afterTransfer: suspend (TPrepared, TracerCryptoTransferResult) -> Unit = { _, _ -> },
        onFinally: () -> Unit = {}
    ) {
        coroutineScope.launch {
            try {
                val prepared = runCatching {
                    withContext(Dispatchers.IO) {
                        prepareInput()
                    }
                }.getOrElse { error ->
                    uiCallbacks.setStatusText(formatPrepareFailure(error))
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
                    cancelTransfer(uiCallbacks, passphraseRequest.canceledStatusText)
                    return@launch
                }

                uiCallbacks.startCryptoProgress(progressOperationText)
                val transferResult = runCatching {
                    runTransfer(prepared, passphrase)
                }.getOrElse { error ->
                    formatTransferFailure(error)
                }
                runCatching {
                    afterTransfer(prepared, transferResult)
                }
                uiCallbacks.finishCryptoProgress(
                    transferResult.progressStatusText,
                    transferResult.message
                )
                uiCallbacks.setStatusText(transferResult.message)
            } finally {
                onFinally()
            }
        }
    }

    fun <TPrepared> launchPreparedTransfer(
        uiCallbacks: TracerTransferUiCallbacks,
        prepareInput: suspend () -> TPrepared,
        formatPrepareFailure: (Throwable) -> String,
        runTransfer: suspend (TPrepared) -> TracerPreparedTransferResult,
        formatTransferFailure: (Throwable) -> TracerPreparedTransferResult,
        afterTransfer: suspend (TPrepared, TracerPreparedTransferResult) -> Unit = { _, _ -> },
        onFinally: () -> Unit = {}
    ) {
        coroutineScope.launch {
            try {
                val prepared = runCatching {
                    withContext(Dispatchers.IO) {
                        prepareInput()
                    }
                }.getOrElse { error ->
                    uiCallbacks.setStatusText(formatPrepareFailure(error))
                    return@launch
                }

                val transferResult = runCatching {
                    runTransfer(prepared)
                }.getOrElse { error ->
                    formatTransferFailure(error)
                }
                runCatching {
                    afterTransfer(prepared, transferResult)
                }
                if (transferResult.statusText != null) {
                    uiCallbacks.setStatusText(transferResult.statusText)
                }
            } finally {
                onFinally()
            }
        }
    }

    private fun cancelTransfer(
        uiCallbacks: TracerTransferUiCallbacks,
        canceledStatusText: String
    ) {
        uiCallbacks.clearCryptoProgress()
        uiCallbacks.setStatusText(canceledStatusText)
    }
}
