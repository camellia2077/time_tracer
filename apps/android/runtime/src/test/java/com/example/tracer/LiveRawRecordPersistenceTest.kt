package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test
import java.io.File
import java.nio.file.Files

class LiveRawRecordPersistenceTest {
    @Test
    fun ensureRawMonthFile_createsFileWithExpectedHeaders() {
        val root = Files.createTempDirectory("runtime-persistence-month").toFile()
        try {
            val target = File(root, "input/2026/2026-03.txt")
            val persistence = createPersistence()

            persistence.ensureRawMonthFile(target, year = 2026, month = 3)

            assertTrue(target.exists())
            assertEquals("y2026\nm03\n\n", target.readText())
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun ensureRawMonthFile_whenFileExists_keepsOriginalContent() {
        val root = Files.createTempDirectory("runtime-persistence-existing").toFile()
        try {
            val target = File(root, "input/2026/2026-03.txt")
            target.parentFile?.mkdirs()
            target.writeText("y2024\nm11\nd1101\n0800work\n")
            val original = target.readText()
            val persistence = createPersistence()

            persistence.ensureRawMonthFile(target, year = 2026, month = 3)

            assertEquals(original, target.readText())
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun buildRawIntervalEventLine_formatsWithoutRemark() {
        val persistence = createPersistence()

        val line = persistence.buildRawIntervalEventLine(
            startIsoTime = "09:00:00",
            endIsoTime = "10:30:00",
            activity = "study",
            remark = ""
        )

        assertEquals("090000-103000study", line)
    }

    @Test
    fun buildRawIntervalEventLine_formatsWithRemark() {
        val persistence = createPersistence()

        val line = persistence.buildRawIntervalEventLine(
            startIsoTime = "09:00:00",
            endIsoTime = "10:30:00",
            activity = "study",
            remark = "focus"
        )

        assertEquals("090000-103000study // focus", line)
    }

    @Test
    fun buildRawEventLine_escapesMultilineRemarkAndBackslash() {
        val persistence = createPersistence()

        val line = persistence.buildRawEventLine(
            isoTime = "09:00:00",
            activity = "study",
            remark = "first line\npath C:\\work"
        )

        assertEquals("090000study // first line\\npath C:\\\\work", line)
    }

    @Test
    fun parsing_extractsActivityAndEndBoundaryFromIntervalLine() {
        val normalization = LiveRawRecordNormalization()
        val parsing = LiveRawRecordParsing(normalization)

        assertEquals("103000", parsing.extractEventTimeToken("090000-103000study // focus"))
        assertEquals("study", parsing.extractActivityName("090000-103000study // focus"))
    }

    @Test
    fun insertEventIntoDayBlock_acceptsLaterSecondWithinSameMinute() {
        val root = Files.createTempDirectory("runtime-persistence-seconds").toFile()
        try {
            val target = File(root, "input/2026/2026-07.txt").apply {
                parentFile?.mkdirs()
                writeText("y2026\nm07\n\nd0703\n110600first\n")
            }
            val persistence = createPersistence()

            persistence.insertEventIntoDayBlock(
                monthFile = target,
                dayMarker = "0703",
                eventLine = "110608second",
                eventTime = "110608",
                normalizedActivity = "second"
            ) { _, _ -> true }

            assertTrue(target.readText().contains("110608second"))
        } finally {
            root.deleteRecursively()
        }
    }

    private fun createPersistence(): LiveRawRecordPersistence {
        val normalization = LiveRawRecordNormalization()
        val parsing = LiveRawRecordParsing(normalization)
        return LiveRawRecordPersistence(parsing)
    }
}
