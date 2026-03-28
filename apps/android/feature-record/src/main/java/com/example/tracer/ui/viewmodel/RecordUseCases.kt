package com.example.tracer

import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

class RecordUseCases(
    private val recordGateway: RecordGateway,
    private val txtStorageGateway: TxtStorageGateway,
    private val queryGateway: QueryGateway
) {
    suspend fun recordNow(state: RecordUiState): RecordUiState {
        val targetDateIso = if (state.useManualDate) {
            state.manualDate.trim()
        } else {
            null
        }

        if (state.useManualDate) {
            if (targetDateIso.isNullOrEmpty()) {
                return state.copy(statusText = "Manual target date is empty. Use YYYY-MM-DD.")
            }

            try {
                SimpleDateFormat("yyyy-MM-dd", Locale.US).apply { isLenient = false }.parse(targetDateIso)
            } catch (_: Exception) {
                return state.copy(statusText = "Invalid target date. Use ISO YYYY-MM-DD.")
            }
        }

        val result = recordGateway.recordNow(
            activityName = state.recordContent,
            remark = state.recordRemark,
            targetDateIso = targetDateIso,
            preferredTxtPath = state.selectedHistoryFile
        )
        val preferredMonth = resolvePreferredMonthForRecord(targetDateIso)
        return refreshAndOpen(state, preferredMonth, result.message)
    }

    suspend fun refreshHistory(state: RecordUiState): RecordUiState {
        val preferredMonth = state.selectedMonth.ifEmpty { null }
        return refreshAndOpen(state, preferredMonth, "TXT history refreshed.")
    }

    suspend fun openHistoryFile(state: RecordUiState, path: String): RecordUiState {
        val inspection = txtStorageGateway.inspectTxtFiles()
        if (!inspection.ok) {
            return state.copy(statusText = inspection.message)
        }

        val normalizedPath = path.replace('\\', '/')
        val inspectionEntry = inspection.entries.firstOrNull {
            it.relativePath.replace('\\', '/') == normalizedPath
        } ?: return state.copy(statusText = "TXT file $path not found.")
        if (!inspectionEntry.canOpen) {
            return state.copy(statusText = inspectionEntry.message)
        }

        val readResult = txtStorageGateway.readTxtFile(inspectionEntry.relativePath)
        if (!readResult.ok) {
            return state.copy(statusText = readResult.message)
        }

        val selectedMonth = inspectionEntry.headerMonth
            ?: state.selectedMonth

        return state.copy(
            selectedHistoryFile = readResult.filePath,
            selectedHistoryContent = readResult.content,
            editableHistoryContent = readResult.content,
            selectedMonth = selectedMonth,
            statusText = "open txt -> ${readResult.filePath}"
        )
    }

    suspend fun openMonth(state: RecordUiState, month: String): RecordUiState {
        if (month.isBlank()) {
            return state
        }
        if (!state.availableMonths.contains(month)) {
            return state.copy(statusText = "Month $month not found in TXT list.")
        }
        return refreshAndOpen(state, month, "open month -> $month")
    }

    suspend fun openPreviousMonth(state: RecordUiState): RecordUiState {
        val months = state.availableMonths.sorted()
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
        val months = state.availableMonths.sorted()
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
            useManualDate = false,
            manualDate = SimpleDateFormat("yyyy-MM-dd", Locale.US).format(Date()),
            historyFiles = emptyList(),
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
        return SimpleDateFormat("yyyy-MM", Locale.US).format(Date())
    }

    private fun resolvePreferredMonthForRecord(targetDateIso: String?): String {
        if (targetDateIso.isNullOrBlank()) {
            return getCurrentMonthKey()
        }

        return try {
            val date = SimpleDateFormat("yyyy-MM-dd", Locale.US).apply { isLenient = false }.parse(targetDateIso)
            SimpleDateFormat("yyyy-MM", Locale.US).format(date!!)
        } catch (_: Exception) {
            getCurrentMonthKey()
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

        val historyFiles = inspection.entries.map { it.relativePath }.sorted()
        val monthToFile = buildMonthToFileIndex(inspection.entries)
        val months = monthToFile.keys.sorted()

        if (months.isEmpty()) {
            return state.copy(
                historyFiles = historyFiles,
                availableMonths = emptyList(),
                selectedMonth = "",
                selectedHistoryFile = "",
                selectedHistoryContent = "",
                editableHistoryContent = "",
                statusText = "$statusPrefix No openable TXT files found under input."
            )
        }

        val targetMonth = when {
            !preferredMonth.isNullOrBlank() && monthToFile.containsKey(preferredMonth) -> preferredMonth
            state.selectedMonth.isNotBlank() && monthToFile.containsKey(state.selectedMonth) -> state.selectedMonth
            monthToFile.containsKey(getCurrentMonthKey()) -> getCurrentMonthKey()
            else -> months.last()
        }
        val targetFile = monthToFile[targetMonth]
            ?: return state.copy(statusText = "$statusPrefix No TXT file for month $targetMonth.")

        val readResult = txtStorageGateway.readTxtFile(targetFile)
        if (!readResult.ok) {
            return state.copy(
                historyFiles = historyFiles,
                availableMonths = months,
                statusText = "$statusPrefix ${readResult.message}"
            )
        }

        return state.copy(
            historyFiles = historyFiles,
            availableMonths = months,
            selectedMonth = targetMonth,
            selectedHistoryFile = readResult.filePath,
            selectedHistoryContent = readResult.content,
            editableHistoryContent = readResult.content,
            statusText = statusPrefix
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
}
