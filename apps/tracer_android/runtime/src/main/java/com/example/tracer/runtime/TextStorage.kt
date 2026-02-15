package com.example.tracer

internal interface TextStorage {
    fun createCurrentMonthFile(): EnsureMonthFileResult
    fun createMonthFile(year: Int, month: Int): EnsureMonthFileResult
    fun listTxtFiles(): TxtHistoryListResult
    fun readTxtFile(relativePath: String): TxtFileContentResult
    fun writeTxtFile(relativePath: String, content: String): TxtFileContentResult
}

internal class LiveRawTextStorage(
    private val liveRawInputPath: String,
    private val recordStore: LiveRawRecordStore
) : TextStorage {
    override fun createCurrentMonthFile(): EnsureMonthFileResult {
        return recordStore.ensureCurrentMonthFile(liveRawInputPath)
    }

    override fun createMonthFile(year: Int, month: Int): EnsureMonthFileResult {
        return recordStore.ensureMonthFile(
            liveRawInputPath = liveRawInputPath,
            year = year,
            month = month
        )
    }

    override fun listTxtFiles(): TxtHistoryListResult {
        return recordStore.listTxtFiles(liveRawInputPath)
    }

    override fun readTxtFile(relativePath: String): TxtFileContentResult {
        return recordStore.readTxtFile(liveRawInputPath, relativePath)
    }

    override fun writeTxtFile(relativePath: String, content: String): TxtFileContentResult {
        return recordStore.writeTxtFile(
            liveRawInputPath = liveRawInputPath,
            relativePath = relativePath,
            content = content
        )
    }
}
