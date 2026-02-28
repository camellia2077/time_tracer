package com.example.tracer

import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import java.io.File
import java.nio.file.Files

class RuntimeIngestServiceTest {
    @Test
    fun ingestFull_whenExecutorThrows_returnsFailure() = runBlocking {
        val root = Files.createTempDirectory("runtime-ingest-fail").toFile()
        try {
            val paths = createPaths(root)
            val service = RuntimeIngestService(
                ensureRuntimePaths = { paths },
                executeAfterInit = { _, _ -> throw IllegalStateException("executor down") },
                nativeIngest = { _, _, _ -> """{"ok":true}""" },
                nativeIngestSingleTxtReplaceMonth = { _, _, _ -> """{"ok":true}""" }
            )

            val result = service.ingestFull()

            assertFalse(result.initialized)
            assertFalse(result.operationOk)
            assertTrue(result.rawResponse.contains("nativeIngest failed"))
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun ingestSingleTxtReplaceMonth_missingHeader_returnsFailureBeforeExecutor() = runBlocking {
        val root = Files.createTempDirectory("runtime-ingest-single-missing").toFile()
        try {
            val paths = createPaths(root)
            val source = File(root, "input.txt")
            source.writeText("y2026\n2026-03-01 | no month header\n")
            var executorCalled = false
            val service = RuntimeIngestService(
                ensureRuntimePaths = { paths },
                executeAfterInit = { _, _ ->
                    executorCalled = true
                    NativeCallResult(initialized = true, operationOk = true, rawResponse = """{"ok":true}""")
                },
                nativeIngest = { _, _, _ -> """{"ok":true}""" },
                nativeIngestSingleTxtReplaceMonth = { _, _, _ -> """{"ok":true}""" }
            )

            val result = service.ingestSingleTxtReplaceMonth(source.absolutePath)

            assertFalse(result.initialized)
            assertFalse(result.operationOk)
            assertTrue(result.rawResponse.contains("requires headers yYYYY + mMM"))
            assertFalse(executorCalled)
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun ingestSingleTxtReplaceMonth_success_writesManagedMonthFileAndNormalizesText() = runBlocking {
        val root = Files.createTempDirectory("runtime-ingest-single-ok").toFile()
        try {
            val paths = createPaths(root)
            val source = File(root, "source.txt")
            source.writeText("\uFEFFy2026\r\nm03\r\n2026-03-01 08:00|study|remark\r\n")
            var capturedManagedInputPath = ""
            val service = RuntimeIngestService(
                ensureRuntimePaths = { paths },
                executeAfterInit = { _, action ->
                    NativeCallResult(
                        initialized = true,
                        operationOk = true,
                        rawResponse = action(paths)
                    )
                },
                nativeIngest = { _, _, _ -> """{"ok":true}""" },
                nativeIngestSingleTxtReplaceMonth = { inputPath, _, _ ->
                    capturedManagedInputPath = inputPath
                    """{"ok":true}"""
                }
            )

            val result = service.ingestSingleTxtReplaceMonth(source.absolutePath)

            assertTrue(result.initialized)
            assertTrue(result.operationOk)
            val managedFile = File(capturedManagedInputPath)
            assertTrue(managedFile.exists())
            assertTrue(managedFile.name == "2026-03.txt")
            val savedContent = managedFile.readText()
            assertFalse(savedContent.startsWith("\uFEFF"))
            assertFalse(savedContent.contains("\r"))
            assertTrue(savedContent.contains("y2026\nm03\n"))
        } finally {
            root.deleteRecursively()
        }
    }

    private fun createPaths(root: File): RuntimePaths {
        val full = File(root, "input/full").apply { mkdirs() }
        val liveRaw = File(root, "input/live_raw").apply { mkdirs() }
        val liveAutoSync = File(root, "input/live_auto_sync").apply { mkdirs() }
        val output = File(root, "output").apply { mkdirs() }
        val configRoot = File(root, "config").apply { mkdirs() }
        val db = File(root, "db/time_data.sqlite3").apply {
            parentFile?.mkdirs()
            writeText("")
        }
        val configToml = File(configRoot, "converter/interval_processor_config.toml").apply {
            parentFile?.mkdirs()
            writeText("dummy=true")
        }
        return RuntimePaths(
            dbPath = db.absolutePath,
            outputRoot = output.absolutePath,
            configRootPath = configRoot.absolutePath,
            configTomlPath = configToml.absolutePath,
            fullInputPath = full.absolutePath,
            liveRawInputPath = liveRaw.absolutePath,
            liveAutoSyncInputPath = liveAutoSync.absolutePath
        )
    }
}
