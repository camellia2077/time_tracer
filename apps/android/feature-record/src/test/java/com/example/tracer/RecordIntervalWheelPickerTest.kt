package com.example.tracer

import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class RecordIntervalWheelPickerTest {
    @Test
    fun wheelPickerNeedsPositionReset_returnsFalseForAnExactlyCenteredTarget() {
        assertFalse(
            wheelPickerNeedsPositionReset(
                firstVisibleItemIndex = 17,
                firstVisibleItemScrollOffset = 0,
                targetIndex = 17
            )
        )
    }

    @Test
    fun wheelPickerNeedsPositionReset_returnsTrueWhenTheTargetIndexHasResidualOffset() {
        assertTrue(
            wheelPickerNeedsPositionReset(
                firstVisibleItemIndex = 17,
                firstVisibleItemScrollOffset = 11,
                targetIndex = 17
            )
        )
    }

    @Test
    fun wheelPickerNeedsPositionReset_returnsTrueWhenTheSelectedItemChanged() {
        assertTrue(
            wheelPickerNeedsPositionReset(
                firstVisibleItemIndex = 17,
                firstVisibleItemScrollOffset = 0,
                targetIndex = 18
            )
        )
    }

    @Test
    fun wheelPickerTextEmphasis_fadesContinuouslyWithDistanceFromTheCenter() {
        val center = wheelPickerTextEmphasis(
            itemCenterOffsetPx = 72,
            viewportCenterOffsetPx = 72,
            itemHeightPx = 48
        )
        val oneRowAway = wheelPickerTextEmphasis(
            itemCenterOffsetPx = 120,
            viewportCenterOffsetPx = 72,
            itemHeightPx = 48
        )
        val twoRowsAway = wheelPickerTextEmphasis(
            itemCenterOffsetPx = 168,
            viewportCenterOffsetPx = 72,
            itemHeightPx = 48
        )

        assertTrue(center > oneRowAway)
        assertTrue(oneRowAway > twoRowsAway)
    }

    @Test
    fun wheelPickerViewportCenterOffset_accountsForVerticalContentPadding() {
        assertTrue(wheelPickerViewportCenterOffset(-48, 96) == 24)
    }
}
