package com.example.tracer

import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableLongStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.runtime.withFrameNanos
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.LifecycleEventObserver
import androidx.lifecycle.compose.LocalLifecycleOwner

/** Presentation clock anchored to the persisted interval start, never to a previous frame. */
internal class RecordElapsedClock(
    private val startedAtEpochMs: Long,
    initialLifecycleState: Lifecycle.State,
    private val nowMillis: () -> Long = System::currentTimeMillis
) {
    var frameElapsedMillis by mutableLongStateOf(sampleElapsedMillis())
        private set
    var isActive by mutableStateOf(initialLifecycleState.isAtLeast(Lifecycle.State.RESUMED))
        private set

    fun onLifecycleEvent(event: Lifecycle.Event) {
        // Refresh synchronously, before Compose can draw a resumed window. A LaunchedEffect
        // alone starts too late and may leave the first visible frame at the old position.
        if (event == Lifecycle.Event.ON_START || event == Lifecycle.Event.ON_RESUME) {
            frameElapsedMillis = sampleElapsedMillis()
        }
        if (event != Lifecycle.Event.ON_ANY) {
            isActive = event.targetState.isAtLeast(Lifecycle.State.RESUMED)
        }
    }

    fun onFrame() {
        if (isActive) frameElapsedMillis = sampleElapsedMillis()
    }

    fun elapsedMillisForDraw(): Long {
        // Subscribe drawing to frame changes, but sample at draw time as well: a queued draw
        // must remain correct even if frame delivery paused without a lifecycle transition.
        frameElapsedMillis
        return sampleElapsedMillis()
    }

    private fun sampleElapsedMillis(): Long = if (startedAtEpochMs > 0L) {
        (nowMillis() - startedAtEpochMs).coerceAtLeast(0L)
    } else {
        0L
    }
}

@Composable
internal fun rememberRecordElapsedClock(startedAtEpochMs: Long): RecordElapsedClock {
    val owner = LocalLifecycleOwner.current
    val clock = remember(startedAtEpochMs, owner) {
        RecordElapsedClock(startedAtEpochMs, owner.lifecycle.currentState)
    }
    DisposableEffect(owner, clock) {
        val observer = LifecycleEventObserver { _, event -> clock.onLifecycleEvent(event) }
        owner.lifecycle.addObserver(observer)
        onDispose { owner.lifecycle.removeObserver(observer) }
    }
    LaunchedEffect(clock, clock.isActive) {
        while (clock.isActive) {
            // Update inside the frame callback, before the frame's drawing work.
            withFrameNanos { clock.onFrame() }
        }
    }
    return clock
}
