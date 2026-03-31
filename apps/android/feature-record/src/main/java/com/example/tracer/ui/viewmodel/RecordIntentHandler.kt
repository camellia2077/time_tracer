package com.example.tracer

internal class RecordIntentHandler(
    private val useCaseCaller: RecordUseCaseCaller
) {
    fun onRecordContentChange(state: RecordUiState, value: String): RecordUiState =
        RecordStateReducer.onRecordContentChange(state, value)

    fun onRecordRemarkChange(state: RecordUiState, value: String): RecordUiState =
        RecordStateReducer.onRecordRemarkChange(state, value)

    fun selectLogicalDayYesterday(state: RecordUiState): RecordUiState =
        RecordStateReducer.selectLogicalDayYesterday(state)

    fun selectLogicalDayToday(state: RecordUiState): RecordUiState =
        RecordStateReducer.selectLogicalDayToday(state)

    fun refreshLogicalDayDefault(
        state: RecordUiState,
        currentTimeMillis: Long
    ): RecordUiState = RecordStateReducer.refreshLogicalDayDefault(
        state = state,
        currentTimeMillis = currentTimeMillis
    )

    fun updateEditableHistoryContent(state: RecordUiState, value: String): RecordUiState =
        RecordStateReducer.updateEditableHistoryContent(state, value)

    fun updateSuggestionPreferences(
        state: RecordUiState,
        lookbackDays: Int,
        topN: Int
    ): RecordUiState = RecordStateReducer.updateSuggestionPreferences(state, lookbackDays, topN)

    fun updateQuickActivities(state: RecordUiState, values: List<String>): RecordUiState =
        RecordStateReducer.updateQuickActivities(state, values)

    fun updateAssistUiState(
        state: RecordUiState,
        assistExpanded: Boolean,
        assistSettingsExpanded: Boolean
    ): RecordUiState = RecordStateReducer.updateAssistUiState(
        state = state,
        assistExpanded = assistExpanded,
        assistSettingsExpanded = assistSettingsExpanded
    )

    fun hideSuggestions(state: RecordUiState): RecordUiState =
        RecordStateReducer.hideSuggestions(state)

    fun showSuggestionsLoading(state: RecordUiState): RecordUiState =
        RecordStateReducer.showSuggestionsLoading(state)

    suspend fun loadActivitySuggestions(state: RecordUiState): RecordUiState =
        useCaseCaller.loadActivitySuggestions(
            state = state,
            lookbackDays = state.suggestionLookbackDays,
            topN = state.suggestionTopN
        )

    fun applySuggestedActivity(state: RecordUiState, activityName: String): RecordUiState =
        RecordStateReducer.applySuggestedActivity(state, activityName)

    fun setStatusText(state: RecordUiState, message: String): RecordUiState =
        RecordStateReducer.setStatusText(state, message)

    fun startCryptoProgress(state: RecordUiState, operationText: String): RecordUiState =
        RecordStateReducer.startCryptoProgress(state, operationText)

    fun updateCryptoProgress(
        state: RecordUiState,
        event: FileCryptoProgressEvent,
        operationTextOverride: String? = null,
        phaseTextOverride: String? = null,
        overallProgressOverride: Float? = null,
        overallTextOverride: String? = null,
        currentTextOverride: String? = null,
        currentProgressOverride: Float? = null
    ): RecordUiState = RecordStateReducer.updateCryptoProgress(
        state = state,
        event = event,
        operationTextOverride = operationTextOverride,
        phaseTextOverride = phaseTextOverride,
        overallProgressOverride = overallProgressOverride,
        overallTextOverride = overallTextOverride,
        currentTextOverride = currentTextOverride,
        currentProgressOverride = currentProgressOverride
    )

    fun finishCryptoProgress(
        state: RecordUiState,
        statusText: String,
        keepVisible: Boolean,
        detailsTextOverride: String? = null
    ): RecordUiState = RecordStateReducer.finishCryptoProgress(
        state = state,
        statusText = statusText,
        keepVisible = keepVisible,
        detailsTextOverride = detailsTextOverride
    )

    fun clearCryptoProgress(state: RecordUiState): RecordUiState =
        RecordStateReducer.clearCryptoProgress(state)

    suspend fun recordNow(state: RecordUiState): RecordUiState =
        useCaseCaller.recordNow(state)

    suspend fun refreshHistory(state: RecordUiState): RecordUiState =
        useCaseCaller.refreshHistory(state)

    suspend fun openHistoryFile(state: RecordUiState, path: String): RecordUiState =
        useCaseCaller.openHistoryFile(state, path)

    suspend fun openMonth(state: RecordUiState, month: String): RecordUiState =
        useCaseCaller.openMonth(state, month)

    suspend fun openPreviousMonth(state: RecordUiState): RecordUiState =
        useCaseCaller.openPreviousMonth(state)

    suspend fun openNextMonth(state: RecordUiState): RecordUiState =
        useCaseCaller.openNextMonth(state)

    suspend fun saveHistoryFileAndSync(state: RecordUiState): RecordUiState =
        useCaseCaller.saveHistoryFileAndSync(state)

    suspend fun createCurrentMonthTxt(state: RecordUiState): RecordUiState =
        useCaseCaller.createCurrentMonthTxt(state)

    fun discardUnsavedHistoryDraft(state: RecordUiState): RecordUiState =
        RecordStateReducer.discardUnsavedHistoryDraft(state)

    fun clearTxtEditorState(state: RecordUiState): RecordUiState =
        useCaseCaller.clearEditorState(state)
}
