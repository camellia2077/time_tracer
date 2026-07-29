package com.example.tracer

import java.io.File
import java.nio.file.Files
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class RuntimeDataFolderSnapshotServiceTest {
    @Test
    fun replace_validatesCandidateAndRemovesFilesOutsideSnapshot() {
        val filesDir = Files.createTempDirectory("snapshot-service").toFile()
        val activeRoot = filesDir.resolve("tracer_core").apply { mkdirs() }
        activeRoot.resolve("config/activity_hierarchy/_system.toml").apply {
            parentFile?.mkdirs()
            writeText("old config")
        }
        activeRoot.resolve("config/old.toml").writeText("old")
        activeRoot.resolve("input/old/old.txt").apply {
            parentFile?.mkdirs()
            writeText("old")
        }
        val stagedRoot = filesDir.resolve("staged").apply { mkdirs() }
        stagedRoot.resolve("config/activity_hierarchy/_system.toml").apply {
            parentFile?.mkdirs()
            writeText("new config")
        }
        stagedRoot.resolve("config/new.toml").writeText("new")
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
        assertFalse(activeRoot.resolve("config/old.toml").exists())
        assertFalse(activeRoot.resolve("input/old/old.txt").exists())
        assertTrue(activeRoot.resolve("config/new.toml").exists())
        assertTrue(activeRoot.resolve("input/2026/2026-07.txt").exists())
        assertTrue(activeRoot.resolve(DATA_FOLDER_SNAPSHOT_MARKER).exists())
    }

    private fun runtimePaths(root: java.io.File): RuntimePaths = RuntimePaths(
        dbPath = root.resolve("db/time_data.sqlite3").absolutePath,
        outputRoot = root.resolve("output").absolutePath,
        configRootPath = root.resolve("config").absolutePath,
        configTomlPath = root.resolve("config/activity_hierarchy/_system.toml").absolutePath,
        inputRootPath = root.resolve("input").absolutePath,
        cacheRootPath = root.resolve("cache").absolutePath
    )
}
