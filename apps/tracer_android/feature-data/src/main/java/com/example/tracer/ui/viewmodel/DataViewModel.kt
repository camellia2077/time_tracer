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
    private val runtimeInitializer: RuntimeInitializer,
    private val recordGateway: RecordGateway
) : ViewModel() {
    var uiState by mutableStateOf(DataUiState())
        private set

    init {
        initializeRuntime()
    }

    private fun initializeRuntime() {
        viewModelScope.launch {
            uiState = uiState.copy(statusText = "nativeInit running...")
            val result = runtimeInitializer.initializeRuntime()
            uiState = uiState.copy(
                initialized = result.initialized,
                statusText = "nativeInit -> ${result.rawResponse}"
            )
        }
    }

    fun ingestSmoke() {
        viewModelScope.launch {
            uiState = uiState.copy(statusText = "nativeIngest(smoke) running...")
            val result = runtimeInitializer.ingestSmoke()
            uiState = uiState.copy(
                initialized = result.initialized,
                statusText = "nativeIngest(smoke) -> ${result.rawResponse}"
            )
        }
    }

    fun ingestFull() {
        viewModelScope.launch {
            uiState = uiState.copy(statusText = "nativeIngest(full) running...")
            val result = runtimeInitializer.ingestFull()
            uiState = uiState.copy(
                initialized = result.initialized,
                statusText = "nativeIngest(full) -> ${result.rawResponse}"
            )
        }
    }

    fun ingestSingleTxtReplaceMonth(inputPath: String) {
        viewModelScope.launch {
            uiState = uiState.copy(
                statusText = "nativeIngest(single_txt_replace_month) running..."
            )
            val result = runtimeInitializer.ingestSingleTxtReplaceMonth(inputPath)
            uiState = uiState.copy(
                initialized = result.initialized,
                statusText = "nativeIngest(single_txt_replace_month) -> ${result.rawResponse}"
            )
        }
    }

    fun clearDataAndReinitialize() {
        viewModelScope.launch {
            uiState = uiState.copy(statusText = "clearing app data...")
            val result = runtimeInitializer.clearAndReinitialize()
            uiState = uiState.copy(
                initialized = result.initialized,
                statusText = "${result.clearMessage}\nnativeInit -> ${result.initResponse}"
            )
        }
    }

    fun clearTxt() {
        viewModelScope.launch {
            uiState = uiState.copy(statusText = "clearing txt...")
            val result = recordGateway.clearTxt()
            uiState = uiState.copy(statusText = result.message)
        }
    }

    fun setStatusText(text: String) {
        uiState = uiState.copy(statusText = text)
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
