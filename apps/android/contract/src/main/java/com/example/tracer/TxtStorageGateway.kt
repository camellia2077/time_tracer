package com.example.tracer

interface TxtStorageGateway {
    suspend fun listTxtFiles(): TxtHistoryListResult
    suspend fun readTxtFile(relativePath: String): TxtFileContentResult
    suspend fun saveTxtFileAndSync(relativePath: String, content: String): RecordActionResult
}
