package com.example.tracer

import androidx.lifecycle.Lifecycle
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class RecordElapsedClockTest {
    private val start = 1_000_000L
    private var now = start + 15_000L

    private fun clock() = RecordElapsedClock(start, Lifecycle.State.RESUMED) { now }

    @Test
    fun wakeRefreshesBeforeAnyFrameOrCoroutineRuns() {
        val clock = clock()
        clock.onLifecycleEvent(Lifecycle.Event.ON_PAUSE)
        clock.onLifecycleEvent(Lifecycle.Event.ON_STOP)
        now = start + 145_500L

        clock.onLifecycleEvent(Lifecycle.Event.ON_START)
        assertEquals(145_500L, clock.frameElapsedMillis)
        assertFalse(clock.isActive)
        clock.onLifecycleEvent(Lifecycle.Event.ON_RESUME)
        assertTrue(clock.isActive)
        assertEquals(145_500L, clock.elapsedMillisForDraw())
        assertEquals(2f / 60f, hourCycleProgressForElapsedSeconds(clock.frameElapsedMillis / 1000))
        assertEquals(25_500f / 60_000f, minuteCycleProgressForElapsedMillis(clock.elapsedMillisForDraw()))
    }

    @Test
    fun resumeWithoutStopAlsoRefreshesSynchronously() {
        val clock = clock()
        clock.onLifecycleEvent(Lifecycle.Event.ON_PAUSE)
        now = start + 70_000L
        clock.onLifecycleEvent(Lifecycle.Event.ON_RESUME)
        assertEquals(70_000L, clock.frameElapsedMillis)
    }

    @Test
    fun queuedDrawSamplesCurrentTimeEvenBeforeNextFrame() {
        val clock = clock()
        // Frame delivery can stall independently of Activity lifecycle (window/lock screen).
        now = start + 3_610_250L
        val elapsed = clock.elapsedMillisForDraw()
        assertEquals(3_610_250L, elapsed)
        assertEquals(0f, hourCycleProgressForElapsedSeconds(elapsed / 1000))
        assertEquals(10_250f / 60_000f, minuteCycleProgressForElapsedMillis(elapsed))
    }

    @Test
    fun backgroundFramesDoNotTickAndForegroundFramesNeverCatchUpFromOldValue() {
        val clock = clock()
        clock.onLifecycleEvent(Lifecycle.Event.ON_PAUSE)
        now = start + 120_000L
        clock.onFrame()
        assertEquals(15_000L, clock.frameElapsedMillis)
        clock.onLifecycleEvent(Lifecycle.Event.ON_RESUME)
        now += 16L
        clock.onFrame()
        assertEquals(120_016L, clock.frameElapsedMillis)
    }

    @Test
    fun recreationImmediatelyUsesPersistedStart() {
        now = start + 7_230_000L
        assertEquals(7_230_000L, clock().frameElapsedMillis)
    }

    @Test
    fun absentStartAndClockBeforeStartHaveZeroProgress() {
        assertEquals(0L, RecordElapsedClock(0L, Lifecycle.State.RESUMED) { now }.elapsedMillisForDraw())
        now = start - 1000L
        val clock = clock()
        assertEquals(0L, clock.frameElapsedMillis)
        assertEquals(0L, clock.elapsedMillisForDraw())
    }
}
