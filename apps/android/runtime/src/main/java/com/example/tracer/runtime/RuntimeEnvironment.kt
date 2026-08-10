package com.example.tracer

import android.content.Context
import java.io.File

internal const val DATA_FOLDER_SNAPSHOT_MARKER = ".data_folder_snapshot"

internal class RuntimeEnvironment(private val context: Context) {
    private companion object {
        const val RUNTIME_ASSET_ROOT = "config"
    }

    private val assetBootstrapper = RuntimeAssetBootstrapper(
        assetManager = context.assets,
        runtimeAssetRoot = RUNTIME_ASSET_ROOT
    )

    @Volatile
    private var lastConfigBundleStatus: RuntimeConfigBundleStatus = unknownConfigBundleStatus()

    fun lastConfigBundleStatus(): RuntimeConfigBundleStatus = lastConfigBundleStatus

    fun prepareRuntimePaths(): RuntimePaths {
        val rootDir = context.filesDir
        // Program config is APK-owned and must be refreshed during every
        // process start. The bootstrapper only preserves config/user.
        assetBootstrapper.bootstrap(rootDir)

        val configRootDir = File(rootDir, "config")
        val programRootDir = File(configRootDir, "program")
        val userRootDir = File(configRootDir, "user")
        userRootDir.mkdirs()
        // The hierarchy is mutable user configuration. Keep the canonical
        // directory present even when an older data snapshot or a minimal
        // installation has not supplied any hierarchy documents yet.
        File(userRootDir, "activity_hierarchy").mkdirs()
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

        val configToml = File(userRootDir, "behavior.toml")
        if (!configToml.exists()) {
            throw IllegalStateException(
                "Missing mutable user behavior TOML: ${configToml.absolutePath}. " +
                    "Populate the private config/user directory before runtime initialization."
            )
        }

        val inputRoot = File(rootDir, "input")
        if (!inputRoot.exists()) {
            inputRoot.mkdirs()
        }
        val cacheRoot = File(outputRoot, "cache")
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

    fun clearAllData(): String {
        val roots = listOf(context.filesDir)
        val editableMessage = RuntimeDataCleanupTargets.clearEditableData(roots)
        val databaseResult = RuntimeDataCleanupTargets.clearDatabaseData(roots)
        if (!databaseResult.ok) {
            throw IllegalStateException(databaseResult.message)
        }
        return "$editableMessage; ${databaseResult.message}"
    }

    fun clearDatabaseData(): ClearDatabaseResult {
        return RuntimeDataCleanupTargets.clearDatabaseData(listOf(context.filesDir))
    }

    fun clearTxtData(): ClearTxtResult {
        return RuntimeDataCleanupTargets.clearTxtData(listOf(context.filesDir))
    }

}
