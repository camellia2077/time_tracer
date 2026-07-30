package com.example.tracer

import java.nio.file.Files
import org.junit.Assert.assertEquals
import org.junit.Test

class RuntimeSqliteDatabaseSwapTest {
    @Test
    fun replaceAndRestoreKeepSqliteSidecarsTogether() {
        val root = Files.createTempDirectory("time-tracer-db-swap").toFile()
        try {
            val active = root.resolve("active/time_data.sqlite3")
            val candidate = root.resolve("candidate/time_data.sqlite3")
            val transaction = root.resolve("transaction")
            requireNotNull(active.parentFile).mkdirs()
            requireNotNull(candidate.parentFile).mkdirs()
            active.writeText("old")
            active.resolveSibling("time_data.sqlite3-wal").writeText("old-wal")
            active.resolveSibling("time_data.sqlite3-journal").writeText("old-journal")
            candidate.writeText("new")
            candidate.resolveSibling("time_data.sqlite3-shm").writeText("new-shm")
            candidate.resolveSibling("time_data.sqlite3-wal").writeText("new-wal")

            val swap = RuntimeSqliteDatabaseSwap()
            swap.replace(active.absolutePath, candidate.absolutePath, transaction)

            assertEquals("new", active.readText())
            assertEquals("new-shm", active.resolveSibling("time_data.sqlite3-shm").readText())
            assertEquals("new-wal", active.resolveSibling("time_data.sqlite3-wal").readText())

            swap.restore(active.absolutePath, transaction)

            assertEquals("old", active.readText())
            assertEquals("old-wal", active.resolveSibling("time_data.sqlite3-wal").readText())
            assertEquals("old-journal", active.resolveSibling("time_data.sqlite3-journal").readText())
        } finally {
            root.deleteRecursively()
        }
    }
}
