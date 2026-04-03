package com.example.tracer

import android.content.Context
import java.io.File

internal class RuntimeEnvironment(private val context: Context) {
    private companion object {
        const val RUNTIME_ROOT_DIR_NAME = "tracer_core"
        const val LEGACY_RUNTIME_ROOT_DIR_NAME = "time_tracer"
        const val RUNTIME_ASSET_ROOT = "tracer_core"
    }

    private val rootResolver = RuntimeRootDirectoryResolver(
        filesDir = context.filesDir,
        runtimeRootDirName = RUNTIME_ROOT_DIR_NAME,
        legacyRuntimeRootDirName = LEGACY_RUNTIME_ROOT_DIR_NAME
    )
    private val assetBootstrapper = RuntimeAssetBootstrapper(
        assetManager = context.assets,
        runtimeAssetRoot = RUNTIME_ASSET_ROOT
    )

    @Volatile
    private var lastConfigBundleStatus: RuntimeConfigBundleStatus = unknownConfigBundleStatus()

    fun lastConfigBundleStatus(): RuntimeConfigBundleStatus = lastConfigBundleStatus

    fun prepareRuntimePaths(): RuntimePaths {
        val rootDir = rootResolver.resolveRuntimeRootDir()
        assetBootstrapper.bootstrap(rootDir)

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
        return rootResolver.clearRuntimeData()
    }

    fun clearDatabaseData(): ClearDatabaseResult {
        return RuntimeDataCleanupTargets.clearDatabaseData(rootResolver.candidateRuntimeRoots())
    }

    fun clearTxtData(): ClearTxtResult {
        rootResolver.resolveRuntimeRootDir()
        return RuntimeDataCleanupTargets.clearTxtData(rootResolver.candidateRuntimeRoots())
    }
}

