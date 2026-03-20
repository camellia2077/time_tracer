package com.example.tracer

internal class RuntimeConfigService(
    private val storageDelegate: RuntimeStorageDelegate
) {
    suspend fun listConfigTomlFiles(): ConfigTomlListResult =
        storageDelegate.listConfigTomlFiles()

    suspend fun readConfigTomlFile(relativePath: String): TxtFileContentResult =
        storageDelegate.readConfigTomlFile(relativePath)

    suspend fun saveConfigTomlFile(relativePath: String, content: String): TxtFileContentResult =
        storageDelegate.saveConfigTomlFile(relativePath, content)
}
