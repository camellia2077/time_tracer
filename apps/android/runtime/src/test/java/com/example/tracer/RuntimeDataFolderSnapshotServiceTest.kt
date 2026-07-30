package com.example.tracer

import java.io.File
import java.nio.file.Files
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class RuntimeDataFolderSnapshotServiceTest {
    @Test
    fun replace_validatesCandidateAndRemovesFilesOutsideSnapshot() {
        val sandbox = Files.createTempDirectory("snapshot-service").toFile()
        val filesDir = sandbox.resolve("files").apply { mkdirs() }
        val activeRoot = filesDir.apply { mkdirs() }
        activeRoot.resolve("config/user/behavior.toml").apply {
            parentFile?.mkdirs()
            writeText("old config")
        }
        activeRoot.resolve("config/user/old.toml").writeText("old")
        activeRoot.resolve("input/old/old.txt").apply {
            parentFile?.mkdirs()
            writeText("old")
        }
        val stagedRoot = sandbox.resolve("staged").apply { mkdirs() }
        stagedRoot.resolve("config/user/behavior.toml").apply {
            parentFile?.mkdirs()
            writeText("new config")
        }
        stagedRoot.resolve("config/user/new.toml").writeText("new")
        stagedRoot.resolve("input/2026/2026-07.txt").apply {
            parentFile?.mkdirs()
            writeText("new")
        }

        val activePaths = runtimePaths(activeRoot)
        var nativeIngestCalled = false
        val service = RuntimeDataFolderSnapshotService(
            ensureRuntimePaths = { activePaths },
            resetRuntimeCaches = {},
            nativeInit = { NativeCallResult(true, true, "{\"ok\":true}").rawResponse },
            nativeInitPipeline = { NativeCallResult(true, true, "{\"ok\":true}").rawResponse },
            nativeShutdown = { "{\"ok\":true}" },
            nativeValidateStructure = { "{\"ok\":true}" },
            nativeValidateLogic = { _, _ -> "{\"ok\":true}" },
            nativeIngest = { inputPath, _, _ ->
                nativeIngestCalled = true
                File(inputPath).parentFile?.resolve("db/time_data.sqlite3")?.apply {
                    parentFile?.mkdirs()
                    writeText("candidate db")
                }
                "{\"ok\":true}"
            },
            responseCodec = NativeResponseCodec()
        )

        val result = kotlinx.coroutines.runBlocking {
            service.replace(stagedRoot.absolutePath)
        }

        assertTrue(result.ok)
        assertTrue(nativeIngestCalled)
        assertFalse(activeRoot.resolve("config/user/old.toml").exists())
        assertFalse(activeRoot.resolve("input/old/old.txt").exists())
        assertTrue(activeRoot.resolve("config/user/new.toml").exists())
        assertTrue(activeRoot.resolve("input/2026/2026-07.txt").exists())
        assertFalse(activeRoot.resolve("config/program").exists())
        assertTrue(activeRoot.resolve(DATA_FOLDER_SNAPSHOT_MARKER).exists())
    }

    private fun runtimePaths(root: java.io.File): RuntimePaths = RuntimePaths(
        dbPath = root.resolve("db/time_data.sqlite3").absolutePath,
        outputRoot = root.resolve("output").absolutePath,
        configRootPath = root.resolve("config").absolutePath,
        configTomlPath = root.resolve("config/user/behavior.toml").absolutePath,
        inputRootPath = root.resolve("input").absolutePath,
        cacheRootPath = root.resolve("cache").absolutePath
    )
}
