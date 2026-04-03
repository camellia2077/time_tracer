package com.example.tracer

import android.content.res.AssetManager
import java.io.File
import java.io.FileOutputStream
import java.io.IOException

internal class RuntimeAssetBootstrapper(
    private val assetManager: AssetManager,
    private val runtimeAssetRoot: String
) {
    fun bootstrap(rootDir: File) {
        copyAssetTree(
            assetPath = runtimeAssetRoot,
            targetPath = rootDir,
            overwriteExistingFiles = false
        )
        copyAssetTree(
            assetPath = "$runtimeAssetRoot/config",
            targetPath = File(rootDir, "config"),
            overwriteExistingFiles = false
        )
        removeLegacyReportConfigDirs(rootDir)
        refreshLegacyBundleTomlIfNeeded(rootDir)
    }

    private fun copyAssetTree(
        assetPath: String,
        targetPath: File,
        overwriteExistingFiles: Boolean
    ) {
        val children = assetManager.list(assetPath) ?: emptyArray()
        if (children.isEmpty()) {
            copyAssetFile(
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
                assetPath = childAssetPath,
                targetPath = childTargetPath,
                overwriteExistingFiles = overwriteExistingFiles
            )
        }
    }

    private fun copyAssetFile(
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

    private fun refreshLegacyBundleTomlIfNeeded(rootDir: File) {
        val bundleFile = File(rootDir, "config/meta/bundle.toml")
        if (!bundleFile.exists()) {
            return
        }

        val requiresRefresh = try {
            val content = CanonicalTextCodec.readFile(bundleFile)
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
            assetPath = "$runtimeAssetRoot/config/meta/bundle.toml",
            targetFile = bundleFile,
            overwriteExistingFile = true
        )
    }
}

