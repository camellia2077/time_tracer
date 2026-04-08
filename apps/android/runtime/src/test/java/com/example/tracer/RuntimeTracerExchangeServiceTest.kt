package com.example.tracer

import kotlinx.coroutines.runBlocking
import org.json.JSONObject
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Test

class RuntimeTracerExchangeServiceTest {
    @Test
    fun exportTracerExchange_success_mapsContentAndProgress() = runBlocking {
        var progressListener: ((String) -> Unit)? = null
        val service = RuntimeTracerExchangeService(
            responseCodec = NativeResponseCodec(),
            nativeExportTracerExchange = { inputPath, outputPath, passphrase, securityLevel, dateCheckMode ->
                assertEquals("input/root", inputPath)
                assertEquals("output/data.tracer", outputPath)
                assertEquals("secret", passphrase)
                assertEquals(FileCryptoSecurityLevel.MODERATE, securityLevel)
                assertEquals(NativeBridge.DATE_CHECK_CONTINUITY, dateCheckMode)
                progressListener?.invoke(
                    buildProgressJson(
                        operation = "encrypt",
                        phase = "encrypt",
                        currentFileDoneBytes = 32L,
                        currentFileTotalBytes = 64L,
                        overallDoneBytes = 32L,
                        overallTotalBytes = 64L
                    )
                )
                buildNativeOkResponse(
                    JSONObject()
                        .put("output_path", "/resolved/data.tracer")
                        .put("source_root_name", "data")
                        .put("payload_file_count", 2)
                        .put("converter_file_count", 3)
                        .put("manifest_included", true)
                        .toString()
                )
            },
            nativeExportTracerExchangeFromPayloadJson = { _, _ -> error("unused") },
            nativeImportTracerExchange = { _, _, _ -> error("unused") },
            nativeInspectTracerExchange = { _, _ -> error("unused") },
            setProgressListener = { progressListener = it }
        )

        val progressEvents = mutableListOf<FileCryptoProgressEvent>()
        val result = service.exportTracerExchange(
            inputPath = "input/root",
            outputPath = "output/data.tracer",
            passphrase = "secret",
            securityLevel = FileCryptoSecurityLevel.MODERATE,
            dateCheckMode = NativeBridge.DATE_CHECK_CONTINUITY,
            onProgress = progressEvents::add
        )

        assertTrue(result.ok)
        assertEquals("/resolved/data.tracer", result.outputPath)
        assertEquals("data", result.sourceRootName)
        assertEquals(2, result.payloadFileCount)
        assertEquals(3, result.converterFileCount)
        assertTrue(result.manifestIncluded)
        assertEquals(1, progressEvents.size)
        assertEquals(FileCryptoOperation.ENCRYPT, progressEvents.single().operation)
        assertEquals(FileCryptoPhase.ENCRYPT, progressEvents.single().phase)
        assertNull(progressListener)
    }

    @Test
    fun exportTracerExchangeFromPayload_success_mapsContentAndRequest() = runBlocking {
        var progressListener: ((String) -> Unit)? = null
        val service = RuntimeTracerExchangeService(
            responseCodec = NativeResponseCodec(),
            nativeExportTracerExchange = { _, _, _, _, _ -> error("unused") },
            nativeExportTracerExchangeFromPayloadJson = { requestJson, outputFd ->
                assertEquals(37, outputFd)
                val request = JSONObject(requestJson)
                assertEquals("data", request.getString("logical_source_root_name"))
                assertEquals("data.tracer", request.getString("output_display_name"))
                assertEquals("secret", request.getString("passphrase"))
                assertEquals("moderate", request.getString("security_level"))
                assertEquals(NativeBridge.DATE_CHECK_CONTINUITY, request.getInt("date_check_mode"))
                val payloadItems = request.getJSONArray("payload_items")
                assertEquals(1, payloadItems.length())
                val payload = payloadItems.getJSONObject(0)
                assertEquals("2026/2026-03.txt", payload.getString("relative_path_hint"))
                assertTrue(payload.getString("content").contains("y2026"))
                progressListener?.invoke(
                    buildProgressJson(
                        operation = "encrypt",
                        phase = "write_output",
                        currentFileDoneBytes = 64L,
                        currentFileTotalBytes = 64L,
                        overallDoneBytes = 64L,
                        overallTotalBytes = 64L
                    )
                )
                buildNativeOkResponse(
                    JSONObject()
                        .put("output_path", "data.tracer")
                        .put("source_root_name", "data")
                        .put("payload_file_count", 1)
                        .put("converter_file_count", 3)
                        .put("manifest_included", true)
                        .toString()
                )
            },
            nativeImportTracerExchange = { _, _, _ -> error("unused") },
            nativeInspectTracerExchange = { _, _ -> error("unused") },
            setProgressListener = { progressListener = it }
        )

        val progressEvents = mutableListOf<FileCryptoProgressEvent>()
        val result = service.exportTracerExchangeFromPayload(
            payloads = listOf(
                TracerExchangePayloadItem(
                    relativePathHint = "2026/2026-03.txt",
                    content = "y2026\nm03\n0301\n0600 study\n"
                )
            ),
            outputFd = 37,
            passphrase = "secret",
            securityLevel = FileCryptoSecurityLevel.MODERATE,
            dateCheckMode = NativeBridge.DATE_CHECK_CONTINUITY,
            onProgress = progressEvents::add
        )

        assertTrue(result.ok)
        assertEquals("data.tracer", result.outputPath)
        assertEquals("data", result.sourceRootName)
        assertEquals(1, result.payloadFileCount)
        assertEquals(3, result.converterFileCount)
        assertTrue(result.manifestIncluded)
        assertEquals(1, progressEvents.size)
        assertEquals(FileCryptoPhase.WRITE_OUTPUT, progressEvents.single().phase)
        assertNull(progressListener)
    }

    @Test
    fun importTracerExchange_success_mapsConfigFields() = runBlocking {
        var progressListener: ((String) -> Unit)? = null
        val service = RuntimeTracerExchangeService(
            responseCodec = NativeResponseCodec(),
            nativeExportTracerExchange = { _, _, _, _, _ -> error("unused") },
            nativeExportTracerExchangeFromPayloadJson = { _, _ -> error("unused") },
            nativeImportTracerExchange = { inputPath, workRoot, passphrase ->
                assertEquals("bundle.tracer", inputPath)
                assertEquals("work/root", workRoot)
                assertEquals("secret", passphrase)
                progressListener?.invoke(
                    buildProgressJson(
                        operation = "decrypt",
                        phase = "rebuild_database",
                        currentFileDoneBytes = 48L,
                        currentFileTotalBytes = 64L,
                        overallDoneBytes = 96L,
                        overallTotalBytes = 128L
                    )
                )
                buildNativeOkResponse(
                    JSONObject()
                        .put("source_root_name", "data")
                        .put("payload_file_count", 3)
                        .put("replaced_month_count", 3)
                        .put("preserved_month_count", 2)
                        .put("rebuilt_month_count", 5)
                        .put("text_root_updated", true)
                        .put("config_applied", true)
                        .put("database_rebuilt", true)
                        .put("backup_retained_root", "/resolved/backup")
                        .put("backup_cleanup_error", "cleanup failed")
                        .toString()
                )
            },
            nativeInspectTracerExchange = { _, _ -> error("unused") },
            setProgressListener = { progressListener = it }
        )

        val progressEvents = mutableListOf<FileCryptoProgressEvent>()
        val result = service.importTracerExchange(
            inputPath = "bundle.tracer",
            workRoot = "work/root",
            passphrase = "secret",
            onProgress = progressEvents::add
        )

        assertTrue(result.ok)
        assertEquals("data", result.sourceRootName)
        assertEquals(3, result.payloadFileCount)
        assertEquals(3, result.replacedMonthCount)
        assertEquals(2, result.preservedMonthCount)
        assertEquals(5, result.rebuiltMonthCount)
        assertTrue(result.textRootUpdated)
        assertTrue(result.configApplied)
        assertTrue(result.databaseRebuilt)
        assertEquals("/resolved/backup", result.backupRetainedRoot)
        assertEquals("cleanup failed", result.backupCleanupError)
        assertEquals(1, progressEvents.size)
        assertEquals(FileCryptoOperation.DECRYPT, progressEvents.single().operation)
        assertEquals(FileCryptoPhase.UNKNOWN, progressEvents.single().phase)
        assertNull(progressListener)
    }

    @Test
    fun inspectTracerExchange_success_mapsProducerMetadata() = runBlocking {
        var progressListener: ((String) -> Unit)? = null
        val service = RuntimeTracerExchangeService(
            responseCodec = NativeResponseCodec(),
            nativeExportTracerExchange = { _, _, _, _, _ -> error("unused") },
            nativeExportTracerExchangeFromPayloadJson = { _, _ -> error("unused") },
            nativeImportTracerExchange = { _, _, _ -> error("unused") },
            nativeInspectTracerExchange = { inputPath, passphrase ->
                assertEquals("bundle.tracer", inputPath)
                assertEquals("secret", passphrase)
                buildNativeOkResponse(
                    JSONObject()
                        .put("rendered_text", "Package:\n  producer_platform: android")
                        .put("input_path", "/resolved/bundle.tracer")
                        .put("source_root_name", "data")
                        .put("payload_file_count", 5)
                        .put("package_version", 4)
                        .put("producer_platform", "android")
                        .put("producer_app", "time_tracer_android")
                        .put("created_at_utc", "2026-03-20T08:00:00Z")
                        .toString()
                )
            },
            setProgressListener = { progressListener = it }
        )

        val result = service.inspectTracerExchange(
            inputPath = "bundle.tracer",
            passphrase = "secret"
        )

        assertTrue(result.ok)
        assertEquals("Package:\n  producer_platform: android", result.renderedText)
        assertEquals("/resolved/bundle.tracer", result.inputPath)
        assertEquals("data", result.sourceRootName)
        assertEquals(5, result.payloadFileCount)
        assertEquals(4, result.packageVersion)
        assertEquals("android", result.producerPlatform)
        assertEquals("time_tracer_android", result.producerApp)
        assertEquals("2026-03-20T08:00:00Z", result.createdAtUtc)
        assertNull(progressListener)
    }

    @Test
    fun inspectTracerExchange_blankPassphrase_returnsValidationFailure() = runBlocking {
        val service = RuntimeTracerExchangeService(
            responseCodec = NativeResponseCodec(),
            nativeExportTracerExchange = { _, _, _, _, _ -> error("unused") },
            nativeExportTracerExchangeFromPayloadJson = { _, _ -> error("unused") },
            nativeImportTracerExchange = { _, _, _ -> error("unused") },
            nativeInspectTracerExchange = { _, _ -> error("unused") },
            setProgressListener = {}
        )

        val result = service.inspectTracerExchange(
            inputPath = "bundle.tracer",
            passphrase = ""
        )

        assertFalse(result.ok)
        assertTrue(result.message.contains("passphrase must not be empty"))
    }

    private fun buildNativeOkResponse(content: String): String =
        JSONObject()
            .put("ok", true)
            .put("content", content)
            .put("error_message", "")
            .toString()

    private fun buildProgressJson(
        operation: String,
        phase: String,
        currentFileDoneBytes: Long,
        currentFileTotalBytes: Long,
        overallDoneBytes: Long,
        overallTotalBytes: Long
    ): String = JSONObject()
        .put("operation", operation)
        .put("phase", phase)
        .put("current_group_label", "group")
        .put("group_index", 1)
        .put("group_count", 1)
        .put("file_index_in_group", 1)
        .put("file_count_in_group", 1)
        .put("current_file_index", 1)
        .put("total_files", 1)
        .put("current_file_done_bytes", currentFileDoneBytes)
        .put("current_file_total_bytes", currentFileTotalBytes)
        .put("overall_done_bytes", overallDoneBytes)
        .put("overall_total_bytes", overallTotalBytes)
        .put("speed_bytes_per_sec", 0L)
        .put("remaining_bytes", 0L)
        .put("eta_seconds", 0L)
        .toString()
}
