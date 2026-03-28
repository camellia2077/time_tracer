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
            target.writeText("y2024\nm11\n0101\n0800work\n")
            val original = target.readText()
            val persistence = createPersistence()

            persistence.ensureRawMonthFile(target, year = 2026, month = 3)

            assertEquals(original, target.readText())
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
