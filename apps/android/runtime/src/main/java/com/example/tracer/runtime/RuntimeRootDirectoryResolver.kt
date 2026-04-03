package com.example.tracer

import java.io.File

internal class RuntimeRootDirectoryResolver(
    private val filesDir: File,
    private val runtimeRootDirName: String,
    private val legacyRuntimeRootDirName: String
) {
    fun candidateRuntimeRoots(): List<File> {
        val runtimeRootDir = File(filesDir, runtimeRootDirName)
        val legacyRootDir = File(filesDir, legacyRuntimeRootDirName)
        return listOf(runtimeRootDir, legacyRootDir).distinctBy { it.absolutePath }
    }

    fun resolveRuntimeRootDir(): File {
        val runtimeRootDir = File(filesDir, runtimeRootDirName)
        migrateLegacyRuntimeRootIfNeeded(runtimeRootDir)
        return runtimeRootDir
    }

    fun clearRuntimeData(): String {
        val existingRoots = candidateRuntimeRoots().filter { it.exists() }
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

    private fun migrateLegacyRuntimeRootIfNeeded(runtimeRootDir: File) {
        if (runtimeRootDir.exists()) {
            return
        }

        val legacyRootDir = File(filesDir, legacyRuntimeRootDirName)
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
}

