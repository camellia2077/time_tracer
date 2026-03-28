package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import java.io.File
import java.nio.file.Files

class RuntimeDiagnosticsRecorderTest {
    @Test
    fun record_keepsMostRecentInMemoryWithoutWritingJsonl() {
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

            assertEquals("", recorder.diagnosticsLogPath())
            assertFalse(File(root, "output/logs/diagnostics.jsonl").exists())
        } finally {
            root.deleteRecursively()
        }
    }

    private fun createPaths(root: File): RuntimePaths {
        val input = File(root, "input").apply { mkdirs() }
        val cache = File(root, "cache").apply { mkdirs() }
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
            inputRootPath = input.absolutePath,
            cacheRootPath = cache.absolutePath
        )
    }
}
