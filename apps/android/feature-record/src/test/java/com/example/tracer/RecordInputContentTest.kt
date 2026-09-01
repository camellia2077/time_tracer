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
