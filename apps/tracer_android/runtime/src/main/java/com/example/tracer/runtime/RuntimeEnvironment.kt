package com.example.tracer

import android.content.Context
import android.content.res.AssetManager
import java.io.File
import java.io.FileOutputStream
import java.io.IOException

internal class RuntimeEnvironment(private val context: Context) {
    private companion object {
        const val RUNTIME_ROOT_DIR_NAME = "tracer_core"
        const val LEGACY_RUNTIME_ROOT_DIR_NAME = "time_tracer"
        const val RUNTIME_ASSET_ROOT = "tracer_core"
    }

    @Volatile
    private var lastConfigBundleStatus: RuntimeConfigBundleStatus = unknownConfigBundleStatus()

    fun lastConfigBundleStatus(): RuntimeConfigBundleStatus = lastConfigBundleStatus

    fun prepareRuntimePaths(): RuntimePaths {
        val rootDir = resolveRuntimeRootDir()
        // Keep runtime-generated data by default.
        copyAssetTree(
            assetManager = context.assets,
            assetPath = RUNTIME_ASSET_ROOT,
            targetPath = rootDir,
            overwriteExistingFiles = false
        )
        // Keep imported/edited runtime config stable across initialization.
        // Config assets are used for first-time bootstrap and missing-file补齐 only.
        copyAssetTree(
            assetManager = context.assets,
            assetPath = "$RUNTIME_ASSET_ROOT/config",
            targetPath = File(rootDir, "config"),
            overwriteExistingFiles = false
        )
        removeLegacyReportConfigDirs(rootDir)
        refreshLegacyBundleTomlIfNeeded(rootDir)

        val configRootDir = File(rootDir, "config")
        val configBundleStatus = validateRuntimeConfigBundle(configRootDir)
        lastConfigBundleStatus = configBundleStatus
        if (!configBundleStatus.ok) {
            throw IllegalStateException(configBundleStatus.message)
        }

        val dbFile = File(rootDir, "db/time_data.sqlite3")
        dbFile.parentFile?.mkdirs()

        val outputRoot = File(rootDir, "output")
        if (!outputRoot.exists()) {
            outputRoot.mkdirs()
        }

        val configToml = File(configRootDir, "converter/interval_processor_config.toml")
        if (!configToml.exists()) {
            throw IllegalStateException("Missing config TOML: ${configToml.absolutePath}")
        }

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
            configRootPath = configRootDir.absolutePath,
            configTomlPath = configToml.absolutePath,
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
        val runtimeRootDir = File(context.filesDir, RUNTIME_ROOT_DIR_NAME)
        val legacyRootDir = File(context.filesDir, LEGACY_RUNTIME_ROOT_DIR_NAME)
        val existingRoots = listOf(runtimeRootDir, legacyRootDir).filter { it.exists() }
        if (existingRoots.isEmpty()) {
            return "clear -> no runtime data to remove"
        }

        val failedRoots = mutableListOf<String>()
        for (root in existingRoots) {
            if (!root.deleteRecursively()) {
                failedRoots += root.absolutePath
            }
        }
        return if (failedRoots.isEmpty()) {
            "clear -> removed ${existingRoots.joinToString { it.absolutePath }}"
        } else {
            "clear -> failed to remove ${failedRoots.joinToString()}"
        }
    }

    fun clearTxtData(): ClearTxtResult {
        val inputDir = File(resolveRuntimeRootDir(), "input")
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

    private fun resolveRuntimeRootDir(): File {
        val runtimeRootDir = File(context.filesDir, RUNTIME_ROOT_DIR_NAME)
        migrateLegacyRuntimeRootIfNeeded(runtimeRootDir)
        return runtimeRootDir
    }

    private fun migrateLegacyRuntimeRootIfNeeded(runtimeRootDir: File) {
        if (runtimeRootDir.exists()) {
            return
        }

        val legacyRootDir = File(context.filesDir, LEGACY_RUNTIME_ROOT_DIR_NAME)
        if (!legacyRootDir.exists()) {
            return
        }

        runtimeRootDir.parentFile?.mkdirs()
        if (legacyRootDir.renameTo(runtimeRootDir)) {
            return
        }

        val copied = legacyRootDir.copyRecursively(runtimeRootDir, overwrite = false)
        if (copied) {
            legacyRootDir.deleteRecursively()
        }
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

    // Keep user-edited config files stable, but auto-repair legacy bundle schema
    // when runtime still holds an old bundle.toml without visualization paths.
    private fun refreshLegacyBundleTomlIfNeeded(rootDir: File) {
        val bundleFile = File(rootDir, "config/meta/bundle.toml")
        if (!bundleFile.exists()) {
            return
        }

        val requiresRefresh = try {
            val content = bundleFile.readText()
            val hasVisualizationTable =
                content.contains(Regex("""(?m)^\s*\[paths\.visualization\]\s*$"""))
            val hasHeatmapPath =
                content.contains(Regex("""(?m)^\s*heatmap\s*=\s*".+"\s*$"""))
            !(hasVisualizationTable && hasHeatmapPath)
        } catch (_: IOException) {
            true
        }

        if (!requiresRefresh) {
            return
        }

        copyAssetFile(
            assetManager = context.assets,
            assetPath = "$RUNTIME_ASSET_ROOT/config/meta/bundle.toml",
            targetFile = bundleFile,
            overwriteExistingFile = true
        )
    }
}
