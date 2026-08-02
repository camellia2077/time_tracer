package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.json.JSONArray
import org.json.JSONObject

internal class RuntimeQuickAccessService(
    private val ensureConfigTomlStorage: () -> ConfigTomlStorage,
    private val initializeRuntimeInternal: () -> NativeCallResult,
    private val nativeConfig: (String) -> String,
    private val codec: NativeTxtRuntimeCodec
) : QuickAccessGateway {
    private companion object {
        const val QUICK_ACCESS_PATH = "user/quick_access.toml"
    }

    override suspend fun readQuickAccess(): QuickAccessResult = withContext(Dispatchers.IO) {
        val stored = ensureConfigTomlStorage().readTomlFile(QUICK_ACCESS_PATH)
        if (!stored.ok && stored.message == "TOML file not found.") {
            return@withContext QuickAccessResult(ok = true)
        }
        if (!stored.ok) {
            return@withContext QuickAccessResult(ok = false, message = stored.message)
        }

        val initialized = initializeRuntimeInternal()
        if (!initialized.operationOk) {
            return@withContext QuickAccessResult(ok = false, message = "native init failed")
        }
        val response = nativeConfig(JSONObject()
            .put("action", "read_quick_access")
            .put("toml_content", stored.content)
            .toString())
        val parsed = codec.parseQuickAccess(response)
        QuickAccessResult(
            ok = parsed.ok,
            aliases = parsed.aliases,
            message = parsed.message
        )
    }

    override suspend fun writeQuickAccess(aliases: List<String>): QuickAccessResult =
        withContext(Dispatchers.IO) {
            val initialized = initializeRuntimeInternal()
            if (!initialized.operationOk) {
                return@withContext QuickAccessResult(ok = false, message = "native init failed")
            }
            val response = nativeConfig(JSONObject()
                .put("action", "write_quick_access")
                .put("quick_access", JSONArray(aliases))
                .toString())
            val rendered = codec.parseQuickAccess(response)
            if (!rendered.ok) {
                return@withContext QuickAccessResult(
                    ok = false,
                    message = rendered.message
                )
            }
            val saved = ensureConfigTomlStorage().writeTomlFile(
                QUICK_ACCESS_PATH,
                rendered.tomlContent
            )
            if (!saved.ok) {
                return@withContext QuickAccessResult(
                    ok = false,
                    message = saved.message
                )
            }
            QuickAccessResult(ok = true, aliases = rendered.aliases, message = saved.message)
        }
}
