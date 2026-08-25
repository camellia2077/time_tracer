package com.example.tracer

import java.time.LocalDate
import org.junit.Assert.assertEquals
import org.junit.Test

class TxtRawMonthNavigationTest {
    @Test
    fun findsExactDayMarkerAtStartOfLine() {
        val content = """
            d0803
            0900study
            d0804
            1000rest
        """.trimIndent()

        assertEquals(
            content.indexOf("d0804"),
            findRawMonthDayMarkerOffset(content, LocalDate.of(2026, 8, 4))
        )
    }

    @Test
    fun ignoresDayMarkerTextThatIsNotAMarkerLine() {
        val content = "// d0804\nd0805\n0900study"

        assertEquals(-1, findRawMonthDayMarkerOffset(content, LocalDate.of(2026, 8, 4)))
    }
}
