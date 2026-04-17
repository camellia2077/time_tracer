package com.example.tracer.ui.components

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Test

class NativeMultilineEditHistoryTest {
    @Test
    fun initialState_hasNoUndoOrRedo() {
        val history = NativeMultilineEditHistory(initialText = "base")

        assertFalse(history.canUndo)
        assertFalse(history.canRedo)
        assertEquals(
            NativeMultilineEditorSnapshot(
                text = "base",
                selectionStart = 4,
                selectionEnd = 4
            ),
            history.currentSnapshot
        )
    }

    @Test
    fun userEdit_enablesUndo_only() {
        val history = NativeMultilineEditHistory(initialText = "base")

        val changed = history.onUserEdited(
            newText = "base-1",
            selectionStart = 6,
            selectionEnd = 6
        )

        assertTrue(changed)
        assertTrue(history.canUndo)
        assertFalse(history.canRedo)
        assertEquals(
            NativeMultilineEditorSnapshot(
                text = "base-1",
                selectionStart = 6,
                selectionEnd = 6
            ),
            history.currentSnapshot
        )
    }

    @Test
    fun continuousTyping_isCoalescedIntoSingleUndoStep() {
        var nowMs = 0L
        val history = NativeMultilineEditHistory(
            initialText = "a",
            nowProvider = { nowMs }
        )

        history.onUserEdited(
            newText = "ab",
            selectionStart = 2,
            selectionEnd = 2
        )
        nowMs = 100L
        history.onUserEdited(
            newText = "abc",
            selectionStart = 3,
            selectionEnd = 3
        )

        val undone = history.undo()

        assertEquals(
            NativeMultilineEditorSnapshot(
                text = "a",
                selectionStart = 1,
                selectionEnd = 1
            ),
            undone
        )
    }

    @Test
    fun typingAfterCaretMove_startsNewUndoStep() {
        var nowMs = 0L
        val history = NativeMultilineEditHistory(
            initialText = "ab",
            nowProvider = { nowMs }
        )

        history.onUserEdited(
            newText = "abc",
            selectionStart = 3,
            selectionEnd = 3
        )
        history.onSelectionChanged(selectionStart = 1, selectionEnd = 1)
        nowMs = 100L
        history.onUserEdited(
            newText = "axbc",
            selectionStart = 2,
            selectionEnd = 2
        )

        val firstUndo = history.undo()
        val secondUndo = history.undo()

        assertEquals(
            NativeMultilineEditorSnapshot(
                text = "abc",
                selectionStart = 1,
                selectionEnd = 1
            ),
            firstUndo
        )
        assertEquals(
            NativeMultilineEditorSnapshot(
                text = "ab",
                selectionStart = 2,
                selectionEnd = 2
            ),
            secondUndo
        )
    }

    @Test
    fun typingAfterCoalescingWindow_startsNewUndoStep() {
        var nowMs = 0L
        val history = NativeMultilineEditHistory(
            initialText = "a",
            nowProvider = { nowMs }
        )

        history.onUserEdited(
            newText = "ab",
            selectionStart = 2,
            selectionEnd = 2
        )
        nowMs = 2_000L
        history.onUserEdited(
            newText = "abc",
            selectionStart = 3,
            selectionEnd = 3
        )

        val firstUndo = history.undo()
        val secondUndo = history.undo()

        assertEquals(
            NativeMultilineEditorSnapshot(
                text = "ab",
                selectionStart = 2,
                selectionEnd = 2
            ),
            firstUndo
        )
        assertEquals(
            NativeMultilineEditorSnapshot(
                text = "a",
                selectionStart = 1,
                selectionEnd = 1
            ),
            secondUndo
        )
    }

    @Test
    fun undo_thenRedo_restoresExpectedStatesAndSelection() {
        val history = NativeMultilineEditHistory(initialText = "base")
        history.onUserEdited(
            newText = "base-1",
            selectionStart = 2,
            selectionEnd = 2
        )

        val undone = history.undo()

        assertEquals(
            NativeMultilineEditorSnapshot(
                text = "base",
                selectionStart = 4,
                selectionEnd = 4
            ),
            undone
        )
        assertFalse(history.canUndo)
        assertTrue(history.canRedo)

        val redone = history.redo()

        assertEquals(
            NativeMultilineEditorSnapshot(
                text = "base-1",
                selectionStart = 2,
                selectionEnd = 2
            ),
            redone
        )
        assertTrue(history.canUndo)
        assertFalse(history.canRedo)
    }

    @Test
    fun newEditAfterUndo_clearsRedoChain() {
        val history = NativeMultilineEditHistory(initialText = "base")
        history.onUserEdited(
            newText = "base-1",
            selectionStart = 5,
            selectionEnd = 5
        )
        history.undo()

        history.onUserEdited(
            newText = "base-2",
            selectionStart = 3,
            selectionEnd = 3
        )

        assertTrue(history.canUndo)
        assertFalse(history.canRedo)
        assertNull(history.redo())
    }

    @Test
    fun reset_clearsUndoAndRedoHistory() {
        val history = NativeMultilineEditHistory(initialText = "base")
        history.onUserEdited(
            newText = "base-1",
            selectionStart = 5,
            selectionEnd = 5
        )
        history.undo()

        history.reset(text = "saved", selectionStart = 1, selectionEnd = 1)

        assertFalse(history.canUndo)
        assertFalse(history.canRedo)
        assertNull(history.undo())
        assertNull(history.redo())
        assertEquals(
            NativeMultilineEditorSnapshot(
                text = "saved",
                selectionStart = 1,
                selectionEnd = 1
            ),
            history.currentSnapshot
        )
    }

    @Test
    fun selectionChange_updatesCurrentSnapshot_withoutCreatingUndoStep() {
        val history = NativeMultilineEditHistory(initialText = "base")

        val changed = history.onSelectionChanged(selectionStart = 1, selectionEnd = 3)

        assertTrue(changed)
        assertFalse(history.canUndo)
        assertFalse(history.canRedo)
        assertEquals(
            NativeMultilineEditorSnapshot(
                text = "base",
                selectionStart = 1,
                selectionEnd = 3
            ),
            history.currentSnapshot
        )
    }
}
