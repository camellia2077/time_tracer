package com.example.tracer

import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import org.json.JSONObject
import java.io.File
import java.nio.file.Files

class RuntimeRecordDelegateTest {
    @Test
    fun recordNow_atomicFailureKeepsExistingTxtUnchanged() = runBlocking {
        val root = Files.createTempDirectory("runtime-record-delegate-atomic").toFile()
        try {
            val paths = createPaths(root)
            val targetFile = File(paths.inputRootPath, "2026/2026-03.txt").apply {
                parentFile?.mkdirs()
                writeText("y2026\nm03\n0329\n0900study_cpp\n")
            }
            val originalContent = targetFile.readText()

            val delegate = RuntimeRecordDelegate(
                ensureRuntimePaths = { paths },
                ensureTextStorage = {
                    object : TextStorage {
                        override fun listTxtFiles(): TxtHistoryListResult = TxtHistoryListResult(
                            ok = true,
                            files = listOf("2026/2026-03.txt"),
                            message = "ok"
                        )

                        override fun readTxtFile(relativePath: String): TxtFileContentResult = TxtFileContentResult(
                            ok = true,
                            filePath = relativePath,
                            content = targetFile.readText(),
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
                },
                rawRecordStore = InputRecordStore(),
                loadWakeKeywords = {
                    ActivityMappingNamesResult(ok = true, names = listOf("w", "wake"), message = "ok")
                },
                responseCodec = NativeResponseCodec(),
                atomicRecordCodec = NativeAtomicRecordCodec(),
                recordTranslator = NativeRecordTranslator(NativeResponseCodec()),
                inspectTxtFilesInternal = {
                    TxtInspectionResult(
                        ok = true,
                        entries = emptyList(),
                        message = "ok"
                    )
                },
                executeAfterInit = { _, action ->
                    NativeCallResult(
                        initialized = true,
                        operationOk = false,
                        rawResponse = action(paths),
                        operationId = "op-record-1"
                    )
                },
                nativeValidateStructure = { """{"ok":true}""" },
                nativeValidateLogic = { _, _ -> """{"ok":true}""" },
                nativeRecordActivityAtomically = { _, _, _, _, _, _ ->
                    JSONObject()
                        .put("ok", false)
                        .put(
                            "content",
                            JSONObject()
                                .put("ok", false)
                                .put("message", "Atomic record ingest failed. Official TXT has been rolled back: bad ingest")
                                .put("operation_id", "txn-1")
                                .put("warnings", emptyList<String>())
                                .put("rollback_failed", false)
                                .toString()
                        )
                        .put("error_message", "Atomic record ingest failed. Official TXT has been rolled back: bad ingest")
                        .toString()
                },
                nativeIngestSingleTxtReplaceMonth = { _, _, _ -> """{"ok":true}""" },
                nativeClearTxtIngestSyncStatus = { """{"ok":true}""" }
            )

            val result = delegate.recordNow(
                activityName = "study_cpp",
                remark = "remark",
                targetDateIso = "2026-03-29",
                preferredTxtPath = "2026/2026-03.txt",
                timeOrderMode = RecordTimeOrderMode.STRICT_CALENDAR
            )

            assertFalse(result.ok)
            assertEquals(
                "Atomic record ingest failed. Official TXT has been rolled back: bad ingest [op=op-record-1]",
                result.message
            )
            assertEquals(originalContent, targetFile.readText())
        } finally {
            root.deleteRecursively()
        }
    }

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
                loadWakeKeywords = {
                    ActivityMappingNamesResult(ok = true, names = listOf("w", "wake"), message = "ok")
                },
                responseCodec = NativeResponseCodec(),
                atomicRecordCodec = NativeAtomicRecordCodec(),
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
                nativeRecordActivityAtomically = { _, _, _, _, _, _ -> """{"ok":true}""" },
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

    @Test
    fun saveTxtFileAndSync_successAppendsGenericCompletenessWarning() = runBlocking {
        val root = Files.createTempDirectory("runtime-record-delegate-save-generic").toFile()
        try {
            val paths = createPaths(root)
            val delegate = createSaveSuccessDelegate(paths)

            val result = delegate.saveTxtFileAndSync(
                relativePath = "2026/2026-03.txt",
                content = "y2026\nm03\n0301\n0700w\n"
            )

            assertTrue(result.ok)
            assertTrue(result.message.contains("save+re-import -> 2026/2026-03.txt"))
            assertTrue(
                result.message.contains(
                    "Warning: this day currently has fewer than 2 authored events, so some intervals may not be computable yet."
                )
            )
            assertFalse(
                result.message.contains(
                    "Warning: possible overnight continuation; the first event of this day is not wake-related, so no sleep activity will be auto-generated."
                )
            )
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun saveTxtFileAndSync_successAppendsOvernightCompletenessWarning() = runBlocking {
        val root = Files.createTempDirectory("runtime-record-delegate-save-overnight").toFile()
        try {
            val paths = createPaths(root)
            val delegate = createSaveSuccessDelegate(paths)

            val result = delegate.saveTxtFileAndSync(
                relativePath = "2026/2026-03.txt",
                content = "y2026\nm03\n0301\n0700bilibili\n"
            )

            assertTrue(result.ok)
            assertTrue(result.message.contains("save+re-import -> 2026/2026-03.txt"))
            assertTrue(
                result.message.contains(
                    "Warning: possible overnight continuation; the first event of this day is not wake-related, so no sleep activity will be auto-generated."
                )
            )
            assertFalse(
                result.message.contains(
                    "Warning: this day currently has fewer than 2 authored events, so some intervals may not be computable yet."
                )
            )
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun saveTxtFileAndSync_successOmitsCompletenessWarningForCompleteDay() = runBlocking {
        val root = Files.createTempDirectory("runtime-record-delegate-save-complete").toFile()
        try {
            val paths = createPaths(root)
            val delegate = createSaveSuccessDelegate(paths)

            val result = delegate.saveTxtFileAndSync(
                relativePath = "2026/2026-03.txt",
                content = "y2026\nm03\n0301\n0700w\n0800bilibili\n"
            )

            assertTrue(result.ok)
            assertEquals("save+re-import -> 2026/2026-03.txt", result.message)
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun saveTxtFileAndSync_usesConfigDrivenWakeKeywordsForCompletenessScan() = runBlocking {
        val root = Files.createTempDirectory("runtime-record-delegate-save-config-wake").toFile()
        try {
            val paths = createPaths(root)
            val delegate = createSaveSuccessDelegate(
                paths = paths,
                wakeKeywordsResult = ActivityMappingNamesResult(
                    ok = true,
                    names = listOf("riseup"),
                    message = "ok"
                )
            )

            val result = delegate.saveTxtFileAndSync(
                relativePath = "2026/2026-03.txt",
                content = "y2026\nm03\n0301\n0700riseup\n"
            )

            assertTrue(result.ok)
            assertTrue(result.message.contains(InputRecordStore.INCOMPLETE_DAY_WARNING))
            assertFalse(result.message.contains(InputRecordStore.OVERNIGHT_CONTINUATION_WARNING))
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun saveTxtFileAndSync_wakeKeywordQueryFailureFallsBackToGenericWarning() = runBlocking {
        val root = Files.createTempDirectory("runtime-record-delegate-save-wake-query-failure").toFile()
        try {
            val paths = createPaths(root)
            val delegate = createSaveSuccessDelegate(
                paths = paths,
                wakeKeywordsLoader = { throw IllegalStateException("query failed") }
            )

            val result = delegate.saveTxtFileAndSync(
                relativePath = "2026/2026-03.txt",
                content = "y2026\nm03\n0301\n0700bilibili\n"
            )

            assertTrue(result.ok)
            assertTrue(result.message.contains(InputRecordStore.INCOMPLETE_DAY_WARNING))
            assertFalse(result.message.contains(InputRecordStore.OVERNIGHT_CONTINUATION_WARNING))
        } finally {
            root.deleteRecursively()
        }
    }

    private fun createSaveSuccessDelegate(
        paths: RuntimePaths,
        wakeKeywordsResult: ActivityMappingNamesResult = ActivityMappingNamesResult(
            ok = true,
            names = listOf("w", "wake"),
            message = "ok"
        ),
        wakeKeywordsLoader: (suspend () -> ActivityMappingNamesResult)? = null
    ): RuntimeRecordDelegate {
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
        val resolvedWakeKeywordsLoader: suspend () -> ActivityMappingNamesResult =
            wakeKeywordsLoader ?: { wakeKeywordsResult }

        return RuntimeRecordDelegate(
            ensureRuntimePaths = { paths },
            ensureTextStorage = { storage },
            rawRecordStore = InputRecordStore(),
            loadWakeKeywords = resolvedWakeKeywordsLoader,
            responseCodec = NativeResponseCodec(),
            atomicRecordCodec = NativeAtomicRecordCodec(),
            recordTranslator = NativeRecordTranslator(NativeResponseCodec()),
            inspectTxtFilesInternal = {
                TxtInspectionResult(
                    ok = true,
                    entries = emptyList(),
                    message = "ok"
                )
            },
            executeAfterInit = { _, action ->
                NativeCallResult(
                    initialized = true,
                    operationOk = true,
                    rawResponse = action(paths),
                    operationId = "op-save-1"
                )
            },
            nativeValidateStructure = { """{"ok":true}""" },
            nativeValidateLogic = { _, _ -> """{"ok":true}""" },
            nativeRecordActivityAtomically = { _, _, _, _, _, _ -> """{"ok":true}""" },
            nativeIngestSingleTxtReplaceMonth = { _, _, _ -> """{"ok":true}""" },
            nativeClearTxtIngestSyncStatus = { """{"ok":true}""" }
        )
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
