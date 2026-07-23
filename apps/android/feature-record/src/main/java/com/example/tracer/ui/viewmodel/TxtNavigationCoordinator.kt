package com.example.tracer

import android.util.Log
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock

internal sealed interface TxtNavigationRequest {
    val description: String

    data object Refresh : TxtNavigationRequest {
        override val description: String = "refreshHistory"
    }

    data class OpenFile(val path: String) : TxtNavigationRequest {
        override val description: String = "openHistoryFile path=$path"
    }

    data class OpenMonth(val month: String) : TxtNavigationRequest {
        override val description: String = "openMonth month=$month"
    }

    data object PreviousMonth : TxtNavigationRequest {
        override val description: String = "openPreviousMonth"
    }

    data object NextMonth : TxtNavigationRequest {
        override val description: String = "openNextMonth"
    }
}

internal class TxtNavigationCoordinator(
    private val scope: CoroutineScope,
    private val stateProvider: () -> RecordUiState,
    private val stateConsumer: (RecordUiState) -> Unit,
    private val navigate: suspend (RecordUiState, TxtNavigationRequest) -> RecordUiState
) {
    private val mutex = Mutex()
    private var nextRequestId: Long = 0L

    fun launch(request: TxtNavigationRequest) {
        val requestId = ++nextRequestId
        scope.launch {
            mutex.withLock {
                val before = stateProvider()
                log(
                    "navigation start id=$requestId operation=${request.description} " +
                        "selectedMonth=${before.selectedMonth} " +
                        "selectedFile=${before.selectedHistoryFile}"
                )

                val next = navigate(before, request)
                stateConsumer(next)

                log(
                    "navigation complete id=$requestId operation=${request.description} " +
                        "selectedMonth=${next.selectedMonth} " +
                        "selectedFile=${next.selectedHistoryFile}"
                )
            }
        }
    }

    private fun log(message: String) {
        runCatching { Log.d(LOG_TAG, message) }
    }

    private companion object {
        private const val LOG_TAG = "TxtTab"
    }
}
