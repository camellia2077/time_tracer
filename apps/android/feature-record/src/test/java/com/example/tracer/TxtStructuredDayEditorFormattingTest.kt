package com.example.tracer

import java.time.Clock
import java.time.Instant
import java.time.ZoneOffset
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class TxtStructuredDayEditorFormattingTest {
    @Test
    fun formatClockSeconds_returnsIsoTimeForStructuredDayEditRequest() {
        assertEquals("06:24:30", formatClockSeconds(6 * 3_600 + 24 * 60 + 30))
    }

    @Test
    fun currentLogicalDayTimelineSeconds_mapsPreCutoffTimeToThePreviousLogicalDayAxis() {
        val clock = Clock.fixed(Instant.parse("2026-04-17T01:02:03Z"), ZoneOffset.UTC)

        assertEquals(25 * 3_600 + 2 * 60 + 3, currentLogicalDayTimelineSeconds(clock))
    }

    @Test
    fun currentLogicalDayTimelineSeconds_keepsPostCutoffTimeOnTheSameDayAxis() {
        val clock = Clock.fixed(Instant.parse("2026-04-17T06:02:03Z"), ZoneOffset.UTC)

        assertEquals(6 * 3_600 + 2 * 60 + 3, currentLogicalDayTimelineSeconds(clock))
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

    @Test
    fun formatTxtDayEventTime_usesTwelveHourDisplayWithoutChangingStoredTime() {
        val event = TxtDayEditEvent(
            isInterval = true,
            startTime = "00:24:30",
            endTime = "13:04:03",
            activityToken = "study",
            remark = ""
        )

        assertEquals(
            "12:24:30 AM – 1:04:03 PM",
            formatTxtDayEventTime(event, use12HourTime = true)
        )
        assertEquals("00:24:30 – 13:04:03", event.startTime + " – " + event.endTime)
    }

    @Test
    fun canUsePreviousEndAsIntervalStart_acceptsAnEarlierSameDayBoundary() {
        assertTrue(
            canUsePreviousEndAsIntervalStart(
                isInterval = true,
                previousEndTimelineSeconds = 9 * 3_600,
                startTimelineSeconds = 9 * 3_600 + 5 * 60,
                endTimelineSeconds = 10 * 3_600
            )
        )
    }

    @Test
    fun canUsePreviousEndAsIntervalStart_rejectsNoBoundaryChangeOrInvalidRange() {
        assertTrue(
            !canUsePreviousEndAsIntervalStart(
                isInterval = true,
                previousEndTimelineSeconds = null,
                startTimelineSeconds = 9 * 3_600 + 5 * 60,
                endTimelineSeconds = 10 * 3_600
            )
        )
        assertTrue(
            !canUsePreviousEndAsIntervalStart(
                isInterval = true,
                previousEndTimelineSeconds = 9 * 3_600,
                startTimelineSeconds = 9 * 3_600,
                endTimelineSeconds = 10 * 3_600
            )
        )
        assertTrue(
            !canUsePreviousEndAsIntervalStart(
                isInterval = true,
                previousEndTimelineSeconds = 10 * 3_600,
                startTimelineSeconds = 9 * 3_600,
                endTimelineSeconds = 10 * 3_600
            )
        )
        assertTrue(
            !canUsePreviousEndAsIntervalStart(
                isInterval = false,
                previousEndTimelineSeconds = 9 * 3_600,
                startTimelineSeconds = 9 * 3_600 + 5 * 60,
                endTimelineSeconds = 10 * 3_600
            )
        )
    }

    @Test
    fun buildTxtDayActivitySearchOccurrences_matchesCanonicalAndAliasForEitherAuthoredForm() {
        val roots = listOf(
            CanonicalPathNode(
                name = "study",
                path = "study",
                entries = listOf(
                    CanonicalCatalogEntry(
                        canonicalLeaf = "math",
                        canonicalPath = "study_math",
                        sourceFilePath = "study.toml",
                        aliases = listOf("数学")
                    )
                )
            )
        )
        val occurrences = buildTxtDayActivitySearchOccurrences(
            events = listOf(
                TxtDayEditEvent(false, "", "09:00:00", "数学", ""),
                TxtDayEditEvent(false, "", "10:00:00", "study_math", "")
            ),
            roots = roots
        )

        assertEquals(2, occurrences.size)
        occurrences.forEach { occurrence ->
            assertTrue(occurrence.searchTokens.contains("数学"))
            assertTrue(occurrence.searchTokens.contains("study_math"))
        }
    }
}
