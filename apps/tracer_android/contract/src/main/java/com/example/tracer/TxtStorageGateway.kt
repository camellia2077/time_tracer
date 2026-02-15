package com.example.tracer

interface TxtStorageGateway {
    suspend fun listLiveTxtFiles(): TxtHistoryListResult
    suspend fun readLiveTxtFile(relativePath: String): TxtFileContentResult
    suspend fun saveLiveTxtFileAndSync(relativePath: String, content: String): RecordActionResult
}
