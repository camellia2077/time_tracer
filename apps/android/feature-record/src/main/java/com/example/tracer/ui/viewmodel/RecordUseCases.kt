package com.example.tracer

import android.util.Log
import java.time.Clock

class RecordUseCases(
    private val recordGateway: RecordGateway,
    private val txtStorageGateway: TxtStorageGateway,
    private val queryGateway: QueryGateway,
    private val reportGateway: ReportGateway = UnavailableRecordReportGateway,
    internal val recordInputPersistence: RecordInputPersistence = NoOpRecordInputPersistence,
    private val textProvider: RecordTextProvider = DefaultRecordTextProvider,
    private val clock: Clock = Clock.systemDefaultZone()
) {
    data class TxtRepresentationSaveOutcome(
        val state: RecordUiState,
        val result: TxtFileContentResult
    )

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
        val durationClockText = queryDatabaseDurationText(
            targetDateIso = targetDateIso,
            rawActivityToken = state.recordContent
        )
        val successSummary = buildRecordSuccessSummary(
            rawActivityToken = state.recordContent,
            durationClockText = durationClockText
        )
        val preferredMonth = datePolicy.resolvePreferredMonthForRecord(targetDateIso)
        // Clear the previous activity/remark only after a successful write. Failed writes keep
        // the draft intact so users can retry without re-entering their in-progress input.
        val stateAfterRecord = state.copy(
            recordContent = "",
            recordRemark = "",
            lastRecordedActivityAlias = successSummary.aliasToken,
            lastRecordedDuration = successSummary.inputDurationText
        )
        return historyNavigator.refreshAndOpen(
            stateAfterRecord,
            preferredMonth,
            successSummary.statusText
        )
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
        if (!isValidTime(normalizedStart) || !isValidTime(normalizedEnd)) {
            return state.copy(statusText = "Record blocked: start/end must use HH MM SS.")
        }

        val targetDateIso = state.attributionDateIso.ifBlank {
            datePolicy.resolveTargetDateIso(state.logicalDayTarget)
        }
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
        val durationClockText = queryDatabaseDurationText(
            targetDateIso = targetDateIso,
            rawActivityToken = state.recordContent
        )
        val successSummary = buildRecordSuccessSummary(
            rawActivityToken = state.recordContent,
            durationClockText = durationClockText
        )
        val preferredMonth = datePolicy.resolvePreferredMonthForRecord(targetDateIso)
        val stateAfterRecord = state.copy(
            recordContent = "",
            recordRemark = "",
            intervalStart = "",
            intervalEnd = "",
            intervalStartedAtEpochMs = 0L,
            attributionDateIso = "",
            lastRecordedActivityAlias = successSummary.aliasToken,
            lastRecordedDuration = successSummary.inputDurationText
        )
        return historyNavigator.refreshAndOpen(
            stateAfterRecord,
            preferredMonth,
            successSummary.statusText
        )
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

    suspend fun saveHistoryFileRepresentationOnly(
        state: RecordUiState,
        content: String
    ): TxtRepresentationSaveOutcome {
        val selectedFile = state.selectedHistoryFile
        if (selectedFile.isBlank()) {
            return TxtRepresentationSaveOutcome(
                state = state,
                result = TxtFileContentResult(
                    ok = false,
                    filePath = selectedFile,
                    content = content,
                    message = "No TXT file selected."
                )
            )
        }

        val result = txtStorageGateway.saveTxtFile(
            relativePath = selectedFile,
            content = content
        )
        if (!result.ok) {
            return TxtRepresentationSaveOutcome(state = state, result = result)
        }

        val persistedContent = result.content.ifBlank { content }
        val nextDrafts = state.historyDraftsByFile.toMutableMap()
        nextDrafts.remove(selectedFile)
        return TxtRepresentationSaveOutcome(
            state = state.copy(
                selectedHistoryContent = persistedContent,
                editableHistoryContent = persistedContent,
                historyDraftsByFile = nextDrafts,
                statusText = result.message
            ),
            result = result.copy(content = persistedContent)
        )
    }

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
        val anchorDateIso = datePolicy.resolveTargetDateIso(state.logicalDayTarget)
        logActivitySuggestionsRequestStart(
            logicalDayTarget = state.logicalDayTarget,
            lookbackDays = lookbackDays,
            topN = topN,
            anchorDateIso = anchorDateIso
        )
        val result = queryGateway.queryActivitySuggestions(
            lookbackDays = lookbackDays,
            topN = topN,
            anchorDateIso = anchorDateIso
        )
        logActivitySuggestionsRequestResult(
            logicalDayTarget = state.logicalDayTarget,
            lookbackDays = lookbackDays,
            topN = topN,
            anchorDateIso = anchorDateIso,
            result = result
        )
        if (!result.ok) {
            return state.copy(
                suggestedActivities = emptyList(),
                isSuggestionsLoading = false,
                statusText = result.message
            )
        }
        val aliasMappingsResult = queryGateway.listActivityAliasMappings()
        val aliasByCanonical = if (aliasMappingsResult.ok) {
            aliasMappingsResult.entries.firstAliasByCanonical()
        } else {
            emptyMap()
        }

        return state.copy(
            suggestedActivities = result.suggestions.mapNotNull { canonicalToken ->
                val trimmedCanonical = canonicalToken.trim()
                if (trimmedCanonical.isEmpty()) {
                    null
                } else {
                    RecordSuggestedActivity(
                        canonicalToken = trimmedCanonical,
                        aliasToken = aliasByCanonical[trimmedCanonical].orEmpty()
                    )
                }
            },
            isSuggestionsLoading = false,
            statusText = result.message
        )
    }

    suspend fun loadCanonicalCatalog(state: RecordUiState): RecordUiState {
        val canonicalCatalogResult = queryGateway.listCanonicalCatalog()
        return if (canonicalCatalogResult.ok) {
            state.copy(
                canonicalCatalogRoots = canonicalCatalogResult.roots,
                canonicalCatalogStatusText = "",
                isCanonicalCatalogVisible = true,
                isCanonicalCatalogLoading = false
            )
        } else {
            state.copy(
                canonicalCatalogRoots = emptyList(),
                canonicalCatalogStatusText = canonicalCatalogResult.message,
                isCanonicalCatalogVisible = true,
                isCanonicalCatalogLoading = false
            )
        }
    }

    suspend fun applySuggestedActivity(
        state: RecordUiState,
        suggestedActivityToken: String
    ): RecordUiState {
        val trimmedToken = suggestedActivityToken.trim()
        val matchedSuggestion = state.suggestedActivities.firstOrNull { suggestion ->
            suggestion.canonicalToken == trimmedToken || suggestion.aliasToken == trimmedToken
        }
        val trimmedCanonical = matchedSuggestion?.canonicalToken ?: trimmedToken
        if (trimmedCanonical.isEmpty()) {
            logSuggestedActivityApply(
                canonicalActivityName = suggestedActivityToken,
                outputMode = state.suggestionOutputMode,
                appliedToken = null,
                status = "ignored blank canonical suggestion."
            )
            return state.copy(suggestionsVisible = false)
        }

        if (state.suggestionOutputMode == RecordSuggestionOutputMode.CANONICAL) {
            logSuggestedActivityApply(
                canonicalActivityName = trimmedCanonical,
                outputMode = state.suggestionOutputMode,
                appliedToken = trimmedCanonical,
                status = "applied canonical suggested activity."
            )
            return state.copy(
                recordContent = trimmedCanonical,
                suggestionsVisible = false,
                statusText = ""
            )
        }

        val cachedAlias = matchedSuggestion?.aliasToken?.trim().orEmpty()
        if (cachedAlias.isNotEmpty()) {
            logSuggestedActivityApply(
                canonicalActivityName = trimmedCanonical,
                outputMode = state.suggestionOutputMode,
                appliedToken = cachedAlias,
                status = "applied suggested activity alias."
            )
            return state.copy(
                recordContent = cachedAlias,
                suggestionsVisible = false,
                statusText = ""
            )
        }

        val mappingResult = queryGateway.listActivityAliasMappings()
        if (!mappingResult.ok) {
            logSuggestedActivityApply(
                canonicalActivityName = trimmedCanonical,
                outputMode = state.suggestionOutputMode,
                appliedToken = null,
                status = mappingResult.message
            )
            return state.copy(
                suggestionsVisible = false,
                statusText = mappingResult.message
            )
        }

        val resolvedAlias = mappingResult.entries
            .firstOrNull { it.canonical == trimmedCanonical }
            ?.alias
            ?.trim()
            .orEmpty()
        if (resolvedAlias.isEmpty()) {
            val message =
                "Suggested activity unavailable for authoring: no alias mapped for $trimmedCanonical."
            logSuggestedActivityApply(
                canonicalActivityName = trimmedCanonical,
                outputMode = state.suggestionOutputMode,
                appliedToken = null,
                status = message
            )
            return state.copy(
                suggestionsVisible = false,
                statusText = message
            )
        }

        logSuggestedActivityApply(
            canonicalActivityName = trimmedCanonical,
            outputMode = state.suggestionOutputMode,
            appliedToken = resolvedAlias,
            status = "applied suggested activity alias."
        )
        return state.copy(
            recordContent = resolvedAlias,
            suggestionsVisible = false,
            statusText = ""
        )
    }

    fun clearEditorState(state: RecordUiState): RecordUiState {
        return state.copy(
            authoringMode = RecordAuthoringMode.POINT,
            recordContent = "",
            recordRemark = "",
            intervalStart = "",
            intervalEnd = "",
            intervalStartedAtEpochMs = 0L,
            attributionDateIso = "",
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
            canonicalCatalogRoots = emptyList(),
            canonicalCatalogStatusText = "",
            lastRecordedActivityAlias = "",
            lastRecordedDuration = "",
            suggestionsVisible = false,
            isCanonicalCatalogVisible = false,
            canonicalBrowserTarget = null,
            isCanonicalCatalogLoading = false,
            isSuggestionsLoading = false,
            statusText = "TXT editor state reset."
        )
    }

    private fun isValidTime(value: String): Boolean {
        if ((value.length != 4 && value.length != 6) || !value.all { it.isDigit() }) {
            return false
        }
        val hours = value.substring(0, 2).toIntOrNull() ?: return false
        val minutes = value.substring(2, 4).toIntOrNull() ?: return false
        val seconds = if (value.length == 6) {
            value.substring(4, 6).toIntOrNull() ?: return false
        } else {
            0
        }
        return hours in 0..23 && minutes in 0..59 && seconds in 0..59
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

    private suspend fun buildRecordSuccessSummary(
        rawActivityToken: String,
        durationClockText: String
    ): RecordSuccessSummary {
        val tokenSummary = resolveActivityTokenSummary(rawActivityToken)
        val statusDurationText = formatClockDuration(durationClockText) ?: durationClockText
        // Record success is surfaced in two places: an inline summary card and a snackbar.
        // Keep the snackbar payload as a logical two-line string here so the app layer can
        // either render it structurally (preferred) or still fall back to newline-aware text.
        return RecordSuccessSummary(
            canonicalToken = tokenSummary.canonicalToken,
            aliasToken = tokenSummary.aliasToken,
            inputDurationText = durationClockText,
            statusText = textProvider.recordedActivityStatus(
                canonicalToken = tokenSummary.canonicalToken,
                durationText = statusDurationText
            )
        )
    }

    private suspend fun resolveActivityTokenSummary(rawActivityToken: String): ActivityTokenSummary {
        val trimmedToken = rawActivityToken.trim()
        if (trimmedToken.isEmpty()) {
            return ActivityTokenSummary(canonicalToken = trimmedToken, aliasToken = trimmedToken)
        }
        val mappingResult = queryGateway.listActivityAliasMappings()
        if (!mappingResult.ok) {
            return ActivityTokenSummary(canonicalToken = trimmedToken, aliasToken = trimmedToken)
        }
        val aliasMatch = mappingResult.entries.firstOrNull { entry ->
            entry.alias.trim() == trimmedToken
        }
        if (aliasMatch != null) {
            return ActivityTokenSummary(
                canonicalToken = aliasMatch.canonical.trim().ifEmpty { trimmedToken },
                aliasToken = aliasMatch.alias.trim().ifEmpty { trimmedToken }
            )
        }

        val canonicalMatch = mappingResult.entries.firstOrNull { entry ->
            entry.canonical.trim() == trimmedToken
        }
        return ActivityTokenSummary(
            canonicalToken = trimmedToken,
            aliasToken = canonicalMatch?.alias?.trim()?.takeIf { it.isNotEmpty() } ?: trimmedToken
        )
    }

    private suspend fun queryDatabaseDurationText(
        targetDateIso: String,
        rawActivityToken: String
    ): String {
        val tokenSummary = resolveActivityTokenSummary(rawActivityToken)
        val reportResult = runCatching {
            reportGateway.reportStructured(
                TemporalReportQueryRequest(
                    displayMode = ReportDisplayMode.DAY,
                    selection = TemporalSelectionPayload(
                        kind = TemporalSelectionKind.SINGLE_DAY,
                        date = targetDateIso
                    )
                )
            )
        }.getOrNull()
        val report = reportResult?.report
        if (reportResult?.operationOk != true || report == null) {
            return textProvider.unavailableDuration()
        }

        val wakeKeywords = runCatching {
            queryGateway.listWakeKeywords()
        }.getOrNull()
        val isWakeKeyword = wakeKeywords?.ok == true && wakeKeywords.names.any {
            it.trim().equals(rawActivityToken.trim(), ignoreCase = true)
        }
        val candidateNames = if (isWakeKeyword) {
            // Wake is a point event; the user-facing duration is the inferred
            // sleep activity created by Core, not the wake token itself.
            listOf("sleep_night")
        } else {
            listOf(tokenSummary.canonicalToken.trim())
        }
        val durationSeconds = report.activities
            .asSequence()
            .filter { activity -> activity.activityName.trim() in candidateNames }
            .maxByOrNull { it.logicalId }
            ?.durationSeconds
            ?.takeIf { it > 0L }
            ?: return textProvider.unavailableDuration()
        return formatDurationClock(durationSeconds.coerceAtMost(Int.MAX_VALUE.toLong()).toInt())
    }

    private companion object {
        private val MONTH_KEY_REGEX = Regex("""\d{4}-\d{2}""")
        private const val SUGGESTION_LOG_TAG = "TimeTracerSuggestions"
        private const val SECONDS_PER_MINUTE = 60
        private const val SECONDS_PER_HOUR = 60 * SECONDS_PER_MINUTE
    }
}

internal object UnavailableRecordReportGateway : ReportGateway {
    override suspend fun reportMarkdown(request: TemporalReportQueryRequest): ReportCallResult =
        ReportCallResult(
            initialized = false,
            operationOk = false,
            outputText = "",
            rawResponse = ""
        )
}
