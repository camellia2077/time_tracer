package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Test

class TxtStructuredDayEditorFormattingTest {
    @Test
    fun formatClockSeconds_returnsIsoTimeForStructuredDayEditRequest() {
        assertEquals("06:24:30", formatClockSeconds(6 * 3_600 + 24 * 60 + 30))
    }

    @Test
    fun formatTxtDayEventTime_rendersIsoPointTimeWithoutDuplicatingSeparators() {
        val event = TxtDayEditEvent(
            isInterval = false,
            startTime = "",
            endTime = "06:24:30",
            activityToken = "wake_up",
            remark = ""
        )

        assertEquals("06:24:30", formatTxtDayEventTime(event))
    }

    @Test
    fun formatTxtDayEventTime_rendersBothIsoIntervalBoundaries() {
        val event = TxtDayEditEvent(
            isInterval = true,
            startTime = "06:24:30",
            endTime = "08:04:03",
            activityToken = "study",
            remark = ""
        )

        assertEquals("06:24:30 – 08:04:03", formatTxtDayEventTime(event))
    }
}
