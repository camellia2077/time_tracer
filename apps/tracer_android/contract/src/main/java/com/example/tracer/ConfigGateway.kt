package com.example.tracer

interface ConfigGateway {
    suspend fun listConfigTomlFiles(): ConfigTomlListResult
    suspend fun readConfigTomlFile(relativePath: String): TxtFileContentResult
    suspend fun saveConfigTomlFile(relativePath: String, content: String): TxtFileContentResult
}
