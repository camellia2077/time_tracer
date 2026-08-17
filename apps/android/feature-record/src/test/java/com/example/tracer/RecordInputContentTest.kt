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
    fun elapsedMinuteProgress_snapsToCurrentPhaseAfterForegroundResume() {
        assertEquals(
            true,
            shouldSnapElapsedMinuteProgress(
                needsForegroundResync = true,
                currentPosition = 12f,
                targetPosition = 15f
            )
        )
    }

    @Test
    fun elapsedMinuteProgress_keepsAnimatingForVisibleTimerUpdates() {
        assertEquals(
            false,
            shouldSnapElapsedMinuteProgress(
                needsForegroundResync = false,
                currentPosition = 12f,
                targetPosition = 12.016667f
            )
        )
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
