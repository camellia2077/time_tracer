package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Assert.fail
import org.junit.Test
import java.io.File
import java.nio.file.Files

class RuntimeCoreAdapterTest {
    @Test
    fun initializeRuntimeInternal_success_returnsNativeEnvelopeAndRecordsDiagnostic() {
        var currentPaths: RuntimePaths? = null
        val paths = createPaths(Files.createTempDirectory("core-adapter-init").toFile())
        val recorder = RuntimeDiagnosticsRecorder(runtimePathsProvider = { currentPaths })
        val adapter = RuntimeCoreAdapter(
            ensureRuntimePaths = {
                currentPaths = paths
                paths
            },
            runtimePathsProvider = { currentPaths },
            nativeInit = { """{"ok":true,"content":"","error_message":""}""" },
            nativeQuery = { """{"ok":true,"content":"","error_message":""}""" },
            nativeTree = { """{"ok":true,"content":"","error_message":""}""" },
            responseCodec = NativeResponseCodec(),
            reportTranslator = NativeReportTranslator(NativeResponseCodec()),
            diagnosticsRecorder = recorder,
            nextOperationId = { stage -> "op-$stage" },
            errorMapper = RuntimeErrorMapper()
        )

        val result = adapter.initializeRuntimeInternal()

        assertTrue(result.initialized)
        assertTrue(result.operationOk)
        assertEquals("op-native_init", result.operationId)
        assertEquals("native_init", recorder.recent(10).first().stage)
    }

    @Test
    fun initializeRuntimeInternal_exception_recordsDiagnosticAndRethrows() {
        val recorder = RuntimeDiagnosticsRecorder(runtimePathsProvider = { null })
        val adapter = RuntimeCoreAdapter(
            ensureRuntimePaths = { throw IllegalStateException("boom") },
            runtimePathsProvider = { null },
            nativeInit = { """{"ok":true,"content":"","error_message":""}""" },
            nativeQuery = { """{"ok":true,"content":"","error_message":""}""" },
            nativeTree = { """{"ok":true,"content":"","error_message":""}""" },
            responseCodec = NativeResponseCodec(),
            reportTranslator = NativeReportTranslator(NativeResponseCodec()),
            diagnosticsRecorder = recorder,
            nextOperationId = { stage -> "op-$stage" },
            errorMapper = RuntimeErrorMapper()
        )

        try {
            adapter.initializeRuntimeInternal()
            fail("Expected initializeRuntimeInternal to rethrow the initialization error.")
        } catch (error: IllegalStateException) {
            assertEquals("boom", error.message)
        }

        val entry = recorder.recent(10).first()
        assertEquals("native_init", entry.stage)
        assertTrue(entry.message.contains("nativeInit failed: boom"))
    }

    @Test
    fun executeNativeDataQuery_usesOriginalOperationNameAndBridgePath() {
        var currentPaths: RuntimePaths? = null
        val paths = createPaths(Files.createTempDirectory("core-adapter-query").toFile())
        var observedPaths: RuntimePaths? = null
        var capturedRequest: DataQueryRequest? = null
        val adapter = RuntimeCoreAdapter(
            ensureRuntimePaths = {
                currentPaths = paths
                paths
            },
            runtimePathsProvider = { currentPaths },
            nativeInit = { """{"ok":true,"content":"","error_message":""}""" },
            nativeQuery = { request ->
                capturedRequest = request
                """{"ok":true,"content":"query-ok","error_message":""}"""
            },
            nativeTree = { """{"ok":true,"content":"","error_message":""}""" },
            responseCodec = NativeResponseCodec(),
            reportTranslator = NativeReportTranslator(NativeResponseCodec()),
            diagnosticsRecorder = RuntimeDiagnosticsRecorder(runtimePathsProvider = { currentPaths }),
            nextOperationId = { stage -> "op-$stage" },
            errorMapper = RuntimeErrorMapper()
        )

        val result = adapter.executeNativeDataQuery(
            request = DataQueryRequest(action = 42),
            onRuntimePaths = { observedPaths = it }
        )

        assertTrue(result.operationOk)
        assertEquals("op-native_query_42", result.operationId)
        assertEquals(42, capturedRequest?.action)
        assertEquals(paths, observedPaths)
    }

    @Test
    fun executeNativeTreeQuery_usesOriginalOperationNameAndBridgePath() {
        var currentPaths: RuntimePaths? = null
        val paths = createPaths(Files.createTempDirectory("core-adapter-tree").toFile())
        var capturedParams: DataTreeQueryParams? = null
        val adapter = RuntimeCoreAdapter(
            ensureRuntimePaths = {
                currentPaths = paths
                paths
            },
            runtimePathsProvider = { currentPaths },
            nativeInit = { """{"ok":true,"content":"","error_message":""}""" },
            nativeQuery = { """{"ok":true,"content":"","error_message":""}""" },
            nativeTree = { params ->
                capturedParams = params
                """{"ok":true,"content":"","error_message":""}"""
            },
            responseCodec = NativeResponseCodec(),
            reportTranslator = NativeReportTranslator(NativeResponseCodec()),
            diagnosticsRecorder = RuntimeDiagnosticsRecorder(runtimePathsProvider = { currentPaths }),
            nextOperationId = { stage -> "op-$stage" },
            errorMapper = RuntimeErrorMapper()
        )

        val params = DataTreeQueryParams(
            period = DataTreePeriod.MONTH,
            periodArgument = "202603",
            level = 2
        )
        val result = adapter.executeNativeTreeQuery(params)

        assertTrue(result.operationOk)
        assertEquals("op-native_tree", result.operationId)
        assertEquals(params, capturedParams)
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
