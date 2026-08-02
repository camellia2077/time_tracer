package com.example.tracer

import kotlinx.coroutines.runBlocking
import org.json.JSONObject
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import java.io.File
import java.nio.file.Files

class RuntimeQuickAccessServiceTest {
    @Test
    fun missingFileReadsAsEmptyWithoutCallingCore() = runBlocking {
        val root = Files.createTempDirectory("quick-access-service").toFile()
        try {
            var coreCalls = 0
            val service = service(root) { request ->
                coreCalls += 1
                JSONObject().put("ok", false).toString()
            }

            val result = service.readQuickAccess()

            assertTrue(result.ok)
            assertTrue(result.aliases.isEmpty())
            assertEquals(0, coreCalls)
            assertFalse(File(root, "user/quick_access.toml").exists())
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun writeUsesCoreRenderedContentAndAndroidCreatesFile() = runBlocking {
        val root = Files.createTempDirectory("quick-access-service").toFile()
        try {
            val aliases = listOf("学习", "休息")
            var request: JSONObject? = null
            val service = service(root) { requestJson ->
                request = JSONObject(requestJson)
                JSONObject()
                    .put("ok", true)
                    .put("quick_access", org.json.JSONArray(aliases))
                    .put("toml_content", "quick_access = [\"学习\", \"休息\"]\n")
                    .toString()
            }

            val result = service.writeQuickAccess(aliases)

            assertTrue(result.ok)
            assertEquals(aliases, result.aliases)
            assertEquals("write_quick_access", request?.getString("action"))
            assertEquals(
                "quick_access = [\"学习\", \"休息\"]\n",
                File(root, "user/quick_access.toml").readText()
            )
        } finally {
            root.deleteRecursively()
        }
    }

    private fun service(
        root: File,
        nativeConfig: (String) -> String
    ): RuntimeQuickAccessService = RuntimeQuickAccessService(
        ensureConfigTomlStorage = { ConfigTomlStorage(root.absolutePath) },
        initializeRuntimeInternal = {
            NativeCallResult(initialized = true, operationOk = true, rawResponse = "{}")
        },
        nativeConfig = nativeConfig,
        codec = NativeTxtRuntimeCodec()
    )
}
