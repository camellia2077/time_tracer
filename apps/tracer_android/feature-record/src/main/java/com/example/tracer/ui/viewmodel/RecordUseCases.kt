package com.example.tracer

import java.text.SimpleDateFormat
import java.util.Calendar
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
            targetDateIso = targetDateIso
        )
        val preferredMonth = resolvePreferredMonthForRecord(targetDateIso)
        return refreshAndOpen(state, preferredMonth, result.message)
    }

    suspend fun createCurrentMonthTxt(state: RecordUiState): RecordUiState {
        val result = recordGateway.createCurrentMonthTxt()
        val preferredMonth = getCurrentMonthKey()
        return refreshAndOpen(state, preferredMonth, result.message)
    }

    suspend fun refreshHistory(state: RecordUiState): RecordUiState {
        val preferredMonth = state.selectedMonth.ifEmpty { null }
        return refreshAndOpen(state, preferredMonth, "TXT history refreshed.")
    }

    suspend fun openHistoryFile(state: RecordUiState, path: String): RecordUiState {
        val readResult = txtStorageGateway.readLiveTxtFile(path)
        if (!readResult.ok) {
            return state.copy(statusText = readResult.message)
        }

        val selectedMonth = parseMonthFromContent(readResult.content)
            ?: parseMonthFromFilePath(readResult.filePath)
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
            val createResult = recordGateway.createMonthTxt(month)
            return refreshAndOpen(state, month, createResult.message)
        }
        return refreshAndOpen(state, month, "open month -> $month")
    }

    suspend fun openPreviousMonth(state: RecordUiState): RecordUiState {
        val baseMonth = state.selectedMonth.ifBlank { getCurrentMonthKey() }
        val targetMonth = shiftMonth(baseMonth, -1)
            ?: return state.copy(statusText = "Invalid month: $baseMonth")
        if (!state.availableMonths.contains(targetMonth)) {
            val createResult = recordGateway.createMonthTxt(targetMonth)
            return refreshAndOpen(state, targetMonth, createResult.message)
        }
        return refreshAndOpen(state, targetMonth, "open month -> $targetMonth")
    }

    suspend fun openNextMonth(state: RecordUiState): RecordUiState {
        val baseMonth = state.selectedMonth.ifBlank { getCurrentMonthKey() }
        val targetMonth = shiftMonth(baseMonth, 1)
            ?: return state.copy(statusText = "Invalid month: $baseMonth")
        if (!state.availableMonths.contains(targetMonth)) {
            val createResult = recordGateway.createMonthTxt(targetMonth)
            return refreshAndOpen(state, targetMonth, createResult.message)
        }
        return refreshAndOpen(state, targetMonth, "open month -> $targetMonth")
    }

    suspend fun saveHistoryFileAndSync(state: RecordUiState): RecordUiState {
        val selectedFile = state.selectedHistoryFile
        if (selectedFile.isEmpty()) {
            return state.copy(statusText = "No TXT file selected.")
        }

        val saveResult = txtStorageGateway.saveLiveTxtFileAndSync(
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
            statusText = "live txt cleared. editor state reset."
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
        val history = txtStorageGateway.listLiveTxtFiles()
        if (!history.ok) {
            return state.copy(statusText = "$statusPrefix ${history.message}")
        }

        val monthToFile = buildMonthToFileIndex(history.files)
        val months = monthToFile.keys.sorted()

        if (months.isEmpty()) {
            return state.copy(
                historyFiles = history.files,
                availableMonths = emptyList(),
                selectedMonth = "",
                selectedHistoryFile = "",
                selectedHistoryContent = "",
                editableHistoryContent = "",
                statusText = statusPrefix
            )
        }

        val targetMonth = when {
            !preferredMonth.isNullOrBlank() && monthToFile.containsKey(preferredMonth) -> preferredMonth
            state.selectedMonth.isNotBlank() && monthToFile.containsKey(state.selectedMonth) -> state.selectedMonth
            else -> months.last()
        }
        val targetFile = monthToFile[targetMonth]
            ?: return state.copy(statusText = "$statusPrefix No TXT file for month $targetMonth.")

        val readResult = txtStorageGateway.readLiveTxtFile(targetFile)
        if (!readResult.ok) {
            return state.copy(
                historyFiles = history.files,
                availableMonths = months,
                statusText = "$statusPrefix ${readResult.message}"
            )
        }

        return state.copy(
            historyFiles = history.files,
            availableMonths = months,
            selectedMonth = targetMonth,
            selectedHistoryFile = readResult.filePath,
            selectedHistoryContent = readResult.content,
            editableHistoryContent = readResult.content,
            statusText = statusPrefix
        )
    }

    private suspend fun buildMonthToFileIndex(files: List<String>): Map<String, String> {
        val index = linkedMapOf<String, String>()
        for (path in files.sorted()) {
            val readResult = txtStorageGateway.readLiveTxtFile(path)
            if (!readResult.ok) {
                continue
            }
            val month = parseMonthFromContent(readResult.content)
                ?: parseMonthFromFilePath(readResult.filePath)
                ?: continue
            if (!index.containsKey(month)) {
                index[month] = readResult.filePath
            }
        }
        return index
    }

    private fun parseMonthFromContent(content: String): String? {
        var year: String? = null
        var month: String? = null
        val yearRegex = Regex("""^y(\d{4})$""")
        val dayRegex = Regex("""^(\d{2})(\d{2})$""")

        for (rawLine in content.lineSequence()) {
            val line = rawLine.trim()
            if (line.isEmpty()) {
                continue
            }

            val yearMatch = yearRegex.matchEntire(line)
            if (yearMatch != null && year == null) {
                year = yearMatch.groupValues[1]
                continue
            }

            val dayMatch = dayRegex.matchEntire(line)
            if (dayMatch != null) {
                val monthValue = dayMatch.groupValues[1].toIntOrNull() ?: continue
                if (monthValue in 1..12) {
                    month = String.format(Locale.US, "%02d", monthValue)
                    break
                }
            }
        }

        if (year == null || month == null) {
            return null
        }
        return "$year-$month"
    }

    private fun parseMonthFromFilePath(path: String): String? {
        val fileName = path.substringAfterLast('/')
        val match = Regex("""^(\d{4})[-_](\d{2})\.txt$""").matchEntire(fileName)
            ?: return null
        val year = match.groupValues[1]
        val monthValue = match.groupValues[2].toIntOrNull() ?: return null
        if (monthValue !in 1..12) {
            return null
        }
        return "$year-${String.format(Locale.US, "%02d", monthValue)}"
    }

    private fun shiftMonth(month: String, delta: Int): String? {
        return try {
            val date = SimpleDateFormat("yyyy-MM", Locale.US).apply { isLenient = false }.parse(month)
                ?: return null
            val cal = Calendar.getInstance().apply {
                time = date
                add(Calendar.MONTH, delta)
            }
            SimpleDateFormat("yyyy-MM", Locale.US).format(cal.time)
        } catch (_: Exception) {
            null
        }
    }
}
