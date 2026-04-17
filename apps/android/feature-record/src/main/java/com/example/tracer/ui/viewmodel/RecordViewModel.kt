package com.example.tracer

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch

enum class RecordLogicalDayTarget {
    YESTERDAY,
    TODAY
}

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
    val logicalDayTarget: RecordLogicalDayTarget =
        defaultLogicalDayTarget(System.currentTimeMillis()),
    val logicalDayIsUserOverride: Boolean = false,
    val historyFiles: List<String> = emptyList(),
    val txtInspectionEntries: List<TxtInspectionEntry> = emptyList(),
    val availableMonths: List<String> = emptyList(),
    val selectedMonth: String = "",
    val selectedHistoryFile: String = "",
    val selectedHistoryContent: String = "",
    val editableHistoryContent: String = "",
    // Keep unsaved TXT edits in memory for the current app session so switching tabs/months/files
    // feels like a normal editor: users do not lose draft text until they explicitly save or
    // discard it. This cache is intentionally UI-session-only and is never treated as persisted
    // storage.
    val historyDraftsByFile: Map<String, String> = emptyMap(),
    val quickActivities: List<String> = listOf("meal", "洗漱", "上厕所"),
    val assistExpanded: Boolean = false,
    val assistSettingsExpanded: Boolean = false,
    val suggestionLookbackDays: Int = 7,
    val suggestionTopN: Int = 5,
    val suggestedActivities: List<String> = emptyList(),
    val suggestionsVisible: Boolean = false,
    val isSuggestionsLoading: Boolean = false,
    val isTxtPreviewVisible: Boolean = false,
    val isTxtPreviewLoading: Boolean = false,
    val txtPreviewStatusText: String = "",
    val statusText: String = "",
    val cryptoProgress: CryptoProgressUiState = CryptoProgressUiState()
)

class RecordViewModel(private val recordUseCases: RecordUseCases) : ViewModel() {
    private val intentHandler = RecordIntentHandler(
        useCaseCaller = RecordUseCaseCaller(recordUseCases)
    )
    private var txtPreviewRequestVersion: Long = 0L

    var uiState by mutableStateOf(RecordUiState())
        private set

    fun onRecordContentChange(value: String) {
        uiState = intentHandler.onRecordContentChange(uiState, value)
    }

    fun onRecordRemarkChange(value: String) {
        uiState = intentHandler.onRecordRemarkChange(uiState, value)
    }

    fun selectLogicalDayYesterday() {
        uiState = intentHandler.selectLogicalDayYesterday(uiState)
    }

    fun selectLogicalDayToday() {
        uiState = intentHandler.selectLogicalDayToday(uiState)
    }

    fun refreshLogicalDayDefault(currentTimeMillis: Long = System.currentTimeMillis()) {
        uiState = intentHandler.refreshLogicalDayDefault(
            state = uiState,
            currentTimeMillis = currentTimeMillis
        )
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
        phaseTextOverride: String? = null,
        overallProgressOverride: Float? = null,
        overallTextOverride: String? = null,
        currentTextOverride: String? = null,
        currentProgressOverride: Float? = null
    ) {
        uiState = intentHandler.updateCryptoProgress(
            state = uiState,
            event = event,
            operationTextOverride = operationTextOverride,
            phaseTextOverride = phaseTextOverride,
            overallProgressOverride = overallProgressOverride,
            overallTextOverride = overallTextOverride,
            currentTextOverride = currentTextOverride,
            currentProgressOverride = currentProgressOverride
        )
    }

    fun finishCryptoProgress(
        statusText: String,
        keepVisible: Boolean,
        detailsTextOverride: String? = null
    ) {
        uiState = intentHandler.finishCryptoProgress(
            state = uiState,
            statusText = statusText,
            keepVisible = keepVisible,
            detailsTextOverride = detailsTextOverride
        )
    }

    fun clearCryptoProgress() {
        uiState = intentHandler.clearCryptoProgress(uiState)
    }

    fun openTxtPreview() {
        uiState = intentHandler.showTxtPreviewLoading(uiState)
        txtPreviewRequestVersion += 1L
        val requestVersion = txtPreviewRequestVersion
        viewModelScope.launch {
            val previousStatusText = uiState.statusText
            val resultState = intentHandler.openTxtPreview(uiState)
            if (txtPreviewRequestVersion != requestVersion) {
                return@launch
            }
            uiState = resultState.copy(
                isTxtPreviewVisible = true,
                isTxtPreviewLoading = false,
                txtPreviewStatusText = resultState.statusText,
                statusText = previousStatusText
            )
        }
    }

    fun dismissTxtPreview() {
        txtPreviewRequestVersion += 1L
        uiState = intentHandler.dismissTxtPreview(uiState)
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

    fun createCurrentMonthTxt() {
        viewModelScope.launch {
            uiState = intentHandler.createCurrentMonthTxt(uiState)
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
