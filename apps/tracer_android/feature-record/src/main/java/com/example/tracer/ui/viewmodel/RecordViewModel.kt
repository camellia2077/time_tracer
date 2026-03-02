package com.example.tracer

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

data class CryptoProgressUiState(
    val isVisible: Boolean = false,
    val operationText: String = "",
    val phaseText: String = "",
    val statusText: String = "",
    val overallProgress: Float = 0f,
    val overallText: String = "",
    val currentProgress: Float = 0f,
    val currentText: String = "",
    val detailsText: String = "",
    val advancedDetailsText: String = "",
    val startedAtEpochMs: Long = 0L
)

data class RecordUiState(
    val recordContent: String = "",
    val recordRemark: String = "",
    val useManualDate: Boolean = false,
    val manualDate: String = currentIsoDate(),
    val historyFiles: List<String> = emptyList(),
    val availableMonths: List<String> = emptyList(),
    val selectedMonth: String = "",
    val selectedHistoryFile: String = "",
    val selectedHistoryContent: String = "",
    val editableHistoryContent: String = "",
    val quickActivities: List<String> = listOf("meal", "洗漱", "上厕所"),
    val assistExpanded: Boolean = false,
    val assistSettingsExpanded: Boolean = false,
    val suggestionLookbackDays: Int = 7,
    val suggestionTopN: Int = 5,
    val suggestedActivities: List<String> = emptyList(),
    val suggestionsVisible: Boolean = false,
    val isSuggestionsLoading: Boolean = false,
    val statusText: String = "",
    val cryptoProgress: CryptoProgressUiState = CryptoProgressUiState()
)

private fun currentIsoDate(): String =
    SimpleDateFormat("yyyy-MM-dd", Locale.US).format(Date())

class RecordViewModel(private val recordUseCases: RecordUseCases) : ViewModel() {
    private val intentHandler = RecordIntentHandler(
        useCaseCaller = RecordUseCaseCaller(recordUseCases)
    )

    var uiState by mutableStateOf(RecordUiState())
        private set

    fun onRecordContentChange(value: String) {
        uiState = intentHandler.onRecordContentChange(uiState, value)
    }

    fun onRecordRemarkChange(value: String) {
        uiState = intentHandler.onRecordRemarkChange(uiState, value)
    }

    fun useAutoDate() {
        uiState = intentHandler.useAutoDate(uiState)
    }

    fun useManualDate() {
        uiState = intentHandler.useManualDate(uiState)
    }

    fun onManualDateChange(value: String) {
        uiState = intentHandler.onManualDateChange(uiState, value)
    }

    fun updateEditableHistoryContent(value: String) {
        uiState = intentHandler.updateEditableHistoryContent(uiState, value)
    }

    fun updateSuggestionPreferences(lookbackDays: Int, topN: Int) {
        uiState = intentHandler.updateSuggestionPreferences(
            state = uiState,
            lookbackDays = lookbackDays,
            topN = topN
        )
    }

    fun updateQuickActivities(values: List<String>) {
        uiState = intentHandler.updateQuickActivities(uiState, values)
    }

    fun updateAssistUiState(assistExpanded: Boolean, assistSettingsExpanded: Boolean) {
        uiState = intentHandler.updateAssistUiState(
            state = uiState,
            assistExpanded = assistExpanded,
            assistSettingsExpanded = assistSettingsExpanded
        )
    }

    fun toggleSuggestions() {
        if (uiState.suggestionsVisible) {
            uiState = intentHandler.hideSuggestions(uiState)
            return
        }

        uiState = intentHandler.showSuggestionsLoading(uiState)
        viewModelScope.launch {
            val resultState = intentHandler.loadActivitySuggestions(uiState)
            uiState = uiState.copy(
                suggestedActivities = resultState.suggestedActivities,
                isSuggestionsLoading = resultState.isSuggestionsLoading,
                statusText = resultState.statusText
            )
        }
    }

    fun applySuggestedActivity(activityName: String) {
        uiState = intentHandler.applySuggestedActivity(uiState, activityName)
    }

    fun setStatusText(message: String) {
        uiState = intentHandler.setStatusText(uiState, message)
    }

    fun startCryptoProgress(operationText: String) {
        uiState = intentHandler.startCryptoProgress(uiState, operationText)
    }

    fun updateCryptoProgress(
        event: FileCryptoProgressEvent,
        operationTextOverride: String? = null,
        overallProgressOverride: Float? = null,
        overallTextOverride: String? = null,
        currentTextOverride: String? = null,
        currentProgressOverride: Float? = null
    ) {
        uiState = intentHandler.updateCryptoProgress(
            state = uiState,
            event = event,
            operationTextOverride = operationTextOverride,
            overallProgressOverride = overallProgressOverride,
            overallTextOverride = overallTextOverride,
            currentTextOverride = currentTextOverride,
            currentProgressOverride = currentProgressOverride
        )
    }

    fun finishCryptoProgress(statusText: String, keepVisible: Boolean) {
        uiState = intentHandler.finishCryptoProgress(
            state = uiState,
            statusText = statusText,
            keepVisible = keepVisible
        )
    }

    fun clearCryptoProgress() {
        uiState = intentHandler.clearCryptoProgress(uiState)
    }

    fun recordNow() {
        viewModelScope.launch {
            uiState = intentHandler.recordNow(uiState)
        }
    }

    fun refreshHistory() {
        viewModelScope.launch {
            uiState = intentHandler.refreshHistory(uiState)
        }
    }

    fun openHistoryFile(path: String) {
        viewModelScope.launch {
            uiState = intentHandler.openHistoryFile(uiState, path)
        }
    }

    fun openMonth(month: String) {
        viewModelScope.launch {
            uiState = intentHandler.openMonth(uiState, month)
        }
    }

    fun openPreviousMonth() {
        viewModelScope.launch {
            uiState = intentHandler.openPreviousMonth(uiState)
        }
    }

    fun openNextMonth() {
        viewModelScope.launch {
            uiState = intentHandler.openNextMonth(uiState)
        }
    }

    fun saveHistoryFileAndSync() {
        viewModelScope.launch {
            uiState = intentHandler.saveHistoryFileAndSync(uiState)
        }
    }

    fun discardUnsavedHistoryDraft() {
        uiState = intentHandler.discardUnsavedHistoryDraft(uiState)
    }

    fun clearTxtEditorState() {
        uiState = intentHandler.clearTxtEditorState(uiState)
    }
}

class RecordViewModelFactory(
    private val recordGateway: RecordGateway,
    private val txtStorageGateway: TxtStorageGateway,
    private val queryGateway: QueryGateway
) : ViewModelProvider.Factory {
    override fun <T : ViewModel> create(modelClass: Class<T>): T {
        if (modelClass.isAssignableFrom(RecordViewModel::class.java)) {
            @Suppress("UNCHECKED_CAST")
            return RecordViewModel(
                RecordUseCases(
                    recordGateway = recordGateway,
                    txtStorageGateway = txtStorageGateway,
                    queryGateway = queryGateway
                )
            ) as T
        }
        throw IllegalArgumentException("Unknown ViewModel class: ${modelClass.name}")
    }
}
