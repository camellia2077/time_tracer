package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import java.time.LocalDate

class TxtEditorSessionReducerTest {
    @Test
    fun syncExternalMonthDraft_onlyUpdatesWhenExternalSnapshotChanges() {
        val initial = TxtEditorSessionState()
        val synced = TxtEditorSessionReducer.syncExternalMonthDraft(
            state = initial,
            selectedHistoryContent = "saved",
            editableHistoryContent = "draft"
        )

        val repeated = TxtEditorSessionReducer.syncExternalMonthDraft(
            state = synced,
            selectedHistoryContent = "saved",
            editableHistoryContent = "draft"
        )

        assertEquals(
            TxtDraftSessionState(
                baselineText = "saved",
                draftText = "draft"
            ),
            synced.allDraftState
        )
        assertEquals(synced, repeated)
    }

    @Test
    fun closeEditorSession_resetsAllDraftToBaseline_andHidesEditor() {
        val state = TxtEditorSessionState(
            outputMode = TxtOutputMode.ALL,
            isEditorContentVisible = true,
            allDraftState = TxtDraftSessionState(
                baselineText = "saved",
                draftText = "draft"
            )
        )

        val closed = TxtEditorSessionReducer.closeEditorSession(
            state = state,
            resolvedDayBody = ""
        )

        assertFalse(closed.isEditorContentVisible)
        assertEquals("saved", closed.allDraftState.baselineText)
        assertEquals("saved", closed.allDraftState.draftText)
    }

    @Test
    fun reducerDerivesUiStateForAllAndDayModes() {
        val allState = TxtEditorSessionState(
            outputMode = TxtOutputMode.ALL,
            allDraftState = TxtDraftSessionState(
                baselineText = "saved",
                draftText = "draft"
            )
        )
        assertEquals(
            TxtEditorSessionUiState(
                editorText = "draft",
                hasUnsavedChanges = true,
                canIngest = true
            ),
            TxtEditorSessionReducer.deriveEditorUiState(
                state = allState,
                canEditDay = false
            )
        )

        val dayState = TxtEditorSessionState(
            outputMode = TxtOutputMode.DAY,
            dayDraftState = TxtDraftSessionState(
                baselineText = "0900study\n",
                draftText = "0900study\n1000break\n"
            )
        )
        assertEquals(
            TxtEditorSessionUiState(
                editorText = "0900study\n1000break\n",
                hasUnsavedChanges = true,
                canIngest = false
            ),
            TxtEditorSessionReducer.deriveEditorUiState(
                state = dayState,
                canEditDay = false
            )
        )
    }

    @Test
    fun tryApplyPendingOpenedDay_onlyAppliesInsideTargetMonth() {
        val pendingDay = LocalDate.of(2026, 4, 17)
        val state = TxtEditorSessionState(pendingOpenedDay = pendingDay)

        val applied = TxtEditorSessionReducer.tryApplyPendingOpenedDay(
            state = state,
            selectedHistoryFile = "2026/2026-04.txt",
            selectedMonth = "2026-04"
        )
        val ignored = TxtEditorSessionReducer.tryApplyPendingOpenedDay(
            state = state,
            selectedHistoryFile = "2026/2026-04.txt",
            selectedMonth = "2026-05"
        )

        assertEquals("0417", applied.dayMarkerInput)
        assertEquals(null, applied.pendingOpenedDay)
        assertEquals(state, ignored)
    }

    @Test
    fun onEditorTextChange_routesToCurrentModeDraft() {
        val allState = TxtEditorSessionReducer.onEditorTextChange(
            state = TxtEditorSessionState(
                outputMode = TxtOutputMode.ALL,
                allDraftState = TxtDraftSessionState(
                    baselineText = "saved",
                    draftText = "saved"
                )
            ),
            nextValue = "draft"
        )
        assertTrue(allState.allDraftState.hasUnsavedChanges)

        val dayState = TxtEditorSessionReducer.onEditorTextChange(
            state = TxtEditorSessionState(
                outputMode = TxtOutputMode.DAY,
                dayDraftState = TxtDraftSessionState(
                    baselineText = "0900study\n",
                    draftText = "0900study\n"
                )
            ),
            nextValue = "0900study\n1000break\n"
        )
        assertTrue(dayState.dayDraftState.hasUnsavedChanges)
    }
}
