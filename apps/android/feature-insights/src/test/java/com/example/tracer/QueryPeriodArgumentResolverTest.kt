package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class QueryPeriodArgumentResolverTest {
    private val resolver = QueryPeriodArgumentResolver()

    private val source = QueryPeriodSource(
        dayDigits = "20260214",
        monthDigits = "202602",
        yearDigits = "2026",
        weekDigits = "202607",
        rangeStartDigits = "20260201",
        rangeEndDigits = "20260214",
        recentDays = "7"
    )

    @Test
    fun resolveAndValidate_supportsAllStatsPeriods() {
        val expectedArguments = mapOf(
            DataTreePeriod.DAY to "20260214",
            DataTreePeriod.WEEK to "2026-W07",
            DataTreePeriod.MONTH to "202602",
            DataTreePeriod.YEAR to "2026",
            DataTreePeriod.RECENT to "7",
            DataTreePeriod.RANGE to "2026-02-01|2026-02-14"
        )

        for ((period, expectedArgument) in expectedArguments) {
            val result = resolver.resolveAndValidate(
                period = period,
                source = source,
                subjectLabel = "Stats"
            )
            assertTrue(
                "Expected success for period=${period.wireValue}, actual=$result",
                result is QueryPeriodResolveResult.Success
            )
            assertEquals(
                "Unexpected period argument for period=${period.wireValue}",
                expectedArgument,
                (result as QueryPeriodResolveResult.Success).argument
            )
        }
    }
}
