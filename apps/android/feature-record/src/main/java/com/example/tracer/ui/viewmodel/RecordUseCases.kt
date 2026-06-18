package com.example.tracer

import java.time.Clock

class RecordUseCases(
    private val recordGateway: RecordGateway,
    private val txtStorageGateway: TxtStorageGateway,
    private val queryGateway: QueryGateway,
    internal val recordInputPersistence: RecordInputPersistence = NoOpRecordInputPersistence,
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
            preferredTxtPath = resolvePreferredTxtPathForRecord(
                targetDateIso = targetDateIso,
                selectedHistoryFile = state.selectedHistoryFile
            ),
            timeOrderMode = timeOrderMode
        )
        if (!result.ok) {
            return state.copy(statusText = result.message)
        }
        val preferredMonth = datePolicy.resolvePreferredMonthForRecord(targetDateIso)
        // Clear the previous activity/remark only after a successful write. Failed writes keep
        // the draft intact so users can retry without re-entering their in-progress input.
        val stateAfterRecord = state.copy(
            recordContent = "",
            recordRemark = ""
        )
        return historyNavigator.refreshAndOpen(stateAfterRecord, preferredMonth, result.message)
    }

    suspend fun recordInterval(state: RecordUiState): RecordUiState {
        val normalizedStart = state.intervalStart.trim()
        val normalizedEnd = state.intervalEnd.trim()
        if (state.recordContent.isBlank()) {
            return state.copy(statusText = "Record blocked: activity token is required.")
        }
        if (normalizedStart.isEmpty() || normalizedEnd.isEmpty()) {
            return state.copy(statusText = "Record blocked: start/end are required for interval mode.")
        }
        if (!isValidHhmm(normalizedStart) || !isValidHhmm(normalizedEnd)) {
            return state.copy(statusText = "Record blocked: start/end must use HHMM.")
        }

        val targetDateIso = datePolicy.resolveTargetDateIso(state.logicalDayTarget)
        val result = recordGateway.recordInterval(
            activityName = state.recordContent,
            startTime = normalizedStart,
            endTime = normalizedEnd,
            remark = state.recordRemark,
            targetDateIso = targetDateIso,
            preferredTxtPath = resolvePreferredTxtPathForRecord(
                targetDateIso = targetDateIso,
                selectedHistoryFile = state.selectedHistoryFile
            )
        )
        if (!result.ok) {
            return state.copy(statusText = result.message)
        }
        val preferredMonth = datePolicy.resolvePreferredMonthForRecord(targetDateIso)
        val stateAfterRecord = state.copy(
            recordContent = "",
            recordRemark = "",
            intervalStart = "",
            intervalEnd = ""
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
        val targetMonth = resolveRefreshTargetMonth(state)
        return if (targetMonth == datePolicy.currentMonthKey()) {
            openOrCreateMonth(
                state = state,
                month = targetMonth,
                statusPrefix = "TXT history refreshed."
            )
        } else {
            historyNavigator.refreshAndOpen(state, targetMonth, "TXT history refreshed.")
        }
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
        openOrCreateMonth(
            state = state,
            month = month,
            statusPrefix = "open month -> $month"
        )

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
            topN = topN,
            anchorDateIso = datePolicy.resolveTargetDateIso(state.logicalDayTarget)
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
            authoringMode = RecordAuthoringMode.POINT,
            recordContent = "",
            recordRemark = "",
            intervalStart = "",
            intervalEnd = "",
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

    private fun isValidHhmm(hhmm: String): Boolean {
        if (hhmm.length != 4 || !hhmm.all { it.isDigit() }) {
            return false
        }
        val hours = hhmm.substring(0, 2).toIntOrNull() ?: return false
        val minutes = hhmm.substring(2, 4).toIntOrNull() ?: return false
        return hours in 0..23 && minutes in 0..59
    }

    private suspend fun openOrCreateMonth(
        state: RecordUiState,
        month: String,
        statusPrefix: String
    ): RecordUiState {
        if (month.isBlank()) {
            return state
        }

        val existingOpen = historyNavigator.refreshAndOpenExistingMonth(
            state = state,
            month = month,
            statusPrefix = statusPrefix
        )
        if (existingOpen.found) {
            return existingOpen.state
        }

        val createResult = recordGateway.createMonthTxt(month)
        if (!createResult.ok) {
            return existingOpen.state.copy(statusText = createResult.message)
        }
        return historyNavigator.refreshAndOpen(
            state = existingOpen.state,
            preferredMonth = month,
            statusPrefix = createResult.message
        )
    }

    private fun resolveRefreshTargetMonth(state: RecordUiState): String {
        val currentMonth = datePolicy.currentMonthKey()
        val selectedMonth = state.selectedMonth
        if (selectedMonth.isBlank()) {
            return currentMonth
        }
        val latestKnownMonth = state.availableMonths.maxOrNull()
        return if (selectedMonth == latestKnownMonth && currentMonth > selectedMonth) {
            currentMonth
        } else {
            selectedMonth
        }
    }

    private fun resolvePreferredTxtPathForRecord(
        targetDateIso: String?,
        selectedHistoryFile: String
    ): String? {
        val preferredMonth = datePolicy.resolvePreferredMonthForRecord(targetDateIso)
        val canonicalPath = canonicalTxtPathForMonth(preferredMonth) ?: return null
        val normalizedSelectedPath = selectedHistoryFile.trim().replace('\\', '/')
        return normalizedSelectedPath.takeIf { it == canonicalPath }
    }

    private fun canonicalTxtPathForMonth(monthKey: String): String? {
        if (!MONTH_KEY_REGEX.matches(monthKey)) {
            return null
        }
        val year = monthKey.substring(0, 4)
        return "$year/$monthKey.txt"
    }

    private companion object {
        private val MONTH_KEY_REGEX = Regex("""\d{4}-\d{2}""")
    }
}
