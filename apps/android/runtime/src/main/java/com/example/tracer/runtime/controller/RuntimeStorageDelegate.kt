package com.example.tracer

import android.util.Log
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

private const val TXT_STORAGE_LOG_TAG = "TxtStorage"

internal class RuntimeStorageDelegate(
    private val ensureTextStorage: () -> TextStorage,
    private val ensureConfigTomlStorage: () -> ConfigTomlStorage,
    private val inspectTxtFilesInternal: () -> TxtInspectionResult
) {
    suspend fun inspectTxtFiles(): TxtInspectionResult = withContext(Dispatchers.IO) {
        try {
            inspectTxtFilesInternal()
        } catch (error: Exception) {
            TxtInspectionResult(
                ok = false,
                entries = emptyList(),
                message = formatNativeFailure("inspect txt failed", error)
            )
        }
    }

    suspend fun listTxtFiles(): TxtHistoryListResult = withContext(Dispatchers.IO) {
        try {
            val storage = ensureTextStorage()
            storage.listTxtFiles().also { result ->
                logInfo(
                    TXT_STORAGE_LOG_TAG,
                    "listTxtFiles ok=${result.ok} count=${result.files.size} files=${result.files}"
                )
            }
        } catch (error: Exception) {
            logError(TXT_STORAGE_LOG_TAG, "listTxtFiles exception", error)
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
            storage.readTxtFile(relativePath).also { result ->
                logInfo(
                    TXT_STORAGE_LOG_TAG,
                    "readTxtFile path=$relativePath ok=${result.ok} returnedPath=${result.filePath} content=${result.content.toLogSnippet()}"
                )
            }
        } catch (error: Exception) {
            logError(TXT_STORAGE_LOG_TAG, "readTxtFile exception path=$relativePath", error)
            TxtFileContentResult(
                ok = false,
                filePath = relativePath,
                content = "",
                message = formatNativeFailure("read txt failed", error)
            )
        }
    }

    suspend fun saveTxtFile(relativePath: String, content: String): TxtFileContentResult =
        withContext(Dispatchers.IO) {
            try {
                val storage = ensureTextStorage()
                storage.writeTxtFile(relativePath, content).also { result ->
                    logInfo(
                        TXT_STORAGE_LOG_TAG,
                        "saveTxtFile path=$relativePath ok=${result.ok} returnedPath=${result.filePath} content=${content.toLogSnippet()}"
                    )
                }
            } catch (error: Exception) {
                logError(TXT_STORAGE_LOG_TAG, "saveTxtFile exception path=$relativePath", error)
                TxtFileContentResult(
                    ok = false,
                    filePath = relativePath,
                    content = "",
                    message = formatNativeFailure("save txt failed", error)
                )
            }
        }

private fun logInfo(tag: String, message: String) {
    runCatching { Log.i(tag, message) }
}

private fun logError(tag: String, message: String, error: Throwable) {
    runCatching { Log.e(tag, message, error) }
}

private fun String.toLogSnippet(maxLength: Int = 160): String {
    val sanitized = replace("\r", "\\r").replace("\n", "\\n")
    return if (sanitized.length <= maxLength) {
        sanitized
    } else {
        sanitized.take(maxLength) + "...(${sanitized.length})"
    }
}

    suspend fun listConfigTomlFiles(): ConfigTomlListResult = withContext(Dispatchers.IO) {
        try {
            val storage = ensureConfigTomlStorage()
            storage.listTomlFiles()
        } catch (error: Exception) {
            ConfigTomlListResult(
                ok = false,
                aliasFiles = emptyList(),
                chartFiles = emptyList(),
                metaFiles = emptyList(),
                insightsFiles = emptyList(),
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

    suspend fun deleteConfigTomlFile(relativePath: String): TxtFileContentResult =
        withContext(Dispatchers.IO) {
            try {
                ensureConfigTomlStorage().deleteTomlFile(relativePath)
            } catch (error: Exception) {
                TxtFileContentResult(
                    ok = false,
                    filePath = relativePath,
                    content = "",
                    message = formatNativeFailure("delete config toml failed", error)
                )
            }
        }
}
