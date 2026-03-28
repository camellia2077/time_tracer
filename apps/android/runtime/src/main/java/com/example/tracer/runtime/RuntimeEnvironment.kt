package com.example.tracer

import android.content.Context
import android.content.res.AssetManager
import java.io.File
import java.io.FileOutputStream
import java.io.IOException

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

        val inputRoot = File(rootDir, "input")
        if (!inputRoot.exists()) {
            inputRoot.mkdirs()
        }
        val cacheRoot = File(rootDir, "cache")
        if (!cacheRoot.exists()) {
            cacheRoot.mkdirs()
        }

        return RuntimePaths(
            dbPath = dbFile.absolutePath,
            outputRoot = outputRoot.absolutePath,
            configRootPath = configRootDir.absolutePath,
            configTomlPath = configToml.absolutePath,
            inputRootPath = inputRoot.absolutePath,
            cacheRootPath = cacheRoot.absolutePath
        )
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

    fun clearDatabaseData(): ClearDatabaseResult {
        return RuntimeDataCleanupTargets.clearDatabaseData(candidateRuntimeRoots())
    }

    fun clearTxtData(): ClearTxtResult {
        val runtimeRootDir = resolveRuntimeRootDir()
        val legacyRootDir = File(context.filesDir, LEGACY_RUNTIME_ROOT_DIR_NAME)
        return RuntimeDataCleanupTargets.clearTxtData(
            listOf(runtimeRootDir, legacyRootDir).distinctBy { it.absolutePath }
        )
    }

    private fun candidateRuntimeRoots(): List<File> {
        val runtimeRootDir = File(context.filesDir, RUNTIME_ROOT_DIR_NAME)
        val legacyRootDir = File(context.filesDir, LEGACY_RUNTIME_ROOT_DIR_NAME)
        return listOf(runtimeRootDir, legacyRootDir).distinctBy { it.absolutePath }
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
            assetManager = context.assets,
            assetPath = "$RUNTIME_ASSET_ROOT/config/meta/bundle.toml",
            targetFile = bundleFile,
            overwriteExistingFile = true
        )
    }
}
