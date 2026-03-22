package com.example.tracer

internal class RecordUseCaseCaller(
    private val recordUseCases: RecordUseCases
) {
    suspend fun loadActivitySuggestions(
        state: RecordUiState,
        lookbackDays: Int,
        topN: Int
    ): RecordUiState = recordUseCases.loadActivitySuggestions(
        state = state,
        lookbackDays = lookbackDays,
        topN = topN
    )

    suspend fun recordNow(state: RecordUiState): RecordUiState =
        recordUseCases.recordNow(state)

    suspend fun refreshHistory(state: RecordUiState): RecordUiState =
        recordUseCases.refreshHistory(state)

    suspend fun openHistoryFile(state: RecordUiState, path: String): RecordUiState =
        recordUseCases.openHistoryFile(state, path)

    suspend fun openMonth(state: RecordUiState, month: String): RecordUiState =
        recordUseCases.openMonth(state, month)

    suspend fun openPreviousMonth(state: RecordUiState): RecordUiState =
        recordUseCases.openPreviousMonth(state)

    suspend fun openNextMonth(state: RecordUiState): RecordUiState =
        recordUseCases.openNextMonth(state)

    suspend fun saveHistoryFileAndSync(state: RecordUiState): RecordUiState =
        recordUseCases.saveHistoryFileAndSync(state)

    suspend fun createCurrentMonthTxt(state: RecordUiState): RecordUiState =
        recordUseCases.createCurrentMonthTxt(state)

    fun clearEditorState(state: RecordUiState): RecordUiState =
        recordUseCases.clearEditorState(state)
}
