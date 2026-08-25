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

internal enum class TxtActivityNameTargetMode {
    CANONICAL,
    ALIAS
}

internal data class TxtEditorSessionState(
    val outputMode: TxtOutputMode = TxtOutputMode.DAY,
    val dayMarkerInput: String = "0101",
    val autoDayMarkerLoadedKey: String = "",
    val pendingOpenedDay: LocalDate? = null,
    val isEditorContentVisible: Boolean = false,
    val allDraftState: TxtDraftSessionState = TxtDraftSessionState(),
    val dayDraftState: TxtDraftSessionState = TxtDraftSessionState(),
    val lastSyncedAllDraftState: TxtDraftSessionState = TxtDraftSessionState(),
    val selectionContextKey: String = ""
)

internal object TxtEditorSessionReducer {
    fun normalizedDayMarkerInput(state: TxtEditorSessionState): String =
        state.dayMarkerInput.filter { it.isDigit() }.take(4)

    fun syncSelectionContext(
        state: TxtEditorSessionState,
        selectedHistoryFile: String,
        selectedMonth: String
    ): TxtEditorSessionState {
        val contextKey = "$selectedHistoryFile@$selectedMonth"
        val nextState = state.copy(selectionContextKey = contextKey)
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

    fun isCurrentSelection(
        state: TxtEditorSessionState,
        selectedHistoryFile: String,
        selectedMonth: String
    ): Boolean = state.selectionContextKey == "$selectedHistoryFile@$selectedMonth"

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

