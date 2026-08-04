package com.example.tracer

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import java.time.LocalDate

internal class TxtEditorSessionController(
    initialState: TxtEditorSessionState = TxtEditorSessionState()
) {
    var state by mutableStateOf(initialState)
        internal set

    val normalizedDayMarkerInput: String
        get() = TxtEditorSessionReducer.normalizedDayMarkerInput(state)

    fun syncSelectionContext(selectedHistoryFile: String, selectedMonth: String) {
        state = TxtEditorSessionReducer.syncSelectionContext(
            state = state,
            selectedHistoryFile = selectedHistoryFile,
            selectedMonth = selectedMonth
        )
    }

    fun syncExternalMonthDraft(
        selectedHistoryContent: String,
        editableHistoryContent: String
    ) {
        state = TxtEditorSessionReducer.syncExternalMonthDraft(
            state = state,
            selectedHistoryContent = selectedHistoryContent,
            editableHistoryContent = editableHistoryContent
        )
    }

    fun defaultAutoDayMarkerLoadKey(
        selectedHistoryFile: String,
        selectedMonth: String,
        logicalDayTarget: RecordLogicalDayTarget
    ): String = TxtEditorSessionReducer.defaultAutoDayMarkerLoadKey(
        selectedHistoryFile = selectedHistoryFile,
        selectedMonth = selectedMonth,
        logicalDayTarget = logicalDayTarget
    )

    fun hasLoadedAutoDayMarker(loadKey: String): Boolean =
        TxtEditorSessionReducer.hasLoadedAutoDayMarker(state, loadKey)

    fun tryApplyPendingOpenedDay(
        selectedHistoryFile: String,
        selectedMonth: String
    ): Boolean {
        val previousState = state
        state = TxtEditorSessionReducer.tryApplyPendingOpenedDay(
            state = state,
            selectedHistoryFile = selectedHistoryFile,
            selectedMonth = selectedMonth
        )
        return state != previousState
    }

    fun applyAutoDayMarker(
        selectedHistoryFile: String,
        selectedMonth: String,
        logicalDayTarget: RecordLogicalDayTarget,
        normalizedDayMarker: String
    ) {
        state = TxtEditorSessionReducer.applyAutoDayMarker(
            state = state,
            selectedHistoryFile = selectedHistoryFile,
            selectedMonth = selectedMonth,
            logicalDayTarget = logicalDayTarget,
            normalizedDayMarker = normalizedDayMarker
        )
    }

    fun updatePendingOpenedDay(value: LocalDate?) {
        state = TxtEditorSessionReducer.updatePendingOpenedDay(state, value)
    }

    fun updateDayMarkerInput(value: String) {
        state = TxtEditorSessionReducer.updateDayMarkerInput(state, value)
    }

    fun updateOutputMode(value: TxtOutputMode) {
        state = TxtEditorSessionReducer.updateOutputMode(state, value)
    }

    fun updateActivityNameTargetMode(value: TxtActivityNameTargetMode) {
        state = TxtEditorSessionReducer.updateActivityNameTargetMode(state, value)
    }

    fun isCurrentSelection(selectedHistoryFile: String, selectedMonth: String): Boolean =
        TxtEditorSessionReducer.isCurrentSelection(state, selectedHistoryFile, selectedMonth)

    fun openEditor(resolvedDayBody: String) {
        state = TxtEditorSessionReducer.openEditor(state, resolvedDayBody)
    }

    fun closeEditor(resolvedDayBody: String): Boolean {
        val wasVisible = state.isEditorContentVisible
        state = TxtEditorSessionReducer.closeEditor(state, resolvedDayBody)
        return wasVisible
    }

    fun syncResolvedDayBody(value: String) {
        state = TxtEditorSessionReducer.syncResolvedDayBody(state, value)
    }

    fun updateAllDraft(value: String) {
        state = TxtEditorSessionReducer.updateAllDraft(state, value)
    }

    fun currentMonthContent(fallbackEditableHistoryContent: String): String =
        TxtEditorSessionReducer.currentMonthContent(state, fallbackEditableHistoryContent)

    fun deriveEditorUiState(canEditDay: Boolean): TxtEditorSessionUiState =
        TxtEditorSessionReducer.deriveEditorUiState(state, canEditDay)

    fun closeEditorSession(
        resolvedDayBody: String,
        onDiscardAllDraft: () -> Unit
    ): Boolean {
        val wasVisible = state.isEditorContentVisible
        state = TxtEditorSessionReducer.closeEditorSession(state, resolvedDayBody)
        if (wasVisible) {
            onDiscardAllDraft()
        }
        return wasVisible
    }

    fun onEditorTextChange(nextValue: String) {
        state = TxtEditorSessionReducer.onEditorTextChange(state, nextValue)
    }
}

internal suspend fun ingestDayDraft(
    txtStorageGateway: TxtStorageGateway,
    monthContent: String,
    dayMarker: String,
    dayDraftBody: String,
    onMergedMonthContent: (String) -> Unit,
    onSaveHistoryFile: () -> Unit
): Boolean {
    // DAY editing intentionally stays local while users type. Only an explicit ingest should
    // re-enter the shared month-TXT pipeline, merge the edited block back into the month text,
    // and then persist/sync that month file.
    val replaced = txtStorageGateway.replaceTxtDayBlock(
        content = monthContent,
        dayMarker = dayMarker,
        editedDayBody = dayDraftBody
    )
    if (!replaced.ok) {
        return false
    }
    onMergedMonthContent(replaced.updatedContent)
    onSaveHistoryFile()
    return true
}
