package com.example.tracer

import java.time.Clock

class RecordUseCases(
    private val recordGateway: RecordGateway,
    private val txtStorageGateway: TxtStorageGateway,
    private val queryGateway: QueryGateway,
    private val clock: Clock = Clock.systemDefaultZone()
) {
    private val datePolicy = RecordUseCaseDatePolicy(clock)
    private val historyNavigator = RecordTxtHistoryNavigator(
        txtStorageGateway = txtStorageGateway,
        currentMonthKeyProvider = datePolicy::currentMonthKey
    )

    // This clock is the Android session's single source of truth for logical-day semantics.
    // Record, TXT preview, TXT day-marker defaults, and editor reset all must share this same
    // clock/zone so "today vs yesterday" stays consistent across tabs and tests do not inherit
    // an implicit host-machine time zone from scattered systemDefault() calls.
    internal val logicalDayClock: Clock
        get() = clock

    // Seed UI state from the injected logical-day clock instead of from RecordUiState defaults.
    // Keeping initialization here makes the time/zone dependency explicit and testable.
    internal fun initialUiState(): RecordUiState = RecordUiState(
        logicalDayTarget = datePolicy.defaultLogicalDayTarget()
    )

    suspend fun recordNow(state: RecordUiState): RecordUiState {
        val targetDateIso = datePolicy.resolveTargetDateIso(state.logicalDayTarget)
        val timeOrderMode = datePolicy.resolveRecordTimeOrderMode(state.logicalDayTarget)

        val result = recordGateway.recordNow(
            activityName = state.recordContent,
            remark = state.recordRemark,
            targetDateIso = targetDateIso,
            preferredTxtPath = state.selectedHistoryFile,
            timeOrderMode = timeOrderMode
        )
        val preferredMonth = datePolicy.resolvePreferredMonthForRecord(targetDateIso)
        // Clear the previous activity/remark after every record attempt. Otherwise users have to
        // manually delete the last input before typing the next activity, especially when a
        // failed insert leaves stale content in the fields.
        val stateAfterRecord = state.copy(
            recordContent = "",
            recordRemark = ""
        )
        return historyNavigator.refreshAndOpen(stateAfterRecord, preferredMonth, result.message)
    }

    suspend fun openTxtPreview(state: RecordUiState): RecordUiState {
        val targetDateIso = datePolicy.resolveTargetDateIso(state.logicalDayTarget)
        val preferredMonth = datePolicy.resolvePreferredMonthForRecord(targetDateIso)
        return historyNavigator.refreshAndOpen(
            state = state.copy(
                selectedMonth = "",
                selectedHistoryFile = "",
                selectedHistoryContent = "",
                editableHistoryContent = ""
            ),
            preferredMonth = preferredMonth,
            statusPrefix = "TXT preview refreshed."
        )
    }

    suspend fun refreshHistory(state: RecordUiState): RecordUiState {
        val preferredMonth = state.selectedMonth.ifEmpty { null }
        return historyNavigator.refreshAndOpen(state, preferredMonth, "TXT history refreshed.")
    }

    suspend fun openHistoryFile(
        state: RecordUiState,
        path: String,
        statusPrefixOverride: String? = null
    ): RecordUiState = historyNavigator.openHistoryFile(
        state = state,
        path = path,
        statusPrefixOverride = statusPrefixOverride
    )

    suspend fun openMonth(state: RecordUiState, month: String): RecordUiState =
        historyNavigator.openMonth(state, month)

    suspend fun openPreviousMonth(state: RecordUiState): RecordUiState =
        historyNavigator.openPreviousMonth(state)

    suspend fun openNextMonth(state: RecordUiState): RecordUiState =
        historyNavigator.openNextMonth(state)

    suspend fun saveHistoryFileAndSync(state: RecordUiState): RecordUiState =
        historyNavigator.saveHistoryFileAndSync(state)

    suspend fun createCurrentMonthTxt(state: RecordUiState): RecordUiState {
        val result = recordGateway.createCurrentMonthTxt()
        if (!result.ok) {
            return state.copy(statusText = result.message)
        }
        val currentMonth = datePolicy.currentMonthKey()
        return historyNavigator.refreshAndOpen(state, currentMonth, result.message)
    }

    suspend fun loadActivitySuggestions(
        state: RecordUiState,
        lookbackDays: Int = 7,
        topN: Int = 5
    ): RecordUiState {
        val result = queryGateway.queryActivitySuggestions(
            lookbackDays = lookbackDays,
            topN = topN
        )
        if (!result.ok) {
            return state.copy(
                suggestedActivities = emptyList(),
                isSuggestionsLoading = false,
                statusText = result.message
            )
        }

        return state.copy(
            suggestedActivities = result.suggestions,
            isSuggestionsLoading = false,
            statusText = result.message
        )
    }

    fun clearEditorState(state: RecordUiState): RecordUiState {
        return state.copy(
            recordContent = "",
            recordRemark = "",
            logicalDayTarget = datePolicy.defaultLogicalDayTarget(),
            logicalDayIsUserOverride = false,
            historyFiles = emptyList(),
            txtInspectionEntries = emptyList(),
            availableMonths = emptyList(),
            selectedMonth = "",
            selectedHistoryFile = "",
            selectedHistoryContent = "",
            editableHistoryContent = "",
            historyDraftsByFile = emptyMap(),
            suggestedActivities = emptyList(),
            suggestionsVisible = false,
            isSuggestionsLoading = false,
            statusText = "TXT editor state reset."
        )
    }
}
