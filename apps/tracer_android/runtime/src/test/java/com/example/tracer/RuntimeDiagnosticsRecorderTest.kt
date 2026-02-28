package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test
import java.io.File
import java.nio.file.Files

class RuntimeDiagnosticsRecorderTest {
    @Test
    fun record_writesJsonlAndKeepsMostRecentInMemory() {
        val root = Files.createTempDirectory("runtime-diagnostics").toFile()
        try {
            val paths = createPaths(root)
            val recorder = RuntimeDiagnosticsRecorder(
                runtimePathsProvider = { paths },
                maxInMemoryEntries = 4
            )

            recorder.record(
                RuntimeDiagnosticRecord(
                    timestampEpochMs = 1_000L,
                    operationId = "op-1",
                    stage = "native_query_1",
                    ok = true,
                    initialized = true,
                    message = "ok",
                    errorLogPath = ""
                )
            )
            recorder.record(
                RuntimeDiagnosticRecord(
                    timestampEpochMs = 2_000L,
                    operationId = "op-2",
                    stage = "native_report_day",
                    ok = false,
                    initialized = true,
                    message = "failed",
                    errorLogPath = "/tmp/error.log"
                )
            )

            val recent = recorder.recent(limit = 10)
            assertEquals(2, recent.size)
            assertEquals("op-2", recent[0].operationId)
            assertEquals("op-1", recent[1].operationId)

            val logFile = File(recorder.diagnosticsLogPath())
            assertTrue(logFile.exists())
            val lines = logFile.readLines().filter { it.isNotBlank() }
            assertEquals(2, lines.size)
            assertTrue(lines[0].contains("op-1"))
            assertTrue(lines[1].contains("op-2"))
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
