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
    val statusText: String = ""
)

private fun currentIsoDate(): String =
    SimpleDateFormat("yyyy-MM-dd", Locale.US).format(Date())

class RecordViewModel(private val recordUseCases: RecordUseCases) : ViewModel() {
    var uiState by mutableStateOf(RecordUiState())
        private set

    fun onRecordContentChange(value: String) {
        uiState = uiState.copy(recordContent = value)
    }

    fun onRecordRemarkChange(value: String) {
        uiState = uiState.copy(recordRemark = value)
    }

    fun useAutoDate() {
        uiState = uiState.copy(useManualDate = false)
    }

    fun useManualDate() {
        uiState = uiState.copy(useManualDate = true)
    }

    fun onManualDateChange(value: String) {
        uiState = uiState.copy(manualDate = value)
    }

    fun updateEditableHistoryContent(value: String) {
        uiState = uiState.copy(editableHistoryContent = value)
    }

    fun updateSuggestionPreferences(lookbackDays: Int, topN: Int) {
        if (uiState.suggestionLookbackDays == lookbackDays && uiState.suggestionTopN == topN) {
            return
        }

        uiState = uiState.copy(
            suggestionLookbackDays = lookbackDays,
            suggestionTopN = topN
        )
    }

    fun updateQuickActivities(values: List<String>) {
        val normalized = values
            .map { it.trim() }
            .filter { it.isNotEmpty() }
            .distinct()
        if (normalized.isEmpty() || uiState.quickActivities == normalized) {
            return
        }
        uiState = uiState.copy(quickActivities = normalized)
    }

    fun updateAssistUiState(assistExpanded: Boolean, assistSettingsExpanded: Boolean) {
        if (uiState.assistExpanded == assistExpanded &&
            uiState.assistSettingsExpanded == assistSettingsExpanded
        ) {
            return
        }
        uiState = uiState.copy(
            assistExpanded = assistExpanded,
            assistSettingsExpanded = assistSettingsExpanded
        )
    }

    fun toggleSuggestions() {
        if (uiState.suggestionsVisible) {
            uiState = uiState.copy(suggestionsVisible = false)
            return
        }

        uiState = uiState.copy(
            suggestionsVisible = true,
            isSuggestionsLoading = true,
            statusText = "Loading activity suggestions..."
        )
        viewModelScope.launch {
            val resultState = recordUseCases.loadActivitySuggestions(
                state = uiState,
                lookbackDays = uiState.suggestionLookbackDays,
                topN = uiState.suggestionTopN
            )
            uiState = uiState.copy(
                suggestedActivities = resultState.suggestedActivities,
                isSuggestionsLoading = resultState.isSuggestionsLoading,
                statusText = resultState.statusText
            )
        }
    }

    fun applySuggestedActivity(activityName: String) {
        uiState = uiState.copy(recordContent = activityName)
    }

    fun setStatusText(message: String) {
        uiState = uiState.copy(statusText = message)
    }

    fun recordNow() {
        viewModelScope.launch {
            uiState = recordUseCases.recordNow(uiState)
        }
    }

    fun createCurrentMonthTxt() {
        viewModelScope.launch {
            uiState = recordUseCases.createCurrentMonthTxt(uiState)
        }
    }

    fun refreshHistory() {
        viewModelScope.launch {
            uiState = recordUseCases.refreshHistory(uiState)
        }
    }

    fun openHistoryFile(path: String) {
        viewModelScope.launch {
            uiState = recordUseCases.openHistoryFile(uiState, path)
        }
    }

    fun openMonth(month: String) {
        viewModelScope.launch {
            uiState = recordUseCases.openMonth(uiState, month)
        }
    }

    fun openPreviousMonth() {
        viewModelScope.launch {
            uiState = recordUseCases.openPreviousMonth(uiState)
        }
    }

    fun openNextMonth() {
        viewModelScope.launch {
            uiState = recordUseCases.openNextMonth(uiState)
        }
    }

    fun saveHistoryFileAndSync() {
        viewModelScope.launch {
            uiState = recordUseCases.saveHistoryFileAndSync(uiState)
        }
    }

    fun clearLiveTxtEditorState() {
        uiState = recordUseCases.clearEditorState(uiState)
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
