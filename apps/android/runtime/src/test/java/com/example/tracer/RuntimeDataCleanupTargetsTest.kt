package com.example.tracer

import java.nio.file.Files
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class RuntimeDataCleanupTargetsTest {
    @Test
    fun clearDatabaseData_removesOnlyWhitelistedDatabaseFiles() {
        val root = Files.createTempDirectory("runtime-clean-db").toFile()
        val databaseDir = root.resolve("db").apply { mkdirs() }
        val databaseFile = databaseDir.resolve("time_data.sqlite3").apply { writeText("db") }
        val walFile = databaseDir.resolve("time_data.sqlite3-wal").apply { writeText("wal") }
        val shmFile = databaseDir.resolve("time_data.sqlite3-shm").apply { writeText("shm") }
        val journalFile = databaseDir.resolve("time_data.sqlite3-journal").apply { writeText("journal") }
        val foreignDatabaseFile = databaseDir.resolve("custom.sqlite3").apply { writeText("keep") }
        val configFile = root.resolve("config/meta/bundle.toml").apply {
            parentFile?.mkdirs()
            writeText("keep")
        }

        val result = RuntimeDataCleanupTargets.clearDatabaseData(listOf(root))

        assertTrue(result.ok)
        assertFalse(databaseFile.exists())
        assertFalse(walFile.exists())
        assertFalse(shmFile.exists())
        assertFalse(journalFile.exists())
        assertTrue(foreignDatabaseFile.exists())
        assertTrue(configFile.exists())
        assertTrue(databaseDir.exists())
    }

    @Test
    fun clearTxtData_removesOnlyWhitelistedTxtFiles() {
        val root = Files.createTempDirectory("runtime-clean-txt").toFile()
        val monthTxt = root.resolve("input/2026/2026-03.txt").apply {
            parentFile?.mkdirs()
            writeText("month")
        }
        val secondMonthTxt = root.resolve("input/2027/2027-01.txt").apply {
            parentFile?.mkdirs()
            writeText("keep removing")
        }
        val nonTxtFile = root.resolve("input/2026/2026-03.json").apply {
            parentFile?.mkdirs()
            writeText("keep")
        }
        val cacheTxt = root.resolve("cache/validate/scratch.txt").apply {
            parentFile?.mkdirs()
            writeText("keep")
        }
        val outputTxt = root.resolve("output/report.txt").apply {
            parentFile?.mkdirs()
            writeText("keep")
        }

        val result = RuntimeDataCleanupTargets.clearTxtData(listOf(root))

        assertTrue(result.ok)
        assertFalse(monthTxt.exists())
        assertFalse(secondMonthTxt.exists())
        assertTrue(nonTxtFile.exists())
        assertTrue(cacheTxt.exists())
        assertTrue(outputTxt.exists())
    }
}
