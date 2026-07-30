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
            targetPath = File(rootDir, "config"),
            overwriteExistingFiles = false
        )
    }

    private fun copyAssetTreeIfPresent(
        assetPath: String,
        targetPath: File,
        overwriteExistingFiles: Boolean
    ) {
        val children = runCatching { assetManager.list(assetPath) }
            .getOrNull()
            ?: return
        if (children.isEmpty()) {
            return
        }
        copyAssetTree(
            assetPath = assetPath,
            targetPath = targetPath,
            overwriteExistingFiles = overwriteExistingFiles
        )
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

}
