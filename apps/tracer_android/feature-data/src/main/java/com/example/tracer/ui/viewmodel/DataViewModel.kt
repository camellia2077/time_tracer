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
        data object IngestFull : DataIntent
        data class IngestSingleTxtReplaceMonth(val inputPath: String) : DataIntent
        data object ClearDataAndReinitialize : DataIntent
        data object ClearTxt : DataIntent
    }

    init {
        dispatchIntent(DataIntent.InitializeRuntime)
    }

    fun ingestFull() {
        dispatchIntent(DataIntent.IngestFull)
    }

    suspend fun ingestFullAndGetResult(): Boolean {
        val update = useCases.ingestFullWithResult(uiState)
        uiState = update.state
        return update.operationOk
    }

    fun ingestSingleTxtReplaceMonth(inputPath: String) {
        dispatchIntent(DataIntent.IngestSingleTxtReplaceMonth(inputPath))
    }

    suspend fun ingestSingleTxtReplaceMonthAndGetResult(inputPath: String): Boolean {
        val update = useCases.ingestSingleTxtReplaceMonthWithResult(
            currentState = uiState,
            inputPath = inputPath
        )
        uiState = update.state
        return update.operationOk
    }

    fun clearDataAndReinitialize() {
        dispatchIntent(DataIntent.ClearDataAndReinitialize)
    }

    fun clearTxt() {
        dispatchIntent(DataIntent.ClearTxt)
    }

    fun setStatusText(text: String) {
        uiState = uiState.copy(statusText = text)
    }

    private fun dispatchIntent(intent: DataIntent) {
        viewModelScope.launch {
            uiState = when (intent) {
                DataIntent.InitializeRuntime -> useCases.initializeRuntime(uiState)
                DataIntent.IngestFull -> useCases.ingestFull(uiState)
                is DataIntent.IngestSingleTxtReplaceMonth -> {
                    useCases.ingestSingleTxtReplaceMonth(
                        currentState = uiState,
                        inputPath = intent.inputPath
                    )
                }

                DataIntent.ClearDataAndReinitialize -> useCases.clearDataAndReinitialize(uiState)
                DataIntent.ClearTxt -> useCases.clearTxt(uiState)
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
