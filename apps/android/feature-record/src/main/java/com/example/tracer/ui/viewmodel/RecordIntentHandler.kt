package com.example.tracer

import java.time.ZoneId

internal class RecordIntentHandler(
    private val useCaseCaller: RecordUseCaseCaller,
    private val logicalDayZoneId: ZoneId
) {
    fun hydratePersistedRecordInput(
        state: RecordUiState,
        persistedInput: PersistedRecordInputSnapshot
    ): RecordUiState = RecordStateReducer.hydratePersistedRecordInput(state, persistedInput)

    fun onAuthoringModeChange(state: RecordUiState, value: RecordAuthoringMode): RecordUiState =
        RecordStateReducer.onAuthoringModeChange(state, value)

    fun onRecordContentChange(state: RecordUiState, value: String): RecordUiState =
        RecordStateReducer.onRecordContentChange(state, value)

    fun onRecordRemarkChange(state: RecordUiState, value: String): RecordUiState =
        RecordStateReducer.onRecordRemarkChange(state, value)

    fun onIntervalStartChange(state: RecordUiState, value: String): RecordUiState =
        RecordStateReducer.onIntervalStartChange(state, value)

    fun onIntervalEndChange(state: RecordUiState, value: String): RecordUiState =
        RecordStateReducer.onIntervalEndChange(state, value)

    fun selectLogicalDayYesterday(state: RecordUiState): RecordUiState =
        RecordStateReducer.selectLogicalDayYesterday(state)

    fun selectLogicalDayToday(state: RecordUiState): RecordUiState =
        RecordStateReducer.selectLogicalDayToday(state)

    fun refreshLogicalDayDefault(
        state: RecordUiState,
        currentTimeMillis: Long
    ): RecordUiState = RecordStateReducer.refreshLogicalDayDefault(
        state = state,
        currentTimeMillis = currentTimeMillis,
        logicalDayZoneId = logicalDayZoneId
    )

    fun updateEditableHistoryContent(state: RecordUiState, value: String): RecordUiState =
        RecordStateReducer.updateEditableHistoryContent(state, value)

    fun updateSuggestionPreferences(
        state: RecordUiState,
        lookbackDays: Int,
        topN: Int
    ): RecordUiState = RecordStateReducer.updateSuggestionPreferences(state, lookbackDays, topN)

    fun updateSuggestionOutputMode(
        state: RecordUiState,
        value: RecordSuggestionOutputMode
    ): RecordUiState = RecordStateReducer.updateSuggestionOutputMode(state, value)

    fun updateCanonicalCatalogDisplayMode(
        state: RecordUiState,
        value: RecordSuggestionOutputMode
    ): RecordUiState = RecordStateReducer.updateCanonicalCatalogDisplayMode(state, value)

    fun updateQuickActivities(state: RecordUiState, values: List<String>): RecordUiState =
        RecordStateReducer.updateQuickActivities(state, values)

    fun updateActualTimeExpanded(
        state: RecordUiState,
        expanded: Boolean
    ): RecordUiState = RecordStateReducer.updateActualTimeExpanded(state, expanded)

    fun updateQuickAccessCardExpanded(
        state: RecordUiState,
        expanded: Boolean
    ): RecordUiState = RecordStateReducer.updateQuickAccessCardExpanded(state, expanded)

    fun updateAssistUiState(
        state: RecordUiState,
        assistSettingsExpanded: Boolean
    ): RecordUiState = RecordStateReducer.updateAssistUiState(
        state = state,
        assistSettingsExpanded = assistSettingsExpanded
    )

    fun updateCollapsedCanonicalRootPaths(
        state: RecordUiState,
        paths: Set<String>
    ): RecordUiState = RecordStateReducer.updateCollapsedCanonicalRootPaths(state, paths)

    fun updateOrderedCanonicalRootPaths(
        state: RecordUiState,
        paths: List<String>
    ): RecordUiState = RecordStateReducer.updateOrderedCanonicalRootPaths(state, paths)

    fun hideSuggestions(state: RecordUiState): RecordUiState =
        RecordStateReducer.hideSuggestions(state)

    fun showSuggestionsLoading(state: RecordUiState): RecordUiState =
        RecordStateReducer.showSuggestionsLoading(state)

    fun hideCanonicalCatalog(state: RecordUiState): RecordUiState =
        RecordStateReducer.hideCanonicalCatalog(state)

    fun showCanonicalCatalogLoading(
        state: RecordUiState,
        target: CanonicalBrowserTarget
    ): RecordUiState = RecordStateReducer.showCanonicalCatalogLoading(state, target)

    suspend fun loadActivitySuggestions(state: RecordUiState): RecordUiState =
        useCaseCaller.loadActivitySuggestions(
            state = state,
            lookbackDays = state.suggestionLookbackDays,
            topN = state.suggestionTopN
        )

    suspend fun loadCanonicalCatalog(state: RecordUiState): RecordUiState =
        useCaseCaller.loadCanonicalCatalog(state)

    suspend fun applySuggestedActivity(
        state: RecordUiState,
        suggestedActivityToken: String
    ): RecordUiState = useCaseCaller.applySuggestedActivity(
        state = state,
        suggestedActivityToken = suggestedActivityToken
    )

    fun applyCanonicalCatalogEntry(
        state: RecordUiState,
        token: String
    ): RecordUiState = RecordStateReducer.applyCanonicalCatalogEntry(state, token)

    fun setStatusText(state: RecordUiState, message: String): RecordUiState =
        RecordStateReducer.setStatusText(state, message)

    fun startCryptoProgress(state: RecordUiState, operationText: String): RecordUiState =
        RecordStateReducer.startCryptoProgress(state, operationText)

    fun updateCryptoProgress(
        state: RecordUiState,
        event: TracerExchangeProgressEvent,
        operationTextOverride: String? = null,
        phaseTextOverride: String? = null,
        overallProgressOverride: Float? = null,
        overallTextOverride: String? = null
    ): RecordUiState = RecordStateReducer.updateCryptoProgress(
        state = state,
        event = event,
        operationTextOverride = operationTextOverride,
        phaseTextOverride = phaseTextOverride,
        overallProgressOverride = overallProgressOverride,
        overallTextOverride = overallTextOverride
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

    fun showTxtPreviewLoading(state: RecordUiState): RecordUiState =
        RecordStateReducer.showTxtPreviewLoading(state)

    fun dismissTxtPreview(state: RecordUiState): RecordUiState =
        RecordStateReducer.dismissTxtPreview(state)

    suspend fun openTxtPreview(state: RecordUiState): RecordUiState =
        useCaseCaller.openTxtPreview(state)

    suspend fun recordNow(state: RecordUiState): RecordUiState =
        useCaseCaller.recordNow(state)

    suspend fun recordInterval(state: RecordUiState): RecordUiState =
        useCaseCaller.recordInterval(state)

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
