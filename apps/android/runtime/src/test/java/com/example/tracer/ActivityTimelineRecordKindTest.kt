package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertThrows
import org.junit.Test

class ActivityTimelineRecordKindTest {
    @Test
    fun fromWireValue_requiresKnownRecordKind() {
        assertEquals(
            ActivityTimelineRecordKind.END_ONLY,
            ActivityTimelineRecordKind.fromWireValue("end_only")
        )
        assertThrows(IllegalStateException::class.java) {
            ActivityTimelineRecordKind.fromWireValue("legacy_value")
        }
    }
}
