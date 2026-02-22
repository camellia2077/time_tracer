package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

internal class RuntimeStorageDelegate(
    private val ensureTextStorage: () -> TextStorage,
    private val ensureConfigTomlStorage: () -> ConfigTomlStorage
) {
    suspend fun listTxtFiles(): TxtHistoryListResult = withContext(Dispatchers.IO) {
        try {
            val storage = ensureTextStorage()
            storage.listTxtFiles()
        } catch (error: Exception) {
            TxtHistoryListResult(
                ok = false,
                files = emptyList(),
                message = formatNativeFailure("list txt failed", error)
            )
        }
    }

    suspend fun readTxtFile(relativePath: String): TxtFileContentResult = withContext(Dispatchers.IO) {
        try {
            val storage = ensureTextStorage()
            storage.readTxtFile(relativePath)
        } catch (error: Exception) {
            TxtFileContentResult(
                ok = false,
                filePath = relativePath,
                content = "",
                message = formatNativeFailure("read txt failed", error)
            )
        }
    }

    suspend fun listConfigTomlFiles(): ConfigTomlListResult = withContext(Dispatchers.IO) {
        try {
            val storage = ensureConfigTomlStorage()
            storage.listTomlFiles()
        } catch (error: Exception) {
            ConfigTomlListResult(
                ok = false,
                converterFiles = emptyList(),
                reportFiles = emptyList(),
                message = formatNativeFailure("list config toml failed", error)
            )
        }
    }

    suspend fun readConfigTomlFile(relativePath: String): TxtFileContentResult = withContext(Dispatchers.IO) {
        try {
            val storage = ensureConfigTomlStorage()
            storage.readTomlFile(relativePath)
        } catch (error: Exception) {
            TxtFileContentResult(
                ok = false,
                filePath = relativePath,
                content = "",
                message = formatNativeFailure("read config toml failed", error)
            )
        }
    }

    suspend fun saveConfigTomlFile(relativePath: String, content: String): TxtFileContentResult =
        withContext(Dispatchers.IO) {
            try {
                val storage = ensureConfigTomlStorage()
                storage.writeTomlFile(relativePath, content)
            } catch (error: Exception) {
                TxtFileContentResult(
                    ok = false,
                    filePath = relativePath,
                    content = "",
                    message = formatNativeFailure("save config toml failed", error)
                )
            }
        }
}
