package com.example.tracer

import androidx.compose.ui.text.TextRange
import androidx.compose.ui.text.input.TextFieldValue
import org.junit.Assert.assertEquals
import org.junit.Test

class RecordInputContentTest {
    @Test
    fun hourCycleProgress_advancesOnlyAfterAFullMinute() {
        assertEquals(0f, hourCycleProgressForElapsedSeconds(1L))
        assertEquals(1f / 60f, hourCycleProgressForElapsedSeconds(60L))
    }

    @Test
    fun elapsedMinuteProgress_includesSubsecondPhase() {
        assertEquals(0.5f, minuteCycleProgressForElapsedMillis(30_000L))
        assertEquals(0.5005f, minuteCycleProgressForElapsedMillis(30_030L))
    }

    @Test
    fun elapsedMinuteProgress_wrapsAtTheNextMinute() {
        assertEquals(0f, minuteCycleProgressForElapsedMillis(60_000L))
        assertEquals(0.5f, minuteCycleProgressForElapsedMillis(90_000L))
    }

    @Test
    fun minuteRollover_hasNoPreviousLapOnInitialStart() {
        for (elapsed in listOf(-1L, 0L, 150L, 30_000L, 59_999L)) {
            assertEquals(0f, minuteCycleRolloverAlpha(elapsed))
        }
    }

    @Test
    fun minuteRollover_keepsCompletedLapAtBoundaryThenFadesItOut() {
        assertEquals(1f, minuteCycleRolloverAlpha(60_000L))
        assertEquals(0.5f, minuteCycleRolloverAlpha(60_150L))
        assertEquals(0f, minuteCycleRolloverAlpha(60_300L))
        assertEquals(0f, minuteCycleRolloverAlpha(61_000L))
    }

    @Test
    fun minuteRollover_newLapMovesDuringFadeWithoutHoldingOrSkippingTime() {
        for (millis in 0L..300L step 16L) {
            val elapsed = 60_000L + millis
            assertEquals(millis / 60_000f, minuteCycleProgressForElapsedMillis(elapsed))
        }
    }

    @Test
    fun minuteRollover_samplesSamePhaseAfterSkippedLapsAndHourBoundary() {
        for (start in listOf(120_000L, 3_600_000L, 7_200_000L)) {
            assertEquals(1f, minuteCycleRolloverAlpha(start))
            assertEquals(0.5f, minuteCycleRolloverAlpha(start + 150L))
            assertEquals(0f, minuteCycleRolloverAlpha(start + 5_000L))
            assertEquals(150f / 60_000f, minuteCycleProgressForElapsedMillis(start + 150L))
        }
    }

    @Test
    fun syncActivityNameInputValue_movesCursorToEndWhenContentChangesExternally() {
        val updatedValue = syncActivityNameInputValue(
            currentValue = TextFieldValue(text = "draft", selection = TextRange.Zero),
            recordContent = "quick activity"
        )

        assertEquals("quick activity", updatedValue.text)
        assertEquals(TextRange("quick activity".length), updatedValue.selection)
    }

    @Test
    fun syncActivityNameInputValue_preservesUserSelectionWhenContentIsUnchanged() {
        val currentValue = TextFieldValue(text = "draft", selection = TextRange(2))

        val updatedValue = syncActivityNameInputValue(
            currentValue = currentValue,
            recordContent = "draft"
        )

        assertEquals(currentValue, updatedValue)
    }
}
