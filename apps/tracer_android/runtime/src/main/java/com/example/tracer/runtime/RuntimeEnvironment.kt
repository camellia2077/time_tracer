package com.example.tracer

import android.content.Context
import android.content.res.AssetManager
import java.io.File
import java.io.FileOutputStream
import java.io.IOException

internal class RuntimeEnvironment(private val context: Context) {
    fun prepareRuntimePaths(): RuntimePaths {
        val rootDir = File(context.filesDir, "time_tracer")
        copyAssetTree(context.assets, "time_tracer", rootDir)
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

    fun clearLiveTxtData(): ClearTxtResult {
        val rootDir = File(context.filesDir, "time_tracer")
        val liveRawDir = File(rootDir, "input/live_raw")
        val liveAutoSyncDir = File(rootDir, "input/live_auto_sync")

        val (rawOk, rawMessage) = resetDirectory(liveRawDir, "live_raw")
        val (autoSyncOk, autoSyncMessage) = resetDirectory(liveAutoSyncDir, "live_auto_sync")

        return ClearTxtResult(
            ok = rawOk && autoSyncOk,
            message = "$rawMessage\n$autoSyncMessage"
        )
    }

    private fun copyAssetTree(assetManager: AssetManager, assetPath: String, targetPath: File) {
        val children = assetManager.list(assetPath) ?: emptyArray()
        if (children.isEmpty()) {
            copyAssetFile(assetManager, assetPath, targetPath)
            return
        }

        if (!targetPath.exists() && !targetPath.mkdirs()) {
            throw IOException("Failed to create directory: ${targetPath.absolutePath}")
        }

        for (child in children) {
            val childAssetPath = "$assetPath/$child"
            val childTargetPath = File(targetPath, child)
            copyAssetTree(assetManager, childAssetPath, childTargetPath)
        }
    }

    private fun copyAssetFile(assetManager: AssetManager, assetPath: String, targetFile: File) {
        // Copy defaults only once; keep user-edited runtime files on next app starts.
        if (targetFile.exists()) {
            return
        }
        targetFile.parentFile?.mkdirs()
        assetManager.open(assetPath).use { input ->
            FileOutputStream(targetFile, false).use { output ->
                input.copyTo(output)
            }
        }
    }

    private fun resetDirectory(dir: File, label: String): Pair<Boolean, String> {
        val deleted = if (dir.exists()) dir.deleteRecursively() else true
        if (!deleted) {
            return false to "clear $label -> failed to delete ${dir.absolutePath}"
        }

        if (!dir.exists() && !dir.mkdirs()) {
            return false to "clear $label -> failed to recreate ${dir.absolutePath}"
        }

        return true to "clear $label -> ready ${dir.absolutePath}"
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
