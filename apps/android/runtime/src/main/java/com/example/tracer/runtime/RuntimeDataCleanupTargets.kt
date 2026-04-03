package com.example.tracer

import java.io.File

internal object RuntimeDataCleanupTargets {
    private const val DatabaseDirName = "db"
    private const val DatabaseFileName = "time_data.sqlite3"
    private val DatabaseWhitelistNames = listOf(
        DatabaseFileName,
        "$DatabaseFileName-wal",
        "$DatabaseFileName-shm",
        "$DatabaseFileName-journal"
    )
    private val TxtWhitelistRoots = listOf("input")

    fun clearDatabaseData(roots: List<File>): ClearDatabaseResult {
        val existingRoots = roots.filter { it.exists() }
        if (existingRoots.isEmpty()) {
            return ClearDatabaseResult(
                ok = true,
                message = "clear db -> no database data to remove"
            )
        }

        val databaseFiles = existingRoots
            .flatMap(::databaseFilesFor)
            .distinctBy { it.absolutePath }
        if (databaseFiles.isEmpty()) {
            return ClearDatabaseResult(
                ok = true,
                message = "clear db -> no database found"
            )
        }

        var removedCount = 0
        val failedPaths = mutableListOf<String>()
        for (databaseFile in databaseFiles) {
            if (databaseFile.delete()) {
                removedCount += 1
            } else {
                failedPaths += databaseFile.absolutePath
            }
        }
        existingRoots.forEach(::pruneEmptyDatabaseDir)

        return if (failedPaths.isNotEmpty()) {
            val preview = failedPaths.take(3).joinToString(" | ")
            val suffix = if (failedPaths.size > 3) {
                " | ...(${failedPaths.size} failed)"
            } else {
                ""
            }
            ClearDatabaseResult(
                ok = false,
                message = "clear db -> removed $removedCount/${databaseFiles.size} whitelisted file(s). failed: $preview$suffix"
            )
        } else {
            ClearDatabaseResult(
                ok = true,
                message = "clear db -> removed $removedCount database file(s)"
            )
        }
    }

    fun clearTxtData(roots: List<File>): ClearTxtResult {
        val existingRoots = roots.filter { it.exists() }
        if (existingRoots.isEmpty()) {
            return ClearTxtResult(
                ok = true,
                message = "clear txt -> no input directory to remove"
            )
        }

        val txtFiles = existingRoots
            .flatMap(::txtFilesFor)
            .distinctBy { it.absolutePath }
        if (txtFiles.isEmpty()) {
            return ClearTxtResult(
                ok = true,
                message = "clear txt -> no txt files in whitelisted input roots"
            )
        }

        var removedCount = 0
        val failedPaths = mutableListOf<String>()
        for (txtFile in txtFiles) {
            if (txtFile.delete()) {
                removedCount += 1
            } else {
                failedPaths += txtFile.absolutePath
            }
        }

        if (failedPaths.isNotEmpty()) {
            val preview = failedPaths.take(3).joinToString(" | ")
            val suffix = if (failedPaths.size > 3) {
                " | ...(${failedPaths.size} failed)"
            } else {
                ""
            }
            return ClearTxtResult(
                ok = false,
                message = "clear txt -> removed $removedCount/${txtFiles.size} whitelisted file(s). failed: $preview$suffix"
            )
        }

        return ClearTxtResult(
            ok = true,
            message = "clear txt -> removed $removedCount txt file(s) from whitelisted input roots"
        )
    }

    private fun databaseFilesFor(root: File): List<File> {
        val dbDir = File(root, DatabaseDirName)
        return DatabaseWhitelistNames
            .map { fileName -> File(dbDir, fileName) }
            .filter { it.exists() && it.isFile }
    }

    private fun pruneEmptyDatabaseDir(root: File) {
        val dbDir = File(root, DatabaseDirName)
        if (!dbDir.exists() || !dbDir.isDirectory) {
            return
        }
        if (dbDir.listFiles()?.isEmpty() == true) {
            dbDir.delete()
        }
    }

    private fun txtFilesFor(root: File): List<File> {
        return TxtWhitelistRoots.flatMap { relativeRoot ->
            val inputRoot = File(root, relativeRoot)
            if (!inputRoot.exists() || !inputRoot.isDirectory) {
                emptyList()
            } else {
                inputRoot.walkTopDown()
                    .filter { it.isFile && it.extension.equals("txt", ignoreCase = true) }
                    .toList()
            }
        }
    }
}

