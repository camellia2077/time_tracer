package com.example.tracer

import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class RuntimeInitServiceTest {
    @Test
    fun initializeRuntime_success_passthroughsInternalResult() = runBlocking {
        val expected = NativeCallResult(
            initialized = true,
            operationOk = true,
            rawResponse = """{"ok":true}"""
        )
        val service = RuntimeInitService(
            initializeRuntimeInternal = { expected },
            clearRuntimeData = { "unused" },
            resetRuntimeCaches = {}
        )

        val actual = service.initializeRuntime()

        assertEquals(expected, actual)
    }

    @Test
    fun initializeRuntime_exception_returnsFailureEnvelope() = runBlocking {
        val service = RuntimeInitService(
            initializeRuntimeInternal = { throw IllegalStateException("boom") },
            clearRuntimeData = { "unused" },
            resetRuntimeCaches = {}
        )

        val actual = service.initializeRuntime()

        assertFalse(actual.initialized)
        assertFalse(actual.operationOk)
        assertTrue(actual.rawResponse.contains("nativeInit failed"))
    }

    @Test
    fun clearAndReinitialize_success_runsClearResetAndInitInOrder() = runBlocking {
        val callOrder = mutableListOf<String>()
        val service = RuntimeInitService(
            initializeRuntimeInternal = {
                callOrder += "init"
                NativeCallResult(
                    initialized = true,
                    operationOk = true,
                    rawResponse = """{"ok":true}"""
                )
            },
            clearRuntimeData = {
                callOrder += "clear"
                "clear -> removed"
            },
            resetRuntimeCaches = {
                callOrder += "reset"
            }
        )

        val actual = service.clearAndReinitialize()

        assertEquals(listOf("clear", "reset", "init"), callOrder)
        assertTrue(actual.initialized)
        assertTrue(actual.operationOk)
        assertEquals("clear -> removed", actual.clearMessage)
    }

    @Test
    fun clearAndReinitialize_exception_returnsFailureResult() = runBlocking {
        val service = RuntimeInitService(
            initializeRuntimeInternal = {
                NativeCallResult(
                    initialized = true,
                    operationOk = true,
                    rawResponse = """{"ok":true}"""
                )
            },
            clearRuntimeData = { throw IllegalArgumentException("cannot delete") },
            resetRuntimeCaches = {}
        )

        val actual = service.clearAndReinitialize()

        assertFalse(actual.initialized)
        assertFalse(actual.operationOk)
        assertEquals("clear -> failed", actual.clearMessage)
        assertTrue(actual.initResponse.contains("clear and reinitialize failed"))
    }
}
