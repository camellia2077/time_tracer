package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Test

class InputRecordStoreCompletenessTest {
    private val store = InputRecordStore()

    @Test
    fun changedCompleteDayDoesNotWarnBecauseAnotherDayInMonthIsIncomplete() {
        val previous = """
            y2026
            m03
            d0301
            0700wake
            0800work
            0900break
            d0302
            0700wake
        """.trimIndent()
        val current = previous.replace("0900break\n", "")

        assertEquals(
            null,
            store.resolveCompletenessWarningForChangedDays(
                previousContent = previous,
                content = current
            )
        )
    }

    @Test
    fun changedIncompleteDayStillWarns() {
        val previous = """
            y2026
            m03
            d0301
            0700wake
            0800work
        """.trimIndent()
        val current = previous.replace("0800work", "")

        assertEquals(
            InputRecordStore.INCOMPLETE_DAY_WARNING,
            store.resolveCompletenessWarningForChangedDays(
                previousContent = previous,
                content = current
            )
        )
    }
}
