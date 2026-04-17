package com.example.tracer.ui.components

import android.content.Context
import android.graphics.Typeface
import android.text.Editable
import android.text.InputType
import android.text.TextWatcher
import android.view.Gravity
import android.view.View
import android.view.inputmethod.EditorInfo
import android.widget.EditText
import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.rememberUpdatedState
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.unit.dp
import androidx.compose.ui.viewinterop.AndroidView

internal data class NativeMultilineEditorSnapshot(
    val text: String,
    val selectionStart: Int,
    val selectionEnd: Int
)

private enum class NativeMultilineCoalescingKind {
    INSERT,
    BACKSPACE,
    FORWARD_DELETE,
    OTHER
}

private data class NativeMultilineCoalescingEdit(
    val kind: NativeMultilineCoalescingKind,
    val continuationAnchor: Int,
    val timestampMs: Long
)

internal data class NativeMultilineInputConfig(
    val inputType: Int,
    val imeOptions: Int
)

internal fun buildNativeMultilineInputConfig(): NativeMultilineInputConfig = NativeMultilineInputConfig(
    inputType =
        InputType.TYPE_CLASS_TEXT or
            InputType.TYPE_TEXT_FLAG_MULTI_LINE or
            InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS or
            InputType.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD,
    imeOptions = EditorInfo.IME_FLAG_NO_EXTRACT_UI or EditorInfo.IME_FLAG_NO_PERSONALIZED_LEARNING
)

class NativeMultilineTextEditorController {
    var canUndo by mutableStateOf(false)
        internal set
    var canRedo by mutableStateOf(false)
        internal set

    // These counters must participate in Compose state. The toolbar buttons live in Compose,
    // while undo/redo execution happens inside AndroidView.update. If the request counters are
    // plain Ints, tapping the icons does not trigger recomposition, so the EditText never sees
    // a new request and the arrow buttons appear broken.
    internal var undoRequestCount by mutableStateOf(0)
    internal var redoRequestCount by mutableStateOf(0)

    fun requestUndo() {
        undoRequestCount += 1
    }

    fun requestRedo() {
        redoRequestCount += 1
    }
}

internal class NativeMultilineEditHistory(
    initialText: String,
    private val nowProvider: () -> Long = { System.currentTimeMillis() }
) {
    companion object {
        private const val UndoCoalescingWindowMs: Long = 1_500L
    }

    private val undoStack = ArrayDeque<NativeMultilineEditorSnapshot>()
    private val redoStack = ArrayDeque<NativeMultilineEditorSnapshot>()
    private var lastCoalescingEdit: NativeMultilineCoalescingEdit? = null
    var currentSnapshot: NativeMultilineEditorSnapshot = NativeMultilineEditorSnapshot(
        text = initialText,
        selectionStart = initialText.length,
        selectionEnd = initialText.length
    )
        private set

    val canUndo: Boolean
        get() = undoStack.isNotEmpty()

    val canRedo: Boolean
        get() = redoStack.isNotEmpty()

    fun onUserEdited(
        newText: String,
        selectionStart: Int,
        selectionEnd: Int
    ): Boolean {
        val normalizedSelectionStart = selectionStart.coerceIn(0, newText.length)
        val normalizedSelectionEnd = selectionEnd.coerceIn(0, newText.length)
        if (
            newText == currentSnapshot.text &&
            normalizedSelectionStart == currentSnapshot.selectionStart &&
            normalizedSelectionEnd == currentSnapshot.selectionEnd
        ) {
            return false
        }
        val previousSnapshot = currentSnapshot
        val nextSnapshot = NativeMultilineEditorSnapshot(
            text = newText,
            selectionStart = normalizedSelectionStart,
            selectionEnd = normalizedSelectionEnd
        )
        val nextCoalescingEdit = describeCoalescingEdit(
            previousSnapshot = previousSnapshot,
            nextSnapshot = nextSnapshot
        )
        if (!shouldCoalesce(previousSnapshot, nextCoalescingEdit)) {
            undoStack.addLast(previousSnapshot)
        }
        currentSnapshot = nextSnapshot
        redoStack.clear()
        lastCoalescingEdit = nextCoalescingEdit
        return true
    }

    fun onSelectionChanged(selectionStart: Int, selectionEnd: Int): Boolean {
        val normalizedSelectionStart = selectionStart.coerceIn(0, currentSnapshot.text.length)
        val normalizedSelectionEnd = selectionEnd.coerceIn(0, currentSnapshot.text.length)
        if (
            normalizedSelectionStart == currentSnapshot.selectionStart &&
            normalizedSelectionEnd == currentSnapshot.selectionEnd
        ) {
            return false
        }
        currentSnapshot = currentSnapshot.copy(
            selectionStart = normalizedSelectionStart,
            selectionEnd = normalizedSelectionEnd
        )
        // A manual cursor/selection move should start a fresh undo group. This mirrors note-app
        // editors more closely than character-by-character undo: once users reposition the caret,
        // the next edit is treated as a new editing action instead of extending the previous one.
        lastCoalescingEdit = null
        return true
    }

    fun reset(
        text: String,
        selectionStart: Int = text.length,
        selectionEnd: Int = text.length
    ) {
        currentSnapshot = NativeMultilineEditorSnapshot(
            text = text,
            selectionStart = selectionStart.coerceIn(0, text.length),
            selectionEnd = selectionEnd.coerceIn(0, text.length)
        )
        undoStack.clear()
        redoStack.clear()
        lastCoalescingEdit = null
    }

    fun undo(): NativeMultilineEditorSnapshot? {
        if (undoStack.isEmpty()) {
            return null
        }
        redoStack.addLast(currentSnapshot)
        currentSnapshot = undoStack.removeLast()
        lastCoalescingEdit = null
        return currentSnapshot
    }

    fun redo(): NativeMultilineEditorSnapshot? {
        if (redoStack.isEmpty()) {
            return null
        }
        undoStack.addLast(currentSnapshot)
        currentSnapshot = redoStack.removeLast()
        lastCoalescingEdit = null
        return currentSnapshot
    }

    private fun shouldCoalesce(
        previousSnapshot: NativeMultilineEditorSnapshot,
        nextCoalescingEdit: NativeMultilineCoalescingEdit
    ): Boolean {
        val lastEdit = lastCoalescingEdit ?: return false
        if (lastEdit.kind != nextCoalescingEdit.kind) {
            return false
        }
        if (nextCoalescingEdit.kind == NativeMultilineCoalescingKind.OTHER) {
            return false
        }
        if (nextCoalescingEdit.timestampMs - lastEdit.timestampMs > UndoCoalescingWindowMs) {
            return false
        }
        if (previousSnapshot.selectionStart != previousSnapshot.selectionEnd) {
            return false
        }
        return lastEdit.continuationAnchor == previousSnapshot.selectionStart
    }

    private fun describeCoalescingEdit(
        previousSnapshot: NativeMultilineEditorSnapshot,
        nextSnapshot: NativeMultilineEditorSnapshot
    ): NativeMultilineCoalescingEdit {
        val timestampMs = nowProvider()
        val previousText = previousSnapshot.text
        val nextText = nextSnapshot.text
        val previousSelectionStart = previousSnapshot.selectionStart
        val previousSelectionEnd = previousSnapshot.selectionEnd
        val nextSelectionStart = nextSnapshot.selectionStart
        val nextSelectionEnd = nextSnapshot.selectionEnd
        if (
            previousSelectionStart != previousSelectionEnd ||
            nextSelectionStart != nextSelectionEnd
        ) {
            return NativeMultilineCoalescingEdit(
                kind = NativeMultilineCoalescingKind.OTHER,
                continuationAnchor = nextSelectionEnd,
                timestampMs = timestampMs
            )
        }

        val lengthDelta = nextText.length - previousText.length
        if (
            lengthDelta > 0 &&
            previousText.startsWith(nextText.take(previousSelectionStart)).not()
        ) {
            return NativeMultilineCoalescingEdit(
                kind = NativeMultilineCoalescingKind.OTHER,
                continuationAnchor = nextSelectionEnd,
                timestampMs = timestampMs
            )
        }

        if (
            lengthDelta > 0 &&
            previousSelectionStart <= previousText.length &&
            nextSelectionStart == previousSelectionStart + lengthDelta &&
            nextText.substring(0, previousSelectionStart) ==
                previousText.substring(0, previousSelectionStart) &&
            nextText.substring(previousSelectionStart + lengthDelta) ==
                previousText.substring(previousSelectionStart)
        ) {
            return NativeMultilineCoalescingEdit(
                kind = NativeMultilineCoalescingKind.INSERT,
                continuationAnchor = nextSelectionStart,
                timestampMs = timestampMs
            )
        }

        val removedLength = previousText.length - nextText.length
        if (
            removedLength > 0 &&
            nextSelectionStart == previousSelectionStart - removedLength &&
            nextText.substring(0, nextSelectionStart) ==
                previousText.substring(0, nextSelectionStart) &&
            nextText.substring(nextSelectionStart) ==
                previousText.substring(previousSelectionStart)
        ) {
            return NativeMultilineCoalescingEdit(
                kind = NativeMultilineCoalescingKind.BACKSPACE,
                continuationAnchor = nextSelectionStart,
                timestampMs = timestampMs
            )
        }

        if (
            removedLength > 0 &&
            nextSelectionStart == previousSelectionStart &&
            nextText.substring(0, previousSelectionStart) ==
                previousText.substring(0, previousSelectionStart) &&
            nextText.substring(previousSelectionStart) ==
                previousText.substring(previousSelectionStart + removedLength)
        ) {
            return NativeMultilineCoalescingEdit(
                kind = NativeMultilineCoalescingKind.FORWARD_DELETE,
                continuationAnchor = nextSelectionStart,
                timestampMs = timestampMs
            )
        }

        return NativeMultilineCoalescingEdit(
            kind = NativeMultilineCoalescingKind.OTHER,
            continuationAnchor = nextSelectionEnd,
            timestampMs = timestampMs
        )
    }
}

private class SelectionAwareEditText(context: Context) : EditText(context) {
    var onSelectionChangedCallback: ((Int, Int) -> Unit)? = null

    override fun onSelectionChanged(selStart: Int, selEnd: Int) {
        super.onSelectionChanged(selStart, selEnd)
        onSelectionChangedCallback?.invoke(selStart, selEnd)
    }
}

private class NativeMultilineEditorState {
    var isProgrammaticUpdate: Boolean = false
    var lastProcessedUndoRequestCount: Int = 0
    var lastProcessedRedoRequestCount: Int = 0
    lateinit var historyState: NativeMultilineEditHistory
}

@Composable
fun NativeMultilineTextEditor(
    value: String,
    onValueChange: (String) -> Unit,
    modifier: Modifier = Modifier,
    readOnly: Boolean = false,
    minLines: Int = 8,
    monospace: Boolean = false,
    controller: NativeMultilineTextEditorController? = null
) {
    val currentOnValueChange by rememberUpdatedState(onValueChange)
    val currentController by rememberUpdatedState(controller)
    val density = LocalDensity.current
    val contentPaddingPx = with(density) { 16.dp.roundToPx() }
    val textColor = MaterialTheme.colorScheme.onSurface.toArgb()
    val cursorColor = MaterialTheme.colorScheme.primary.toArgb()
    val textSizeSp = MaterialTheme.typography.bodyMedium.fontSize.value
    val inputConfig = buildNativeMultilineInputConfig()

    Surface(
        modifier = modifier,
        shape = MaterialTheme.shapes.large,
        color = MaterialTheme.colorScheme.surface,
        border = BorderStroke(1.dp, MaterialTheme.colorScheme.outlineVariant)
    ) {
        AndroidView(
            modifier = Modifier.fillMaxSize(),
            factory = { context ->
                val editText = SelectionAwareEditText(context)
                val viewState = NativeMultilineEditorState()
                viewState.historyState = NativeMultilineEditHistory(value)
                editText.tag = viewState
                editText.background = null
                editText.setPadding(
                    contentPaddingPx,
                    contentPaddingPx,
                    contentPaddingPx,
                    contentPaddingPx
                )
                editText.gravity = Gravity.TOP or Gravity.START
                editText.setHorizontallyScrolling(false)
                editText.isVerticalScrollBarEnabled = true
                editText.scrollBarStyle = View.SCROLLBARS_INSIDE_INSET
                editText.overScrollMode = View.OVER_SCROLL_IF_CONTENT_SCROLLS
                // Design intent:
                // TXT behaves like a plain-text note editor, not a chat box. We therefore bias
                // the native EditText toward literal text entry: no fullscreen extract UI, no
                // personalized learning, and no suggestion/autocorrect-friendly text variation.
                // This reduces cases where IMEs inject automatic spaces or "helpful" rewrites
                // that are fine for messaging apps but harmful for authored TXT data.
                editText.imeOptions = inputConfig.imeOptions
                editText.inputType = inputConfig.inputType
                editText.minLines = minLines
                editText.setText(value)
                editText.setSelection(value.length)
                editText.isLongClickable = true
                editText.setAutofillHints(null)
                editText.setRawInputType(inputConfig.inputType)
                editText.onSelectionChangedCallback = { selectionStart, selectionEnd ->
                    if (!viewState.isProgrammaticUpdate) {
                        viewState.historyState.onSelectionChanged(selectionStart, selectionEnd)
                    }
                }
                editText.addTextChangedListener(
                    object : TextWatcher {
                        override fun beforeTextChanged(
                            s: CharSequence?,
                            start: Int,
                            count: Int,
                            after: Int
                        ) = Unit

                        override fun onTextChanged(
                            s: CharSequence?,
                            start: Int,
                            before: Int,
                            count: Int
                        ) = Unit

                        override fun afterTextChanged(s: Editable?) {
                            if (viewState.isProgrammaticUpdate) {
                                return
                            }
                            val newText = s?.toString().orEmpty()
                            if (
                                !viewState.historyState.onUserEdited(
                                    newText = newText,
                                    selectionStart = editText.selectionStart.coerceAtLeast(0),
                                    selectionEnd = editText.selectionEnd.coerceAtLeast(0)
                                )
                            ) {
                                return
                            }
                            syncControllerState(currentController, viewState.historyState)
                            currentOnValueChange(newText)
                        }
                    }
                )
                syncControllerState(currentController, viewState.historyState)
                editText
            },
            update = { editText ->
                val viewState = editText.tag as? NativeMultilineEditorState
                    ?: NativeMultilineEditorState().also { editText.tag = it }
                editText.setTextColor(textColor)
                editText.setTextSize(android.util.TypedValue.COMPLEX_UNIT_SP, textSizeSp)
                editText.highlightColor = cursorColor
                editText.typeface = if (monospace) Typeface.MONOSPACE else Typeface.DEFAULT
                editText.minLines = minLines
                editText.isFocusable = !readOnly
                editText.isFocusableInTouchMode = !readOnly
                editText.isCursorVisible = !readOnly
                editText.setTextIsSelectable(true)
                editText.showSoftInputOnFocus = !readOnly

                val historyController = currentController
                if (historyController != null) {
                    if (viewState.lastProcessedUndoRequestCount != historyController.undoRequestCount) {
                        viewState.lastProcessedUndoRequestCount = historyController.undoRequestCount
                        val undoneSnapshot = viewState.historyState.undo()
                        if (undoneSnapshot != null) {
                            applyProgrammaticText(editText, viewState, undoneSnapshot)
                            syncControllerState(historyController, viewState.historyState)
                            currentOnValueChange(undoneSnapshot.text)
                            return@AndroidView
                        }
                    }
                    if (viewState.lastProcessedRedoRequestCount != historyController.redoRequestCount) {
                        viewState.lastProcessedRedoRequestCount = historyController.redoRequestCount
                        val redoneSnapshot = viewState.historyState.redo()
                        if (redoneSnapshot != null) {
                            applyProgrammaticText(editText, viewState, redoneSnapshot)
                            syncControllerState(historyController, viewState.historyState)
                            currentOnValueChange(redoneSnapshot.text)
                            return@AndroidView
                        }
                    }
                }

                val currentText = editText.text?.toString().orEmpty()
                if (currentText == value) {
                    syncControllerState(historyController, viewState.historyState)
                    return@AndroidView
                }

                // External value changes that do not originate from the current text session
                // should become a new baseline. This resets undo/redo so the editor session
                // matches the currently loaded TXT content instead of replaying stale history.
                val baselineSnapshot = NativeMultilineEditorSnapshot(
                    text = value,
                    selectionStart = value.length,
                    selectionEnd = value.length
                )
                applyProgrammaticText(editText, viewState, baselineSnapshot)
                viewState.historyState.reset(
                    text = baselineSnapshot.text,
                    selectionStart = baselineSnapshot.selectionStart,
                    selectionEnd = baselineSnapshot.selectionEnd
                )
                syncControllerState(historyController, viewState.historyState)
            }
        )
    }
}

private fun applyProgrammaticText(
    editText: EditText,
    viewState: NativeMultilineEditorState,
    snapshot: NativeMultilineEditorSnapshot
) {
    viewState.isProgrammaticUpdate = true
    editText.setText(snapshot.text)
    val textLength = snapshot.text.length
    val safeSelectionStart = snapshot.selectionStart.coerceIn(0, textLength)
    val safeSelectionEnd = snapshot.selectionEnd.coerceIn(0, textLength)
    editText.setSelection(
        minOf(safeSelectionStart, safeSelectionEnd),
        maxOf(safeSelectionStart, safeSelectionEnd)
    )
    viewState.isProgrammaticUpdate = false
}

private fun syncControllerState(
    controller: NativeMultilineTextEditorController?,
    historyState: NativeMultilineEditHistory
) {
    controller ?: return
    controller.canUndo = historyState.canUndo
    controller.canRedo = historyState.canRedo
}

@Suppress("unused")
private val TransparentColor: Int = Color.Transparent.toArgb()
