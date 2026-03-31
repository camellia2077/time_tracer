package com.example.tracer

import java.time.Clock
import java.time.LocalDate
import java.time.YearMonth
import java.time.format.DateTimeFormatter

class RecordUseCases(
    private val recordGateway: RecordGateway,
    private val txtStorageGateway: TxtStorageGateway,
    private val queryGateway: QueryGateway,
    private val clock: Clock = Clock.systemDefaultZone()
) {
    suspend fun recordNow(state: RecordUiState): RecordUiState {
        val targetDateIso = resolveTargetDateIso(state.logicalDayTarget)
        val timeOrderMode = resolveRecordTimeOrderMode(state.logicalDayTarget)

        val result = recordGateway.recordNow(
            activityName = state.recordContent,
            remark = state.recordRemark,
            targetDateIso = targetDateIso,
            preferredTxtPath = state.selectedHistoryFile,
            timeOrderMode = timeOrderMode
        )
        val preferredMonth = resolvePreferredMonthForRecord(targetDateIso)
        // Clear the previous activity/remark after every record attempt. Otherwise users have to
        // manually delete the last input before typing the next activity, especially when a
        // failed insert leaves stale content in the fields.
        val stateAfterRecord = state.copy(
            recordContent = "",
            recordRemark = ""
        )
        return refreshAndOpen(stateAfterRecord, preferredMonth, result.message)
    }

    suspend fun refreshHistory(state: RecordUiState): RecordUiState {
        val preferredMonth = state.selectedMonth.ifEmpty { null }
        return refreshAndOpen(state, preferredMonth, "TXT history refreshed.")
    }

    suspend fun openHistoryFile(
        state: RecordUiState,
        path: String,
        statusPrefixOverride: String? = null
    ): RecordUiState {
        val inspection = txtStorageGateway.inspectTxtFiles()
        if (!inspection.ok) {
            return state.copy(statusText = inspection.message)
        }

        val sortedEntries = inspection.entries.sortedBy { it.relativePath }
        val normalizedPath = path.replace('\\', '/')
        val inspectionEntry = sortedEntries.firstOrNull {
            it.relativePath.replace('\\', '/') == normalizedPath
        } ?: return state.copy(statusText = "TXT file $path not found.")

        return openInspectionEntry(
            state = state,
            entries = sortedEntries,
            inspectionEntry = inspectionEntry,
            availableMonths = buildMonthToFileIndex(sortedEntries).keys.sorted(),
            statusPrefix = statusPrefixOverride
        )
    }

    suspend fun openMonth(state: RecordUiState, month: String): RecordUiState {
        if (month.isBlank()) {
            return state
        }
        val monthToFile = buildEditorMonthToFileIndex(state.txtInspectionEntries)
        val targetFile = monthToFile[month]
        if (targetFile == null) {
            return state.copy(statusText = "Month $month not found in TXT list.")
        }
        return openHistoryFile(
            state = state,
            path = targetFile,
            statusPrefixOverride = "open month -> $month"
        )
    }

    suspend fun openPreviousMonth(state: RecordUiState): RecordUiState {
        val months = buildEditorMonthToFileIndex(state.txtInspectionEntries).keys.sorted()
        if (months.isEmpty()) {
            return state.copy(statusText = "No TXT files available.")
        }
        val baseIndex = if (state.selectedMonth.isBlank()) {
            months.lastIndex
        } else {
            months.indexOf(state.selectedMonth)
        }
        if (baseIndex <= 0) {
            return state.copy(statusText = "Already at earliest month in TXT list.")
        }
        val targetMonth = months[baseIndex - 1]
        return refreshAndOpen(state, targetMonth, "open month -> $targetMonth")
    }

    suspend fun openNextMonth(state: RecordUiState): RecordUiState {
        val months = buildEditorMonthToFileIndex(state.txtInspectionEntries).keys.sorted()
        if (months.isEmpty()) {
            return state.copy(statusText = "No TXT files available.")
        }
        val baseIndex = if (state.selectedMonth.isBlank()) {
            -1
        } else {
            months.indexOf(state.selectedMonth)
        }
        if (baseIndex >= months.lastIndex) {
            return state.copy(statusText = "Already at latest month in TXT list.")
        }
        val targetMonth = months[baseIndex + 1]
        return refreshAndOpen(state, targetMonth, "open month -> $targetMonth")
    }

    suspend fun saveHistoryFileAndSync(state: RecordUiState): RecordUiState {
        val selectedFile = state.selectedHistoryFile
        if (selectedFile.isEmpty()) {
            return state.copy(statusText = "No TXT file selected.")
        }

        val saveResult = txtStorageGateway.saveTxtFileAndSync(
            relativePath = selectedFile,
            content = state.editableHistoryContent
        )
        return if (saveResult.ok) {
            state.copy(
                selectedHistoryContent = state.editableHistoryContent,
                statusText = saveResult.message
            )
        } else {
            state.copy(statusText = saveResult.message)
        }
    }

    suspend fun createCurrentMonthTxt(state: RecordUiState): RecordUiState {
        val result = recordGateway.createCurrentMonthTxt()
        if (!result.ok) {
            return state.copy(statusText = result.message)
        }
        val currentMonth = getCurrentMonthKey()
        return refreshAndOpen(state, currentMonth, result.message)
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
            logicalDayTarget = defaultLogicalDayTarget(clock.millis(), clock.zone),
            logicalDayIsUserOverride = false,
            historyFiles = emptyList(),
            txtInspectionEntries = emptyList(),
            availableMonths = emptyList(),
            selectedMonth = "",
            selectedHistoryFile = "",
            selectedHistoryContent = "",
            editableHistoryContent = "",
            suggestedActivities = emptyList(),
            suggestionsVisible = false,
            isSuggestionsLoading = false,
            statusText = "TXT editor state reset."
        )
    }

    private fun getCurrentMonthKey(): String {
        return YearMonth.now(clock).format(MONTH_FORMATTER)
    }

    private fun resolvePreferredMonthForRecord(targetDateIso: String?): String {
        if (targetDateIso.isNullOrBlank()) {
            return getCurrentMonthKey()
        }

        return try {
            YearMonth.from(LocalDate.parse(targetDateIso)).format(MONTH_FORMATTER)
        } catch (_: Exception) {
            getCurrentMonthKey()
        }
    }

    private fun resolveTargetDateIso(logicalDayTarget: RecordLogicalDayTarget): String {
        // Record and TXT must share the same logical-day resolver so "yesterday/today" means
        // exactly the same date in both tabs within the current session.
        val targetDate = resolveLogicalDayTargetDate(
            logicalDayTarget = logicalDayTarget,
            clock = clock
        )
        return targetDate.format(DateTimeFormatter.ISO_LOCAL_DATE)
    }

    private fun resolveRecordTimeOrderMode(
        logicalDayTarget: RecordLogicalDayTarget
    ): RecordTimeOrderMode {
        // Explicit mode contract:
        // - record to yesterday enables logical_day_0600 so 00:xx belongs to the same
        //   logical day axis as prior late-night entries.
        // - record to today stays strict calendar ordering and does not apply rollover mapping.
        return when (logicalDayTarget) {
            RecordLogicalDayTarget.YESTERDAY -> RecordTimeOrderMode.LOGICAL_DAY_0600
            RecordLogicalDayTarget.TODAY -> RecordTimeOrderMode.STRICT_CALENDAR
        }
    }

    private suspend fun refreshAndOpen(
        state: RecordUiState,
        preferredMonth: String?,
        statusPrefix: String
    ): RecordUiState {
        val inspection = txtStorageGateway.inspectTxtFiles()
        if (!inspection.ok) {
            return state.copy(statusText = "$statusPrefix ${inspection.message}")
        }

        val sortedEntries = inspection.entries.sortedBy { it.relativePath }
        val historyFiles = sortedEntries.map { it.relativePath }
        val monthToFile = buildMonthToFileIndex(sortedEntries)
        val editorMonthToFile = buildEditorMonthToFileIndex(sortedEntries)
        val months = monthToFile.keys.sorted()
        val editorMonths = editorMonthToFile.keys.sorted()

        if (editorMonths.isEmpty()) {
            if (historyFiles.isEmpty()) {
                return state.copy(
                    historyFiles = emptyList(),
                    txtInspectionEntries = emptyList(),
                    availableMonths = emptyList(),
                    selectedMonth = "",
                    selectedHistoryFile = "",
                    selectedHistoryContent = "",
                    editableHistoryContent = "",
                    statusText = "$statusPrefix No openable TXT files found under input."
                )
            }

            // If every discovered TXT is currently blocked, still open the first entry for repair
            // instead of leaving the editor unusable. This keeps TXT correction possible when the
            // month header or ingest sync state is already broken.
            val firstEntry = sortedEntries.first()
            return openInspectionEntry(
                state = state,
                entries = sortedEntries,
                inspectionEntry = firstEntry,
                availableMonths = months,
                statusPrefix = statusPrefix
            )
        }

        val targetMonth = when {
            !preferredMonth.isNullOrBlank() && editorMonthToFile.containsKey(preferredMonth) -> preferredMonth
            state.selectedMonth.isNotBlank() && editorMonthToFile.containsKey(state.selectedMonth) -> state.selectedMonth
            editorMonthToFile.containsKey(getCurrentMonthKey()) -> getCurrentMonthKey()
            else -> editorMonths.last()
        }
        val targetFile = editorMonthToFile[targetMonth]
            ?: return state.copy(statusText = "$statusPrefix No TXT file for month $targetMonth.")
        val targetEntry = sortedEntries.firstOrNull { it.relativePath == targetFile }
            ?: return state.copy(statusText = "$statusPrefix TXT file metadata missing for $targetFile.")

        return openInspectionEntry(
            state = state,
            entries = sortedEntries,
            inspectionEntry = targetEntry,
            availableMonths = months,
            statusPrefix = statusPrefix
        )
    }

    private suspend fun openInspectionEntry(
        state: RecordUiState,
        entries: List<TxtInspectionEntry>,
        inspectionEntry: TxtInspectionEntry,
        availableMonths: List<String>,
        statusPrefix: String?
    ): RecordUiState {
        val readResult = txtStorageGateway.readTxtFile(inspectionEntry.relativePath)
        if (!readResult.ok) {
            val noOpenableMessage = if (entries.isEmpty()) {
                "$statusPrefix No openable TXT files found under input."
            } else {
                "${statusPrefix ?: "open txt"} ${readResult.message}"
            }
            return state.copy(
                historyFiles = entries.map { it.relativePath },
                txtInspectionEntries = entries,
                availableMonths = availableMonths,
                statusText = noOpenableMessage
            )
        }

        val resolvedStatus = when {
            inspectionEntry.canOpen && !statusPrefix.isNullOrBlank() -> statusPrefix
            inspectionEntry.canOpen -> "open txt -> ${readResult.filePath}"
            !statusPrefix.isNullOrBlank() -> "$statusPrefix Repair needed: ${inspectionEntry.message}"
            else -> "repair txt -> ${readResult.filePath}. ${inspectionEntry.message}"
        }

        return state.copy(
            historyFiles = entries.map { it.relativePath },
            txtInspectionEntries = entries,
            availableMonths = availableMonths,
            selectedMonth = inspectionEntry.headerMonth.orEmpty(),
            selectedHistoryFile = readResult.filePath,
            selectedHistoryContent = readResult.content,
            editableHistoryContent = readResult.content,
            statusText = resolvedStatus
        )
    }

    private fun buildMonthToFileIndex(entries: List<TxtInspectionEntry>): Map<String, String> {
        val index = linkedMapOf<String, String>()
        for (entry in entries.sortedBy { it.relativePath }) {
            if (!entry.canOpen) {
                continue
            }
            val month = entry.headerMonth ?: continue
            if (!index.containsKey(month)) {
                index[month] = entry.relativePath
            }
        }
        return index
    }

    private fun buildEditorMonthToFileIndex(entries: List<TxtInspectionEntry>): Map<String, String> {
        val index = linkedMapOf<String, String>()
        // For the editor, prefer a month's openable TXT when duplicates exist, but keep a blocked
        // candidate as fallback so the month still has a repair entry point instead of disappearing.
        val sortedEntries = entries.sortedWith(
            compareBy<TxtInspectionEntry> { it.headerMonth.orEmpty() }
                .thenByDescending { it.canOpen }
                .thenBy { it.relativePath }
        )
        for (entry in sortedEntries) {
            val month = entry.headerMonth ?: continue
            if (!index.containsKey(month)) {
                index[month] = entry.relativePath
            }
        }
        return index
    }

    private companion object {
        private val MONTH_FORMATTER: DateTimeFormatter = DateTimeFormatter.ofPattern("yyyy-MM")
    }
}
