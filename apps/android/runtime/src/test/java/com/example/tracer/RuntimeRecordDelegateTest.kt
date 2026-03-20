package com.example.tracer

import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import java.io.File
import java.nio.file.Files

class RuntimeRecordDelegateTest {
    @Test
    fun saveTxtFileAndSync_validateFailureDoesNotReadFullErrorReportFile() = runBlocking {
        val root = Files.createTempDirectory("runtime-record-delegate").toFile()
        try {
            val paths = createPaths(root)
            val storage = object : TextStorage {
                override fun listTxtFiles(): TxtHistoryListResult = TxtHistoryListResult(
                    ok = true,
                    files = emptyList(),
                    message = "ok"
                )

                override fun readTxtFile(relativePath: String): TxtFileContentResult = TxtFileContentResult(
                    ok = true,
                    filePath = relativePath,
                    content = "",
                    message = "ok"
                )

                override fun writeTxtFile(relativePath: String, content: String): TxtFileContentResult =
                    TxtFileContentResult(
                        ok = true,
                        filePath = relativePath,
                        content = content,
                        message = "saved"
                    )

                override fun resolveIngestInputPath(relativePath: String): String? = paths.liveRawInputPath

                override fun resolveSourceTarget(relativePath: String): Pair<String, String>? =
                    paths.liveRawInputPath to "2026-03.txt"
            }

            val originalFailureMessage = buildString {
                append(paths.liveRawInputPath.replace('\\', '/'))
                append("/2026-03.txt:4: Line 4: Unrecognized line format: r")
                append("\nFull error report: ")
                append(root.resolve("missing-errors.log").absolutePath)
            }

            val delegate = RuntimeRecordDelegate(
                ensureRuntimePaths = { paths },
                ensureTextStorage = { storage },
                rawRecordStore = LiveRawRecordStore(),
                responseCodec = NativeResponseCodec(),
                recordTranslator = NativeRecordTranslator(NativeResponseCodec()),
                executeAfterInit = { operationName, _ ->
                    when (operationName) {
                        "native_validate_structure" -> NativeCallResult(
                            initialized = true,
                            operationOk = false,
                            rawResponse = buildNativeErrorResponseJson(
                                errorMessage = originalFailureMessage
                            )
                        )

                        else -> NativeCallResult(
                            initialized = true,
                            operationOk = true,
                            rawResponse = """{"ok":true}"""
                        )
                    }
                },
                syncLiveOperation = { """{"ok":true}""" }
            )

            val result = delegate.saveTxtFileAndSync(
                relativePath = "live_raw/2026-03.txt",
                content = "y2026\nm03\n0101\nr\n"
            )

            assertFalse(result.ok)
            assertEquals(
                "save ok, validate failed: $originalFailureMessage",
                result.message
            )
            assertFalse(result.message.contains("Full report read failed"))
            assertFalse(result.message.contains("--- Full Validate Report"))
            assertTrue(result.message.contains("Full error report:"))
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
