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
            }

            val originalFailureMessage = buildString {
                append(paths.inputRootPath.replace('\\', '/'))
                append("/2026/2026-03.txt:4: Line 4: Unrecognized line format: r")
                append("\nFull error report: ")
                append(root.resolve("missing-errors.log").absolutePath)
            }

            val delegate = RuntimeRecordDelegate(
                ensureRuntimePaths = { paths },
                ensureTextStorage = { storage },
                rawRecordStore = InputRecordStore(),
                responseCodec = NativeResponseCodec(),
                recordTranslator = NativeRecordTranslator(NativeResponseCodec()),
                inspectTxtFilesInternal = {
                    TxtInspectionResult(
                        ok = true,
                        entries = emptyList(),
                        message = "ok"
                    )
                },
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
                nativeValidateStructure = { """{"ok":true}""" },
                nativeValidateLogic = { _, _ -> """{"ok":true}""" },
                nativeIngestSingleTxtReplaceMonth = { _, _, _ -> """{"ok":true}""" },
                nativeClearTxtIngestSyncStatus = { """{"ok":true}""" }
            )

            val result = delegate.saveTxtFileAndSync(
                relativePath = "2026/2026-03.txt",
                content = "y2026\nm03\n0101\nr\n"
            )

            assertFalse(result.ok)
            assertEquals(
                "save blocked, validate failed: $originalFailureMessage",
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
