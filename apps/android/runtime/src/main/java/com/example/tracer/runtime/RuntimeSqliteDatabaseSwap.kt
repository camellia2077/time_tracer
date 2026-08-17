package com.example.tracer

import java.io.File
import java.nio.file.Files
import java.nio.file.StandardCopyOption

/**
 * Owns the file-level SQLite replacement protocol used by rebuild migrations.
 * Keeping WAL/SHM handling here prevents migration orchestration from knowing
 * SQLite's sidecar layout.
 */
internal class RuntimeSqliteDatabaseSwap {
    fun replace(activePath: String, candidatePath: String, transactionRoot: File) {
        val active = File(activePath)
        val candidate = File(candidatePath)
        require(candidate.isFile) { "Candidate database was not created." }

        val backupRoot = File(transactionRoot, "backup")
        require(backupRoot.mkdirs()) { "Cannot create database backup directory." }
        moveSqliteFiles(active, backupRoot, replaceExisting = true)
        moveSqliteFiles(
            candidate,
            active.parentFile ?: error("Database directory is unavailable"),
            replaceExisting = true
        )
    }

    fun restore(activePath: String, transactionRoot: File) {
        val active = File(activePath)
        deleteSqliteFiles(active)
        val backupDatabase = File(transactionRoot, "backup/${active.name}")
        if (backupDatabase.exists()) {
            moveSqliteFiles(
                backupDatabase,
                active.parentFile ?: error("Database directory is unavailable"),
                replaceExisting = true
            )
        }
    }

    private fun moveSqliteFiles(sourceDatabase: File, targetDirectory: File, replaceExisting: Boolean) {
        targetDirectory.mkdirs()
        sqliteFiles(sourceDatabase).filter(File::exists).forEach { source ->
            val target = File(targetDirectory, source.name)
            if (replaceExisting) {
                Files.move(source.toPath(), target.toPath(), StandardCopyOption.REPLACE_EXISTING)
            } else {
                Files.move(source.toPath(), target.toPath())
            }
        }
    }

    private fun deleteSqliteFiles(database: File) {
        sqliteFiles(database).forEach { Files.deleteIfExists(it.toPath()) }
    }

    private fun sqliteFiles(database: File): List<File> = listOf(
        database,
        File(database.path + "-wal"),
        File(database.path + "-shm"),
        File(database.path + "-journal")
    )
}
