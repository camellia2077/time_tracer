package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Test
import java.time.LocalDate

class TxtEditorSessionControllerTest {
    @Test
    fun syncSelectionContext_blankFile_clearsSessionState() {
        val controller = TxtEditorSessionController()
        controller.updateOutputMode(TxtOutputMode.ALL)
        controller.updateDayMarkerInput("0417")
        controller.updatePendingOpenedDay(LocalDate.of(2026, 4, 18))
        controller.syncExternalMonthDraft(
            selectedHistoryContent = "saved-all",
            editableHistoryContent = "draft-all"
        )
        controller.openEditor(resolvedDayBody = "0900study\n")
        controller.syncResolvedDayBody("0900study\n1000break\n")

        controller.syncSelectionContext(
            selectedHistoryFile = "",
            selectedMonth = ""
        )

        assertEquals(TxtOutputMode.ALL, controller.state.outputMode)
        assertEquals("", controller.state.autoDayMarkerLoadedKey)
        assertNull(controller.state.pendingOpenedDay)
        assertFalse(controller.state.isEditorContentVisible)
        assertEquals(TxtDraftSessionState(), controller.state.allDraftState)
        assertEquals(TxtDraftSessionState(), controller.state.dayDraftState)
    }

    @Test
    fun tryApplyPendingOpenedDay_sameMonth_updatesMarkerAndClearsPending() {
        val controller = TxtEditorSessionController()
        val pendingDay = LocalDate.of(2026, 4, 17)
        controller.updatePendingOpenedDay(pendingDay)

        val applied = controller.tryApplyPendingOpenedDay(
            selectedHistoryFile = "2026/2026-04.txt",
            selectedMonth = "2026-04"
        )

        assertTrue(applied)
        assertEquals("0417", controller.state.dayMarkerInput)
        assertEquals("2026/2026-04.txt@2026-04@manual-day", controller.state.autoDayMarkerLoadedKey)
        assertNull(controller.state.pendingOpenedDay)
    }

    @Test
    fun applyAutoDayMarker_setsLoadKey_andPreservesExistingMarkerWhenBlank() {
        val controller = TxtEditorSessionController()
        controller.updateDayMarkerInput("0416")

        controller.applyAutoDayMarker(
            selectedHistoryFile = "2026/2026-04.txt",
            selectedMonth = "2026-04",
            logicalDayTarget = RecordLogicalDayTarget.TODAY,
            normalizedDayMarker = ""
        )

        assertEquals("0416", controller.state.dayMarkerInput)
        assertEquals(
            "2026/2026-04.txt@2026-04@TODAY",
            controller.state.autoDayMarkerLoadedKey
        )
    }

    @Test
    fun closeEditor_inDayMode_discardsLocalDraft_andReopenStartsFromResolvedBody() {
        val controller = TxtEditorSessionController()
        controller.syncResolvedDayBody("0900study\n")
        controller.openEditor(resolvedDayBody = "0900study\n")
        controller.onEditorTextChange(nextValue = "0900study\n1000break\n")

        val wasVisible = controller.closeEditor(resolvedDayBody = "0900study\n")

        assertTrue(wasVisible)
        assertFalse(controller.state.isEditorContentVisible)
        assertEquals(
            TxtDraftSessionState(
                baselineText = "0900study\n",
                draftText = "0900study\n"
            ),
            controller.state.dayDraftState
        )

        controller.openEditor(resolvedDayBody = "0900study\n")

        assertTrue(controller.state.isEditorContentVisible)
        assertEquals(
            "0900study\n",
            controller.deriveEditorUiState(canEditDay = true).editorText
        )
    }

    @Test
    fun onEditorTextChange_routesByMode() {
        val controller = TxtEditorSessionController()

        controller.updateOutputMode(TxtOutputMode.ALL)
        controller.syncExternalMonthDraft(
            selectedHistoryContent = "saved-all",
            editableHistoryContent = "original"
        )
        controller.onEditorTextChange(nextValue = "edited-all")

        assertEquals("edited-all", controller.state.allDraftState.draftText)
        assertTrue(controller.state.allDraftState.hasUnsavedChanges)
        assertFalse(controller.state.dayDraftState.hasUnsavedChanges)

        controller.updateOutputMode(TxtOutputMode.DAY)
        controller.syncResolvedDayBody("0900study\n")
        controller.onEditorTextChange(nextValue = "0900study\n1000break\n")

        assertEquals("0900study\n1000break\n", controller.state.dayDraftState.draftText)
        assertTrue(controller.state.dayDraftState.hasUnsavedChanges)
    }

}
