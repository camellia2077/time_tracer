package com.example.tracer

import kotlinx.coroutines.CompletableDeferred
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.test.advanceTimeBy
import kotlinx.coroutines.test.advanceUntilIdle
import kotlinx.coroutines.test.runCurrent
import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Test

@OptIn(ExperimentalCoroutinesApi::class)
class LatestValueAutoSaveCoordinatorTest {
    @Test
    fun savesOnlyTheNewestValueAfterTheTypingDebounce() = runTest {
        val writes = mutableListOf<String>()
        val coordinator = LatestValueAutoSaveCoordinator<String>(
            scope = this,
            debounceMillis = 800,
            save = { value ->
                writes += value
                RecordActionResult(ok = true, message = "saved")
            },
            onLatestSaveFinished = {}
        )

        coordinator.schedule("a")
        advanceTimeBy(500)
        coordinator.schedule("ab")
        advanceTimeBy(799)
        assertEquals(emptyList<String>(), writes)

        advanceTimeBy(1)
        advanceUntilIdle()

        assertEquals(listOf("ab"), writes)
    }

    @Test
    fun flushQueuesOnlyTheLatestValueAfterAnInFlightWrite() = runTest {
        val writes = mutableListOf<String>()
        val firstWriteCanFinish = CompletableDeferred<Unit>()
        val coordinator = LatestValueAutoSaveCoordinator<String>(
            scope = this,
            debounceMillis = 800,
            save = { value ->
                writes += value
                if (value == "first") firstWriteCanFinish.await()
                RecordActionResult(ok = true, message = "saved")
            },
            onLatestSaveFinished = {}
        )

        coordinator.schedule("first")
        advanceTimeBy(800)
        runCurrent()
        coordinator.schedule("second")
        coordinator.flush()
        runCurrent()

        assertEquals(listOf("first"), writes)
        firstWriteCanFinish.complete(Unit)
        advanceUntilIdle()

        assertEquals(listOf("first", "second"), writes)
    }
}
