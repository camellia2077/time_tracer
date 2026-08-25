package com.example.tracer.ui.components

import org.junit.Assert.assertEquals
import org.junit.Test

class NativeMultilineTextEditorControllerTest {
    @Test
    fun requestUndoAndRedo_advanceRequestCounters() {
        val controller = NativeMultilineTextEditorController()

        controller.requestUndo()
        controller.requestUndo()
        controller.requestRedo()

        assertEquals(2, controller.undoRequestCount)
        assertEquals(1, controller.redoRequestCount)
    }

    @Test
    fun requestSelection_recordsRangeAndAdvancesCounter() {
        val controller = NativeMultilineTextEditorController()

        controller.requestSelection(start = 12, end = 17)

        assertEquals(1, controller.selectionRequestCount)
        assertEquals(12, controller.requestedSelectionStart)
        assertEquals(17, controller.requestedSelectionEnd)
    }
}
