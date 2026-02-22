package com.example.tracer

import android.content.Context
import android.content.res.AssetManager
import java.io.File
import java.io.FileOutputStream
import java.io.IOException

internal class RuntimeEnvironment(private val context: Context) {
    fun prepareRuntimePaths(): RuntimePaths {
        val rootDir = File(context.filesDir, "time_tracer")
        // Keep runtime-generated data by default.
        copyAssetTree(
            assetManager = context.assets,
            assetPath = "time_tracer",
            targetPath = rootDir,
            overwriteExistingFiles = false
        )
        // Keep imported/edited runtime config stable across initialization.
        // Config assets are used for first-time bootstrap and missing-file补齐 only.
        copyAssetTree(
            assetManager = context.assets,
            assetPath = "time_tracer/config",
            targetPath = File(rootDir, "config"),
            overwriteExistingFiles = false
        )
        removeLegacyReportConfigDirs(rootDir)

        val dbFile = File(rootDir, "db/time_data.sqlite3")
        dbFile.parentFile?.mkdirs()

        val outputRoot = File(rootDir, "output")
        if (!outputRoot.exists()) {
            outputRoot.mkdirs()
        }

        val configToml = File(rootDir, "config/converter/interval_processor_config.toml")
        if (!configToml.exists()) {
            throw IllegalStateException("Missing config TOML: ${configToml.absolutePath}")
        }

        val smokeInput = File(rootDir, "input/smoke")
        val fullInput = File(rootDir, "input/full")
        val liveRawInput = File(rootDir, "input/live_raw")
        if (!liveRawInput.exists()) {
            liveRawInput.mkdirs()
        }
        val liveAutoSyncInput = File(rootDir, "input/live_auto_sync")
        if (!liveAutoSyncInput.exists()) {
            liveAutoSyncInput.mkdirs()
        }

        cleanupOldErrorLogs(File(rootDir, "output/logs"))

        return RuntimePaths(
            dbPath = dbFile.absolutePath,
            outputRoot = outputRoot.absolutePath,
            configRootPath = File(rootDir, "config").absolutePath,
            configTomlPath = configToml.absolutePath,
            smokeInputPath = smokeInput.absolutePath,
            fullInputPath = fullInput.absolutePath,
            liveRawInputPath = liveRawInput.absolutePath,
            liveAutoSyncInputPath = liveAutoSyncInput.absolutePath
        )
    }

    // Keeps the newest [maxCount] run-scoped error log files.
    // Never deletes errors-latest.log.
    internal fun cleanupOldErrorLogs(logsDir: File, maxCount: Int = 50) {
        if (!logsDir.isDirectory) return
        val logFiles = logsDir.listFiles { f ->
            f.isFile && f.name.startsWith("errors-") &&
                f.name.endsWith(".log") && f.name != "errors-latest.log"
        } ?: return
        if (logFiles.size <= maxCount) return
        logFiles.sortedByDescending { it.lastModified() }
            .drop(maxCount)
            .forEach { it.delete() }
    }

    fun clearRuntimeData(): String {
        val rootDir = File(context.filesDir, "time_tracer")
        if (!rootDir.exists()) {
            return "clear -> no runtime data to remove"
        }

        return if (rootDir.deleteRecursively()) {
            "clear -> removed ${rootDir.absolutePath}"
        } else {
            "clear -> failed to remove ${rootDir.absolutePath}"
        }
    }

    fun clearTxtData(): ClearTxtResult {
        val inputDir = File(context.filesDir, "time_tracer/input")
        if (!inputDir.exists()) {
            return ClearTxtResult(
                ok = true,
                message = "clear txt -> no input directory: ${inputDir.absolutePath}"
            )
        }

        val txtFiles = inputDir
            .walkTopDown()
            .filter { it.isFile && it.extension.equals("txt", ignoreCase = true) }
            .toList()

        if (txtFiles.isEmpty()) {
            return ClearTxtResult(
                ok = true,
                message = "clear txt -> no txt files under ${inputDir.absolutePath}"
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
                message = "clear txt -> removed $removedCount/${txtFiles.size} file(s). failed: $preview$suffix"
            )
        }

        return ClearTxtResult(
            ok = true,
            message = "clear txt -> removed $removedCount txt file(s) under ${inputDir.absolutePath}"
        )
    }

    private fun copyAssetTree(
        assetManager: AssetManager,
        assetPath: String,
        targetPath: File,
        overwriteExistingFiles: Boolean
    ) {
        val children = assetManager.list(assetPath) ?: emptyArray()
        if (children.isEmpty()) {
            copyAssetFile(
                assetManager = assetManager,
                assetPath = assetPath,
                targetFile = targetPath,
                overwriteExistingFile = overwriteExistingFiles
            )
            return
        }

        if (!targetPath.exists() && !targetPath.mkdirs()) {
            throw IOException("Failed to create directory: ${targetPath.absolutePath}")
        }

        for (child in children) {
            val childAssetPath = "$assetPath/$child"
            val childTargetPath = File(targetPath, child)
            copyAssetTree(
                assetManager = assetManager,
                assetPath = childAssetPath,
                targetPath = childTargetPath,
                overwriteExistingFiles = overwriteExistingFiles
            )
        }
    }

    private fun copyAssetFile(
        assetManager: AssetManager,
        assetPath: String,
        targetFile: File,
        overwriteExistingFile: Boolean
    ) {
        if (targetFile.exists() && !overwriteExistingFile) {
            return
        }
        targetFile.parentFile?.mkdirs()
        assetManager.open(assetPath).use { input ->
            FileOutputStream(targetFile, false).use { output ->
                input.copyTo(output)
            }
        }
    }

    private fun removeLegacyReportConfigDirs(rootDir: File) {
        val legacyDirs = listOf(
            File(rootDir, "config/reports/latex"),
            File(rootDir, "config/reports/typst")
        )

        for (dir in legacyDirs) {
            if (dir.exists()) {
                dir.deleteRecursively()
            }
        }
    }
}
