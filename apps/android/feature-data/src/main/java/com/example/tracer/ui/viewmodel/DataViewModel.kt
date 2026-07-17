package com.example.tracer

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch

data class DataUiState(
    val initialized: Boolean = false,
    val statusText: String = "Preparing runtime..."
)

data class DataImportOperationResult(
    val operationOk: Boolean,
    val statusText: String
)

data class DestructiveActionStatusText(
    val running: String,
    val success: String,
    val failure: String
)

class DataViewModel(
    runtimeInitializer: RuntimeInitializer,
    recordGateway: RecordGateway
) : ViewModel() {
    private val useCases: DataUseCases = DataUseCases(
        runtimeInitializer = runtimeInitializer,
        recordGateway = recordGateway
    )
    var uiState by mutableStateOf(DataUiState())
        private set

    private sealed interface DataIntent {
        data object InitializeRuntime : DataIntent
        data class IngestSingleTxtReplaceMonth(val inputPath: String) : DataIntent
        data class ClearDataAndReinitialize(val statusText: DestructiveActionStatusText) : DataIntent
        data class ClearDatabase(val statusText: DestructiveActionStatusText) : DataIntent
        data class RebuildDatabase(val statusText: DestructiveActionStatusText) : DataIntent
        data class ClearTxt(val statusText: DestructiveActionStatusText) : DataIntent
    }

    init {
        dispatchIntent(DataIntent.InitializeRuntime)
    }

    fun ingestSingleTxtReplaceMonth(inputPath: String) {
        dispatchIntent(DataIntent.IngestSingleTxtReplaceMonth(inputPath))
    }

    suspend fun ingestSingleTxtReplaceMonthAndGetResult(inputPath: String): Boolean {
        return ingestSingleTxtReplaceMonthAndGetOperationResult(inputPath).operationOk
    }

    suspend fun ingestSingleTxtReplaceMonthAndGetOperationResult(inputPath: String): DataImportOperationResult {
        val update = useCases.ingestSingleTxtReplaceMonthWithResult(
            currentState = uiState,
            inputPath = inputPath
        )
        uiState = update.state
        return DataImportOperationResult(
            operationOk = update.operationOk,
            statusText = update.state.statusText
        )
    }

    fun clearDataAndReinitialize(statusText: DestructiveActionStatusText) {
        dispatchIntent(DataIntent.ClearDataAndReinitialize(statusText))
    }

    fun clearDatabase(statusText: DestructiveActionStatusText) {
        dispatchIntent(DataIntent.ClearDatabase(statusText))
    }

    fun rebuildDatabase(statusText: DestructiveActionStatusText) {
        dispatchIntent(DataIntent.RebuildDatabase(statusText))
    }

    fun clearTxt(statusText: DestructiveActionStatusText) {
        dispatchIntent(DataIntent.ClearTxt(statusText))
    }

    fun setStatusText(text: String) {
        uiState = uiState.copy(statusText = text)
    }

    private fun dispatchIntent(intent: DataIntent) {
        viewModelScope.launch {
            uiState = when (intent) {
                DataIntent.InitializeRuntime -> useCases.initializeRuntime(uiState)
                is DataIntent.IngestSingleTxtReplaceMonth -> {
                    useCases.ingestSingleTxtReplaceMonth(
                        currentState = uiState,
                        inputPath = intent.inputPath
                    )
                }

                is DataIntent.ClearDataAndReinitialize -> useCases.clearDataAndReinitialize(
                    currentState = uiState,
                    statusText = intent.statusText
                )
                is DataIntent.ClearDatabase -> useCases.clearDatabase(
                    currentState = uiState,
                    statusText = intent.statusText
                )
                is DataIntent.RebuildDatabase -> useCases.rebuildDatabase(
                    currentState = uiState,
                    statusText = intent.statusText
                )
                is DataIntent.ClearTxt -> useCases.clearTxt(
                    currentState = uiState,
                    statusText = intent.statusText
                )
            }
        }
    }
}

class DataViewModelFactory(
    private val runtimeInitializer: RuntimeInitializer,
    private val recordGateway: RecordGateway
) : ViewModelProvider.Factory {
    override fun <T : ViewModel> create(modelClass: Class<T>): T {
        if (modelClass.isAssignableFrom(DataViewModel::class.java)) {
            @Suppress("UNCHECKED_CAST")
            return DataViewModel(runtimeInitializer, recordGateway) as T
        }
        throw IllegalArgumentException("Unknown ViewModel class: ${modelClass.name}")
    }
}
