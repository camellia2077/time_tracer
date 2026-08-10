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
        val outputTxt = root.resolve("output/insights.txt").apply {
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

    @Test
    fun clearEditableData_removesTxtAndHierarchyButPreservesProgramDataAndDatabase() {
        val root = Files.createTempDirectory("runtime-clean-editable").toFile()
        val txtFile = root.resolve("input/2026/2026-03.txt").apply {
            parentFile?.mkdirs()
            writeText("txt")
        }
        val hierarchyFile = root.resolve("config/user/activity_hierarchy/custom.toml").apply {
            parentFile?.mkdirs()
            writeText("hierarchy")
        }
        val quickAccessFile = root.resolve("config/user/quick_access.toml").apply {
            parentFile?.mkdirs()
            writeText("quick_access = [\"study\"]")
        }
        val behaviorFile = root.resolve("config/user/behavior.toml").apply {
            writeText("keep")
        }
        val nonTomlHierarchyFile = root.resolve("config/user/activity_hierarchy/notes.txt").apply {
            writeText("keep")
        }
        val marker = root.resolve(DATA_FOLDER_SNAPSHOT_MARKER).apply { writeText("snapshot") }
        val programFile = root.resolve("config/program/config.toml").apply {
            parentFile?.mkdirs()
            writeText("keep")
        }
        val databaseFile = root.resolve("db/time_data.sqlite3").apply {
            parentFile?.mkdirs()
            writeText("keep")
        }

        val message = RuntimeDataCleanupTargets.clearEditableData(listOf(root))

        assertFalse(txtFile.exists())
        assertFalse(hierarchyFile.exists())
        assertFalse(quickAccessFile.exists())
        assertFalse(marker.exists())
        assertTrue(nonTomlHierarchyFile.exists())
        assertTrue(programFile.exists())
        assertTrue(databaseFile.exists())
        assertTrue(behaviorFile.exists())
        assertTrue(message.contains("1 TXT file(s)"))
        assertTrue(message.contains("1 activity_hierarchy TOML file(s)"))
        assertTrue(message.contains("1 Quick Access TOML file(s)"))
    }

    @Test
    fun clearAllData_removesEditableDataAndDatabase() {
        val root = Files.createTempDirectory("runtime-clean-all").toFile()
        val txtFile = root.resolve("input/current.txt").apply {
            parentFile?.mkdirs()
            writeText("activity")
        }
        val hierarchyFile = root.resolve("config/user/activity_hierarchy/custom.toml").apply {
            parentFile?.mkdirs()
            writeText("category")
        }
        val quickAccessFile = root.resolve("config/user/quick_access.toml").apply {
            parentFile?.mkdirs()
            writeText("quick_access = [\"study\"]")
        }
        val databaseFile = root.resolve("db/time_data.sqlite3").apply {
            parentFile?.mkdirs()
            writeText("database")
        }
        val programFile = root.resolve("config/program/config.toml").apply {
            parentFile?.mkdirs()
            writeText("program")
        }

        RuntimeDataCleanupTargets.clearEditableData(listOf(root))
        val databaseResult = RuntimeDataCleanupTargets.clearDatabaseData(listOf(root))

        assertTrue(databaseResult.ok)
        assertFalse(txtFile.exists())
        assertFalse(hierarchyFile.exists())
        assertFalse(quickAccessFile.exists())
        assertFalse(databaseFile.exists())
        assertTrue(programFile.exists())
    }
}
