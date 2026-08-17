@file:Suppress("LongMethod")

package com.example.tracer

import java.io.File
import java.nio.file.Files
import java.nio.file.StandardCopyOption
import java.util.UUID
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import kotlinx.coroutines.withContext

/**
 * Validates and installs a complete data-folder snapshot.
 *
 * The candidate TOML/TXT tree and database are built first. The active runtime
 * root is replaced only after core has accepted the candidate and ingested all
 * candidate TXT files.
 */
internal class RuntimeDataFolderSnapshotService(
    private val ensureRuntimePaths: () -> RuntimePaths,
    private val resetRuntimeCaches: () -> Unit,
    private val nativeInit: (RuntimePaths) -> String,
    private val nativeInitPipeline: (RuntimePaths) -> String,
    private val nativeShutdown: () -> String,
    private val nativeValidateStructure: (String) -> String,
    private val nativeValidateLogic: (String, Int) -> String,
    private val nativeIngest: (String, Int, Boolean) -> String,
    private val responseCodec: NativeResponseCodec
) {
    private val mutex = Mutex()

    suspend fun replace(stagedRootPath: String): DataFolderSnapshotResult =
        withContext(Dispatchers.IO) { mutex.withLock { replaceLocked(File(stagedRootPath)) } }

    private fun replaceLocked(stagedRoot: File): DataFolderSnapshotResult {
        val stagedConfig = File(stagedRoot, "config")
        val stagedInput = File(stagedRoot, "input")
        val stagedConfigToml = File(stagedConfig, "user/behavior.toml")
        val txtFiles = stagedInput.walkTopDown()
            .filter { it.isFile && it.extension.equals("txt", ignoreCase = true) }
            .toList()
        val tomlFiles = stagedConfig.walkTopDown()
            .filter { it.isFile && it.extension.equals("toml", ignoreCase = true) }
            .toList()
        require(stagedConfig.isDirectory) { "Staged data folder is missing config/." }
        require(stagedInput.isDirectory) { "Staged data folder is missing input/." }
        require(stagedConfigToml.isFile) {
            "Staged data folder is missing config/user/behavior.toml."
        }

        val activePaths = ensureRuntimePaths()
        val activeRoot = File(activePaths.configRootPath).canonicalFile.parentFile
            ?: error("Active runtime root is unavailable.")
        val candidatePaths = activePaths.copy(
            dbPath = File(stagedRoot, "db/time_data.sqlite3").absolutePath,
            outputRoot = File(stagedRoot, "output").absolutePath,
            configRootPath = stagedConfig.absolutePath,
            configTomlPath = stagedConfigToml.absolutePath,
            inputRootPath = stagedInput.absolutePath,
            cacheRootPath = File(stagedRoot, "output/cache").absolutePath
        )
        File(candidatePaths.outputRoot).mkdirs()
        File(candidatePaths.cacheRootPath).mkdirs()
        File(candidatePaths.dbPath).parentFile?.mkdirs()

        try {
            checkNativeOk(nativeShutdown(), "native shutdown failed")
            checkNativeOk(
                nativeInitPipeline(candidatePaths),
                "candidate pipeline runtime init failed"
            )
            for (txtFile in txtFiles) {
                checkNativeOk(
                    nativeValidateStructure(txtFile.absolutePath),
                    "TXT structure validation failed: ${txtFile.name}"
                )
                checkNativeOk(
                    nativeValidateLogic(txtFile.absolutePath, NativeBridge.DATE_CHECK_NONE),
                    "TXT logic validation failed: ${txtFile.name}"
                )
            }
            checkNativeOk(
                nativeIngest(candidatePaths.inputRootPath, NativeBridge.DATE_CHECK_NONE, false),
                "candidate TXT ingest failed"
            )
            checkNativeOk(nativeShutdown(), "candidate native shutdown failed")

            val backupRoot = File(
                activeRoot.parentFile,
                "${activeRoot.name}.data-folder-backup-${UUID.randomUUID()}"
            )
            moveRoot(activeRoot, backupRoot)
            try {
                moveRoot(stagedRoot, activeRoot)
                File(activeRoot, DATA_FOLDER_SNAPSHOT_MARKER).writeText(
                    "source=data_folder_import\n"
                )
                resetRuntimeCaches()
                checkNativeOk(nativeInit(ensureRuntimePaths()), "active native init failed")
                backupRoot.deleteRecursively()
            } catch (error: Exception) {
                runCatching { nativeShutdown() }
                activeRoot.deleteRecursively()
                moveRoot(backupRoot, activeRoot)
                resetRuntimeCaches()
                runCatching { nativeInit(ensureRuntimePaths()) }
                throw error
            }
            return DataFolderSnapshotResult(
                ok = true,
                message = "Data folder replaced from complete snapshot.",
                txtFileCount = txtFiles.size,
                tomlFileCount = tomlFiles.size
            )
        } catch (error: Exception) {
            runCatching { nativeShutdown() }
            resetRuntimeCaches()
            runCatching { nativeInit(ensureRuntimePaths()) }
            return DataFolderSnapshotResult(
                ok = false,
                message = error.message ?: "Data folder snapshot replacement failed."
            )
        }
    }

    private fun checkNativeOk(rawResponse: String, fallback: String) {
        val payload = responseCodec.parse(rawResponse)
        check(payload.ok) { payload.errorMessage.ifBlank { fallback } }
    }

    private fun moveRoot(source: File, target: File) {
        target.parentFile?.mkdirs()
        Files.move(
            source.toPath(),
            target.toPath(),
            StandardCopyOption.REPLACE_EXISTING
        )
    }

}
