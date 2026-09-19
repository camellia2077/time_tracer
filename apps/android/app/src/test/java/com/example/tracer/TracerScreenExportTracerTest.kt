package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Test
import java.time.LocalDateTime

class TracerScreenExportTracerTest {
    @Test
    fun buildTimestampedDataExportName_usesSharedDataNaming() {
        assertEquals(
            "data_2026-09-19_08-12-00",
            buildTimestampedDataExportName(LocalDateTime.of(2026, 9, 19, 8, 12, 0))
        )
    }
}
