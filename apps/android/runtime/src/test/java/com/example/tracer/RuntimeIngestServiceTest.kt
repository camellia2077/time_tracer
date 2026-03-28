package com.example.tracer

import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import java.io.File
import java.nio.file.Files

class RuntimeIngestServiceTest {
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
                nativeValidateStructure = { """{"ok":true}""" },
                nativeValidateLogic = { _, _ -> """{"ok":true}""" },
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
                nativeValidateStructure = { """{"ok":true}""" },
                nativeValidateLogic = { _, _ -> """{"ok":true}""" },
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
            assertTrue(managedFile.parentFile?.name == "2026")
            val savedContent = managedFile.readText()
            assertFalse(savedContent.startsWith("\uFEFF"))
            assertFalse(savedContent.contains("\r"))
            assertTrue(savedContent.contains("y2026\nm03\n"))
            assertTrue(File(paths.cacheRootPath).isDirectory)
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun ingestSingleTxtReplaceMonth_invalidUtf8_returnsFailureBeforeExecutor() = runBlocking {
        val root = Files.createTempDirectory("runtime-ingest-single-invalid-utf8").toFile()
        try {
            val paths = createPaths(root)
            val source = File(root, "source.txt")
            source.writeBytes(byteArrayOf(0xFF.toByte()))
            var executorCalled = false
            val service = RuntimeIngestService(
                ensureRuntimePaths = { paths },
                executeAfterInit = { _, _ ->
                    executorCalled = true
                    NativeCallResult(initialized = true, operationOk = true, rawResponse = """{"ok":true}""")
                },
                nativeValidateStructure = { """{"ok":true}""" },
                nativeValidateLogic = { _, _ -> """{"ok":true}""" },
                nativeIngestSingleTxtReplaceMonth = { _, _, _ -> """{"ok":true}""" }
            )

            val result = service.ingestSingleTxtReplaceMonth(source.absolutePath)

            assertFalse(result.initialized)
            assertFalse(result.operationOk)
            assertTrue(result.rawResponse.contains("Invalid UTF-8"))
            assertFalse(executorCalled)
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun ingestSingleTxtReplaceMonth_structureValidateFailure_doesNotCopyOrIngest() = runBlocking {
        val root = Files.createTempDirectory("runtime-ingest-single-validate-structure").toFile()
        try {
            val paths = createPaths(root)
            val source = File(root, "source.txt")
            source.writeText("y2026\nm03\n2026-03-01 08:00|study|remark\n")
            var ingestCalled = false
            val service = RuntimeIngestService(
                ensureRuntimePaths = { paths },
                executeAfterInit = { operationName, action ->
                    val rawResponse = action(paths)
                    when (operationName) {
                        "native_validate_structure" -> NativeCallResult(
                            initialized = true,
                            operationOk = false,
                            rawResponse = buildNativeErrorResponseJson("structure invalid")
                        )

                        else -> NativeCallResult(
                            initialized = true,
                            operationOk = true,
                            rawResponse = rawResponse
                        )
                    }
                },
                nativeValidateStructure = { """{"ok":true}""" },
                nativeValidateLogic = { _, _ -> """{"ok":true}""" },
                nativeIngestSingleTxtReplaceMonth = { _, _, _ ->
                    ingestCalled = true
                    """{"ok":true}"""
                }
            )

            val result = service.ingestSingleTxtReplaceMonth(source.absolutePath)

            assertTrue(result.initialized)
            assertFalse(result.operationOk)
            assertTrue(result.rawResponse.contains("structure invalid"))
            assertFalse(File(paths.inputRootPath, "2026/2026-03.txt").exists())
            assertFalse(ingestCalled)
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun ingestSingleTxtReplaceMonth_logicValidateFailure_doesNotCopyOrIngest() = runBlocking {
        val root = Files.createTempDirectory("runtime-ingest-single-validate-logic").toFile()
        try {
            val paths = createPaths(root)
            val source = File(root, "source.txt")
            source.writeText("y2026\nm03\n2026-03-01 08:00|study|remark\n")
            var ingestCalled = false
            val service = RuntimeIngestService(
                ensureRuntimePaths = { paths },
                executeAfterInit = { operationName, action ->
                    val rawResponse = action(paths)
                    when (operationName) {
                        "native_validate_logic" -> NativeCallResult(
                            initialized = true,
                            operationOk = false,
                            rawResponse = buildNativeErrorResponseJson("logic invalid")
                        )

                        else -> NativeCallResult(
                            initialized = true,
                            operationOk = true,
                            rawResponse = rawResponse
                        )
                    }
                },
                nativeValidateStructure = { """{"ok":true}""" },
                nativeValidateLogic = { _, _ -> """{"ok":true}""" },
                nativeIngestSingleTxtReplaceMonth = { _, _, _ ->
                    ingestCalled = true
                    """{"ok":true}"""
                }
            )

            val result = service.ingestSingleTxtReplaceMonth(source.absolutePath)

            assertTrue(result.initialized)
            assertFalse(result.operationOk)
            assertTrue(result.rawResponse.contains("logic invalid"))
            assertFalse(File(paths.inputRootPath, "2026/2026-03.txt").exists())
            assertFalse(ingestCalled)
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
