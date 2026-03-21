package com.example.tracer

import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import java.io.File
import java.nio.file.Files

class RuntimeDiagnosticsServiceTest {
    @Test
    fun listRecentDiagnostics_nonPositiveLimit_returnsFailure() = runBlocking {
        val recorder = RuntimeDiagnosticsRecorder(runtimePathsProvider = { null })
        val service = RuntimeDiagnosticsService(
            ensureRuntimePaths = { createPaths(Files.createTempDirectory("diag-list").toFile()) },
            bundleStatusProvider = { unknownConfigBundleStatus() },
            diagnosticsRecorder = recorder,
            nextOperationId = { stage -> "op-$stage" },
            errorMapper = RuntimeErrorMapper()
        )

        val result = service.listRecentDiagnostics(0)

        assertFalse(result.ok)
        assertEquals("diagnostics limit must be greater than 0.", result.message)
    }

    @Test
    fun buildDiagnosticsPayload_nonPositiveEntries_returnsFailure() = runBlocking {
        val recorder = RuntimeDiagnosticsRecorder(runtimePathsProvider = { null })
        val service = RuntimeDiagnosticsService(
            ensureRuntimePaths = { createPaths(Files.createTempDirectory("diag-limit").toFile()) },
            bundleStatusProvider = { unknownConfigBundleStatus() },
            diagnosticsRecorder = recorder,
            nextOperationId = { stage -> "op-$stage" },
            errorMapper = RuntimeErrorMapper()
        )

        val result = service.buildDiagnosticsPayload(0)

        assertFalse(result.ok)
        assertEquals("maxEntries must be greater than 0.", result.message)
    }

    @Test
    fun buildDiagnosticsPayload_runtimePathFailure_recordsDiagnosticAndReturnsPayload() = runBlocking {
        val recorder = RuntimeDiagnosticsRecorder(runtimePathsProvider = { null })
        val bundleStatus = RuntimeConfigBundleStatus(
            ok = true,
            schemaVersion = 1,
            profile = "android",
            bundleName = "main",
            requiredFiles = listOf("converter/a.toml"),
            missingFiles = emptyList(),
            bundlePath = "/tmp/bundle.toml",
            message = "bundle ok",
            validatedAtEpochMs = 1L
        )
        val service = RuntimeDiagnosticsService(
            ensureRuntimePaths = { throw IllegalStateException("boom") },
            bundleStatusProvider = { bundleStatus },
            diagnosticsRecorder = recorder,
            nextOperationId = { stage -> "op-$stage" },
            errorMapper = RuntimeErrorMapper()
        )

        val result = service.buildDiagnosticsPayload(10)

        assertTrue(result.ok)
        assertEquals(1, result.entryCount)
        assertTrue(result.message.contains("Prepared diagnostics payload"))
        assertTrue(result.payload.contains("bundle.ok=true"))
        assertTrue(result.payload.contains("diagnostics.prepare_runtime_paths"))
        assertTrue(result.payload.contains("prepare runtime paths failed: boom"))
        assertEquals(1, recorder.recent(10).size)
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
