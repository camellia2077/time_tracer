package com.example.tracer

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import java.time.LocalDate

internal data class TxtDraftSessionState(
    val baselineText: String = "",
    val draftText: String = ""
) {
    val hasUnsavedChanges: Boolean
        get() = draftText != baselineText

    fun updateDraft(value: String): TxtDraftSessionState = copy(draftText = value)

    fun resetFromResolvedText(value: String): TxtDraftSessionState = TxtDraftSessionState(
        baselineText = value,
        draftText = value
    )
}

internal data class TxtEditorSessionUiState(
    val editorText: String = "",
    val hasUnsavedChanges: Boolean = false,
    val canIngest: Boolean = false
)

internal data class TxtEditorSessionState(
    val outputMode: TxtOutputMode = TxtOutputMode.DAY,
    val dayMarkerInput: String = "0101",
    val autoDayMarkerLoadedKey: String = "",
    val pendingOpenedDay: LocalDate? = null,
    val isEditorContentVisible: Boolean = false,
    val allDraftState: TxtDraftSessionState = TxtDraftSessionState(),
    val dayDraftState: TxtDraftSessionState = TxtDraftSessionState(),
    val lastSyncedAllDraftState: TxtDraftSessionState = TxtDraftSessionState()
)

internal object TxtEditorSessionReducer {
    fun normalizedDayMarkerInput(state: TxtEditorSessionState): String =
        state.dayMarkerInput.filter { it.isDigit() }.take(4)

    fun defaultOutputMode(selectedMonth: String): TxtOutputMode = if (selectedMonth.isBlank()) {
        TxtOutputMode.ALL
    } else {
        TxtOutputMode.DAY
    }

    fun syncSelectionContext(
        state: TxtEditorSessionState,
        selectedHistoryFile: String,
        selectedMonth: String
    ): TxtEditorSessionState {
        val nextState = state.copy(outputMode = defaultOutputMode(selectedMonth))
        if (selectedHistoryFile.isBlank()) {
            return nextState.copy(
                autoDayMarkerLoadedKey = "",
                pendingOpenedDay = null,
                isEditorContentVisible = false,
                allDraftState = TxtDraftSessionState(),
                dayDraftState = TxtDraftSessionState(),
                lastSyncedAllDraftState = TxtDraftSessionState()
            )
        }
        return nextState
    }

    fun syncExternalMonthDraft(
        state: TxtEditorSessionState,
        selectedHistoryContent: String,
        editableHistoryContent: String
    ): TxtEditorSessionState {
        // ALL now follows the same session model as DAY: ViewModel still owns file-level data,
        // but the editor owns the current typing session. We only rehydrate the ALL draft when
        // the external file state actually changes (open/discard/save/merge), so local typing
        // is not overwritten by an unchanged external snapshot on every recomposition.
        val nextExternalState = TxtDraftSessionState(
            baselineText = selectedHistoryContent,
            draftText = editableHistoryContent
        )
        if (state.lastSyncedAllDraftState == nextExternalState) {
            return state
        }
        return state.copy(
            allDraftState = nextExternalState,
            lastSyncedAllDraftState = nextExternalState
        )
    }

    fun defaultAutoDayMarkerLoadKey(
        selectedHistoryFile: String,
        selectedMonth: String,
        logicalDayTarget: RecordLogicalDayTarget
    ): String = "$selectedHistoryFile@$selectedMonth@$logicalDayTarget"

    fun hasLoadedAutoDayMarker(state: TxtEditorSessionState, loadKey: String): Boolean =
        state.autoDayMarkerLoadedKey == loadKey

    fun tryApplyPendingOpenedDay(
        state: TxtEditorSessionState,
        selectedHistoryFile: String,
        selectedMonth: String
    ): TxtEditorSessionState {
        val pendingDay = state.pendingOpenedDay ?: return state
        if (selectedHistoryFile.isBlank() || formatMonthKey(pendingDay) != selectedMonth) {
            return state
        }
        return state.copy(
            dayMarkerInput = formatDayMarker(pendingDay),
            autoDayMarkerLoadedKey = "$selectedHistoryFile@$selectedMonth@manual-day",
            pendingOpenedDay = null
        )
    }

    fun applyAutoDayMarker(
        state: TxtEditorSessionState,
        selectedHistoryFile: String,
        selectedMonth: String,
        logicalDayTarget: RecordLogicalDayTarget,
        normalizedDayMarker: String
    ): TxtEditorSessionState = state.copy(
        dayMarkerInput = normalizedDayMarker.ifBlank { state.dayMarkerInput },
        autoDayMarkerLoadedKey = defaultAutoDayMarkerLoadKey(
            selectedHistoryFile = selectedHistoryFile,
            selectedMonth = selectedMonth,
            logicalDayTarget = logicalDayTarget
        )
    )

    fun updatePendingOpenedDay(
        state: TxtEditorSessionState,
        value: LocalDate?
    ): TxtEditorSessionState = state.copy(pendingOpenedDay = value)

    fun updateDayMarkerInput(
        state: TxtEditorSessionState,
        value: String
    ): TxtEditorSessionState = state.copy(
        dayMarkerInput = value.filter { it.isDigit() }.take(4)
    )

    fun updateOutputMode(
        state: TxtEditorSessionState,
        value: TxtOutputMode
    ): TxtEditorSessionState = state.copy(outputMode = value)

    fun openEditor(
        state: TxtEditorSessionState,
        resolvedDayBody: String
    ): TxtEditorSessionState {
        // Opening DAY should always start from the currently resolved block text. This keeps
        // close/reopen semantics aligned with a file-backed editor: without an explicit ingest,
        // reopening should show the persisted day body rather than a previously abandoned draft.
        val nextState = if (state.outputMode == TxtOutputMode.DAY) {
            syncResolvedDayBody(state, resolvedDayBody)
        } else {
            state
        }
        return nextState.copy(isEditorContentVisible = true)
    }

    fun closeEditor(
        state: TxtEditorSessionState,
        resolvedDayBody: String
    ): TxtEditorSessionState {
        val nextDayDraftState = if (state.isEditorContentVisible && state.outputMode == TxtOutputMode.DAY) {
            state.dayDraftState.resetFromResolvedText(resolvedDayBody)
        } else {
            state.dayDraftState
        }
        return state.copy(
            isEditorContentVisible = false,
            dayDraftState = nextDayDraftState
        )
    }

    fun syncResolvedDayBody(
        state: TxtEditorSessionState,
        value: String
    ): TxtEditorSessionState {
        val nextDayDraftState = state.dayDraftState.resetFromResolvedText(value)
        if (state.dayDraftState == nextDayDraftState) {
            return state
        }
        return state.copy(dayDraftState = nextDayDraftState)
    }

    fun updateDayDraft(
        state: TxtEditorSessionState,
        value: String
    ): TxtEditorSessionState = state.copy(
        dayDraftState = state.dayDraftState.updateDraft(value)
    )

    fun updateAllDraft(
        state: TxtEditorSessionState,
        value: String
    ): TxtEditorSessionState = state.copy(
        allDraftState = state.allDraftState.updateDraft(value)
    )

    fun onEditorTextChange(
        state: TxtEditorSessionState,
        nextValue: String
    ): TxtEditorSessionState = if (state.outputMode == TxtOutputMode.ALL) {
        updateAllDraft(state, nextValue)
    } else {
        updateDayDraft(state, nextValue)
    }

    fun currentMonthContent(
        state: TxtEditorSessionState,
        fallbackEditableHistoryContent: String
    ): String = state.allDraftState.draftText.ifBlank { fallbackEditableHistoryContent }

    fun deriveEditorUiState(
        state: TxtEditorSessionState,
        canEditDay: Boolean
    ): TxtEditorSessionUiState {
        val hasUnsavedChanges = if (state.outputMode == TxtOutputMode.ALL) {
            state.allDraftState.hasUnsavedChanges
        } else {
            state.dayDraftState.hasUnsavedChanges
        }
        val canIngest = if (state.outputMode == TxtOutputMode.ALL) {
            hasUnsavedChanges
        } else {
            canEditDay && hasUnsavedChanges
        }
        val editorText = if (state.outputMode == TxtOutputMode.ALL) {
            state.allDraftState.draftText
        } else {
            state.dayDraftState.draftText
        }
        return TxtEditorSessionUiState(
            editorText = editorText,
            hasUnsavedChanges = hasUnsavedChanges,
            canIngest = canIngest
        )
    }

    fun closeEditorSession(
        state: TxtEditorSessionState,
        resolvedDayBody: String
    ): TxtEditorSessionState {
        val nextState = closeEditor(state, resolvedDayBody)
        if (!state.isEditorContentVisible) {
            return nextState
        }
        // DAY and ALL now share the same session contract: until users explicitly ingest,
        // closing the editor abandons local text. DAY clears its day-body draft; ALL rolls
        // its month draft back to the persisted baseline and asks the file-backed state to
        // discard any mirrored month draft as well.
        return nextState.copy(
            allDraftState = nextState.allDraftState.resetFromResolvedText(
                nextState.allDraftState.baselineText
            )
        )
    }

    fun markAllDraftPersisted(
        state: TxtEditorSessionState,
        persistedMonthContent: String
    ): TxtEditorSessionState {
        val persistedDraftState = state.allDraftState.resetFromResolvedText(persistedMonthContent)
        return state.copy(
            allDraftState = persistedDraftState,
            lastSyncedAllDraftState = persistedDraftState
        )
    }

    fun markDayDraftPersisted(state: TxtEditorSessionState): TxtEditorSessionState =
        state.copy(
            dayDraftState = state.dayDraftState.resetFromResolvedText(state.dayDraftState.draftText)
        )

    fun hideEditor(state: TxtEditorSessionState): TxtEditorSessionState =
        state.copy(isEditorContentVisible = false)
}

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
