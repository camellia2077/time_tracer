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
