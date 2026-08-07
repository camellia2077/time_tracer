package com.example.tracer

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import android.util.Log
import kotlinx.coroutines.Job
import kotlinx.coroutines.launch
import java.time.Clock
import java.time.ZonedDateTime
import java.time.format.DateTimeFormatter

enum class RecordLogicalDayTarget {
    YESTERDAY,
    TODAY
}

enum class RecordAuthoringMode {
    POINT,
    INTERVAL
}

enum class RecordSuggestionOutputMode {
    CANONICAL,
    ALIAS
}

private fun logDebug(tag: String, message: String) {
    runCatching { Log.d(tag, message) }
}

enum class CanonicalBrowserTarget {
    RECORD_INPUT,
    QUICK_ACCESS,
    REPORT_STATUS_PARENT
}

data class RecordSuggestedActivity(
    val canonicalToken: String,
    val aliasToken: String = ""
) {
    fun displayToken(outputMode: RecordSuggestionOutputMode): String =
        if (outputMode == RecordSuggestionOutputMode.ALIAS && aliasToken.isNotBlank()) {
            aliasToken
        } else {
            canonicalToken
        }
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
    val authoringMode: RecordAuthoringMode = RecordAuthoringMode.INTERVAL,
    val txtOutputMode: TxtOutputMode = TxtOutputMode.DAY,
    val recordContent: String = "",
    val recordRemark: String = "",
    val intervalStart: String = "",
    val intervalEnd: String = "",
    val intervalStartedAtEpochMs: Long = 0L,
    val attributionDateIso: String = "",
    // Keep the state object deterministic. The owning ViewModel seeds this from an injected
    // logical-day clock so tests do not inherit the host machine's default time-zone implicitly.
    val logicalDayTarget: RecordLogicalDayTarget = RecordLogicalDayTarget.TODAY,
    val logicalDayIsUserOverride: Boolean = false,
    val txtDayMarker: String = "",
    val txtHistoryLoaded: Boolean = false,
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
    val quickActivities: List<String> = emptyList(),
    val actualTimeExpanded: Boolean = false,
    // The card visibility and the activity-management controls are independent UI states.
    val quickAccessCardExpanded: Boolean = true,
    val assistSettingsExpanded: Boolean = false,
    val suggestionLookbackDays: Int = 7,
    val suggestionTopN: Int = 5,
    val suggestionOutputMode: RecordSuggestionOutputMode = RecordSuggestionOutputMode.CANONICAL,
    val canonicalCatalogDisplayMode: RecordSuggestionOutputMode =
        RecordSuggestionOutputMode.CANONICAL,
    val suggestedActivities: List<RecordSuggestedActivity> = emptyList(),
    val canonicalCatalogRoots: List<CanonicalPathNode> = emptyList(),
    val canonicalCatalogStatusText: String = "",
    val lastRecordedActivityHierarchyLeaf: String = "",
    val lastRecordedDuration: String = "",
    val collapsedCanonicalRootPaths: Set<String> = emptySet(),
    val orderedCanonicalRootPaths: List<String> = emptyList(),
    val suggestionsVisible: Boolean = false,
    val isCanonicalCatalogVisible: Boolean = false,
    val canonicalBrowserTarget: CanonicalBrowserTarget? = null,
    val isCanonicalCatalogLoading: Boolean = false,
    val isSuggestionsLoading: Boolean = false,
    val isTxtPreviewVisible: Boolean = false,
    val isTxtPreviewLoading: Boolean = false,
    val txtPreviewStatusText: String = "",
    val statusText: String = "",
    val cryptoProgress: CryptoProgressUiState = CryptoProgressUiState()
)

class RecordViewModel(private val recordUseCases: RecordUseCases) : ViewModel() {
    val logicalDayClock: Clock
        get() = recordUseCases.logicalDayClock

    private val intentHandler = RecordIntentHandler(
        useCaseCaller = RecordUseCaseCaller(recordUseCases),
        logicalDayZoneId = recordUseCases.logicalDayClock.zone
    )
    private var txtPreviewRequestVersion: Long = 0L
    private var canonicalCatalogLoadJob: Job? = null
    private var hasAppliedInitialPersistedRecordInput: Boolean = false

    val hasAppliedInitialPersistedRecordInputForUi: Boolean
        get() = hasAppliedInitialPersistedRecordInput

    var uiState by mutableStateOf(recordUseCases.initialUiState())
        private set

    private val txtNavigationCoordinator = TxtNavigationCoordinator(
        scope = viewModelScope,
        stateProvider = { uiState },
        stateConsumer = { nextState ->
            uiState = nextState.copy(txtHistoryLoaded = true)
            logDebug(
                TXT_TAB_LOG_TAG,
                "history load complete inspectionCount=${uiState.txtInspectionEntries.size} " +
                    "selectedFile=${uiState.selectedHistoryFile} selectedMonth=${uiState.selectedMonth} " +
                    "historyLoaded=${uiState.txtHistoryLoaded}"
            )
        },
        navigate = { state, request ->
            when (request) {
                TxtNavigationRequest.Refresh -> intentHandler.refreshHistory(state)
                is TxtNavigationRequest.OpenFile -> intentHandler.openHistoryFile(state, request.path)
                is TxtNavigationRequest.OpenMonth -> intentHandler.openMonth(state, request.month)
                TxtNavigationRequest.PreviousMonth -> intentHandler.openPreviousMonth(state)
                TxtNavigationRequest.NextMonth -> intentHandler.openNextMonth(state)
            }
        }
    )

    fun onRecordContentChange(value: String) {
        uiState = intentHandler.onRecordContentChange(uiState, value)
        persistRecordInputState()
    }

    fun onAuthoringModeChange(value: RecordAuthoringMode) {
        uiState = intentHandler.onAuthoringModeChange(uiState, value)
        persistRecordInputState()
    }

    fun onRecordRemarkChange(value: String) {
        uiState = intentHandler.onRecordRemarkChange(uiState, value)
        persistRecordInputState()
    }

    fun onIntervalStartChange(value: String) {
        uiState = intentHandler.onIntervalStartChange(uiState, value)
        persistRecordInputState()
    }

    fun onIntervalEndChange(value: String) {
        uiState = intentHandler.onIntervalEndChange(uiState, value)
        persistRecordInputState()
    }

    fun onTxtOutputModeChange(value: TxtOutputMode) {
        uiState = uiState.copy(txtOutputMode = value)
        persistRecordInputState()
    }

    fun onTxtDayMarkerChange(value: String) {
        uiState = uiState.copy(txtDayMarker = value.filter { it.isDigit() }.take(4))
    }

    fun startIntervalRecording() {
        uiState = uiState.copy(
            intervalStart = currentHhmmss(),
            intervalEnd = "",
            intervalStartedAtEpochMs = logicalDayClock.millis(),
            attributionDateIso = resolveLogicalDayTargetDate(
                uiState.logicalDayTarget,
                logicalDayClock
            ).toString()
        )
        persistRecordInputState()
    }

    fun stopIntervalRecording() {
        uiState = uiState.copy(intervalEnd = currentHhmmss())
        persistRecordInputState()
    }

    fun discardIntervalDraft() {
        uiState = uiState.copy(
            intervalStart = "",
            intervalEnd = "",
            intervalStartedAtEpochMs = 0L,
            attributionDateIso = ""
        )
        persistRecordInputState()
    }

    fun hydratePersistedRecordInput(persistedInput: PersistedRecordInputSnapshot) {
        if (hasAppliedInitialPersistedRecordInput) {
            return
        }
        if (hasLocalRecordInputEdits(uiState)) {
            hasAppliedInitialPersistedRecordInput = true
            return
        }
        uiState = intentHandler.hydratePersistedRecordInput(uiState, persistedInput).let { hydrated ->
            val draft = persistedInput.draft
            if (
                hydrated.attributionDateIso.isBlank() &&
                draft != null &&
                draft.intervalStartedAtEpochMs > 0L
            ) {
                hydrated.copy(
                    attributionDateIso = resolveLogicalDayDateForInstant(
                        draft.intervalStartedAtEpochMs,
                        logicalDayClock.zone
                    ).toString()
                )
            } else {
                hydrated
            }
        }
        hasAppliedInitialPersistedRecordInput = true
        persistRecordInputState()
    }

    fun selectLogicalDayYesterday() {
        uiState = intentHandler.selectLogicalDayYesterday(uiState)
        persistRecordInputState()
    }

    fun selectLogicalDayToday() {
        uiState = intentHandler.selectLogicalDayToday(uiState)
        persistRecordInputState()
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

    suspend fun saveHistoryFileRepresentationOnly(content: String): TxtFileContentResult {
        val outcome = recordUseCases.saveHistoryFileRepresentationOnly(uiState, content)
        uiState = outcome.state
        return outcome.result
    }

    fun updateSuggestionPreferences(lookbackDays: Int, topN: Int) {
        uiState = intentHandler.updateSuggestionPreferences(
            state = uiState,
            lookbackDays = lookbackDays,
            topN = topN
        )
    }

    fun updateSuggestionPreferencesAndReloadIfVisible(lookbackDays: Int, topN: Int) {
        uiState = intentHandler.updateSuggestionPreferences(
            state = uiState,
            lookbackDays = lookbackDays,
            topN = topN
        )
        if (!uiState.suggestionsVisible) {
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

    fun updateSuggestionOutputMode(value: RecordSuggestionOutputMode) {
        uiState = intentHandler.updateSuggestionOutputMode(uiState, value)
    }

    fun updateCanonicalCatalogDisplayMode(value: RecordSuggestionOutputMode) {
        uiState = intentHandler.updateCanonicalCatalogDisplayMode(uiState, value)
    }

    fun updateQuickActivities(values: List<String>) {
        uiState = intentHandler.updateQuickActivities(uiState, values)
    }

    fun updateActualTimeExpanded(expanded: Boolean) {
        uiState = intentHandler.updateActualTimeExpanded(uiState, expanded)
    }

    fun updateQuickAccessCardExpanded(expanded: Boolean) {
        uiState = intentHandler.updateQuickAccessCardExpanded(uiState, expanded)
    }

    fun updateAssistUiState(assistSettingsExpanded: Boolean) {
        uiState = intentHandler.updateAssistUiState(
            state = uiState,
            assistSettingsExpanded = assistSettingsExpanded
        )
    }

    fun updateCollapsedCanonicalRootPaths(paths: Set<String>) {
        uiState = intentHandler.updateCollapsedCanonicalRootPaths(uiState, paths)
    }

    fun updateOrderedCanonicalRootPaths(paths: List<String>) {
        uiState = intentHandler.updateOrderedCanonicalRootPaths(uiState, paths)
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

    fun dismissSuggestions() {
        uiState = intentHandler.hideSuggestions(uiState)
    }

    fun applySuggestedActivity(activityName: String) {
        uiState = intentHandler.hideSuggestions(uiState)
        viewModelScope.launch {
            uiState = intentHandler.applySuggestedActivity(uiState, activityName)
            persistRecordInputState()
        }
    }

    fun openCanonicalCatalog() {
        openCanonicalCatalog(CanonicalBrowserTarget.RECORD_INPUT)
    }

    fun openQuickAccessCanonicalCatalog() {
        openCanonicalCatalog(CanonicalBrowserTarget.QUICK_ACCESS)
    }

    fun openDailyStatusParentCatalog() {
        openCanonicalCatalog(CanonicalBrowserTarget.REPORT_STATUS_PARENT)
    }

    private fun openCanonicalCatalog(target: CanonicalBrowserTarget) {
        canonicalCatalogLoadJob?.cancel()
        uiState = intentHandler.showCanonicalCatalogLoading(uiState, target)
        canonicalCatalogLoadJob = viewModelScope.launch {
            uiState = intentHandler.loadCanonicalCatalog(uiState)
        }
    }

    fun dismissCanonicalCatalog() {
        canonicalCatalogLoadJob?.cancel()
        canonicalCatalogLoadJob = null
        uiState = intentHandler.hideCanonicalCatalog(uiState)
    }

    fun applyCanonicalCatalogEntry(token: String) {
        canonicalCatalogLoadJob?.cancel()
        canonicalCatalogLoadJob = null
        uiState = intentHandler.applyCanonicalCatalogEntry(uiState, token)
        persistRecordInputState()
    }

    fun setStatusText(message: String) {
        uiState = intentHandler.setStatusText(uiState, message)
    }

    fun startCryptoProgress(operationText: String) {
        uiState = intentHandler.startCryptoProgress(uiState, operationText)
    }

    fun updateCryptoProgress(
        event: TracerExchangeProgressEvent,
        operationTextOverride: String? = null,
        phaseTextOverride: String? = null,
        overallProgressOverride: Float? = null,
        overallTextOverride: String? = null
    ) {
        uiState = intentHandler.updateCryptoProgress(
            state = uiState,
            event = event,
            operationTextOverride = operationTextOverride,
            phaseTextOverride = phaseTextOverride,
            overallProgressOverride = overallProgressOverride,
            overallTextOverride = overallTextOverride
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
        Log.i(
            "TimeTracerRecord",
            "ui.record_now.click activity=${uiState.recordContent.trim()} " +
                "logicalDayTarget=${uiState.logicalDayTarget} attributionDate=${uiState.attributionDateIso}"
        )
        viewModelScope.launch {
            uiState = intentHandler.recordNow(uiState)
            Log.i(
                "TimeTracerRecord",
                "ui.record_now.result status=${uiState.statusText.lineSequence().firstOrNull()}"
            )
            persistRecordInputState()
        }
    }

    fun recordInterval() {
        Log.i(
            "TimeTracerRecord",
            "ui.record_interval.click activity=${uiState.recordContent.trim()} " +
                "start=${uiState.intervalStart} end=${uiState.intervalEnd}"
        )
        viewModelScope.launch {
            uiState = intentHandler.recordInterval(uiState)
            Log.i(
                "TimeTracerRecord",
                "ui.record_interval.result status=${uiState.statusText.lineSequence().firstOrNull()}"
            )
            persistRecordInputState()
        }
    }

    private fun currentHhmmss(): String =
        ZonedDateTime.now(logicalDayClock).format(HHMMSS_FORMATTER)

    private companion object {
        private const val TXT_TAB_LOG_TAG = "TxtTab"
        private val HHMMSS_FORMATTER: DateTimeFormatter = DateTimeFormatter.ofPattern("HHmmss")
    }

    fun refreshHistory(): Job {
        logDebug(
            TXT_TAB_LOG_TAG,
            "history load start inspectionCount=${uiState.txtInspectionEntries.size} " +
                "selectedFile=${uiState.selectedHistoryFile} selectedMonth=${uiState.selectedMonth}"
        )
        uiState = uiState.copy(txtHistoryLoaded = false)
        return txtNavigationCoordinator.launch(TxtNavigationRequest.Refresh)
    }

    fun openHistoryFile(path: String) {
        txtNavigationCoordinator.launch(TxtNavigationRequest.OpenFile(path))
    }

    fun openMonth(month: String) {
        txtNavigationCoordinator.launch(TxtNavigationRequest.OpenMonth(month))
    }

    fun openPreviousMonth() {
        txtNavigationCoordinator.launch(TxtNavigationRequest.PreviousMonth)
    }

    fun openNextMonth() {
        txtNavigationCoordinator.launch(TxtNavigationRequest.NextMonth)
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
        persistRecordInputState()
    }

    private fun hasLocalRecordInputEdits(state: RecordUiState): Boolean {
        return state.authoringMode != RecordAuthoringMode.INTERVAL ||
            // DAY is the first-open default. Do not treat that default as a local edit,
            // otherwise cold-start hydration is skipped and a persisted ALL selection is lost.
            state.txtOutputMode != TxtOutputMode.DAY ||
            RecordStateReducer.hasPersistableRecordDraft(state) ||
            state.logicalDayIsUserOverride
    }

    private fun persistRecordInputState() {
        if (recordUseCases.recordInputPersistence === NoOpRecordInputPersistence) {
            return
        }
        val snapshot = uiState
        viewModelScope.launch {
            recordUseCases.recordInputPersistence.persistLastAuthoringMode(snapshot.authoringMode)
            recordUseCases.recordInputPersistence.persistLastTxtOutputMode(snapshot.txtOutputMode)
            val draft = if (RecordStateReducer.hasPersistableRecordDraft(snapshot)) {
                PersistedRecordInputDraft(
                    recordContent = snapshot.recordContent,
                    recordRemark = snapshot.recordRemark,
                    intervalStart = snapshot.intervalStart,
                    intervalEnd = snapshot.intervalEnd,
                    intervalStartedAtEpochMs = snapshot.intervalStartedAtEpochMs,
                    attributionDateIso = snapshot.attributionDateIso,
                    logicalDayTarget = snapshot.logicalDayTarget
                )
            } else {
                null
            }
            if (draft != null) {
                recordUseCases.recordInputPersistence.persistDraft(draft)
            } else {
                recordUseCases.recordInputPersistence.clearDraft()
            }
        }
    }

}

class RecordViewModelFactory(
    private val recordGateway: RecordGateway,
    private val txtStorageGateway: TxtStorageGateway,
    private val queryGateway: QueryGateway,
    private val reportGateway: ReportGateway = UnavailableRecordReportGateway,
    private val initialPersistedRecordInput: PersistedRecordInputSnapshot = PersistedRecordInputSnapshot(),
    private val recordInputPersistence: RecordInputPersistence = NoOpRecordInputPersistence,
    private val textProvider: RecordTextProvider = DefaultRecordTextProvider,
    private val clock: Clock = Clock.systemDefaultZone()
) : ViewModelProvider.Factory {
    override fun <T : ViewModel> create(modelClass: Class<T>): T {
        if (modelClass.isAssignableFrom(RecordViewModel::class.java)) {
            @Suppress("UNCHECKED_CAST")
            return RecordViewModel(
                RecordUseCases(
                    recordGateway = recordGateway,
                    txtStorageGateway = txtStorageGateway,
                    queryGateway = queryGateway,
                    reportGateway = reportGateway,
                    recordInputPersistence = recordInputPersistence,
                    textProvider = textProvider,
                    clock = clock
                )
            ).also {
                if (initialPersistedRecordInput != PersistedRecordInputSnapshot()) {
                    it.hydratePersistedRecordInput(initialPersistedRecordInput)
                }
            } as T
        }
        throw IllegalArgumentException("Unknown ViewModel class: ${modelClass.name}")
    }
}
