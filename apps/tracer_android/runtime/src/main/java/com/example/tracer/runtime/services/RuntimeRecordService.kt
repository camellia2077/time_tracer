package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

internal class RuntimeRecordService(
    private val recordDelegate: RuntimeRecordDelegate,
    private val storageDelegate: RuntimeStorageDelegate,
    private val clearTxtData: () -> ClearTxtResult
) {
    suspend fun createCurrentMonthTxt(): RecordActionResult =
        recordDelegate.createCurrentMonthTxt()

    suspend fun createMonthTxt(month: String): RecordActionResult =
        recordDelegate.createMonthTxt(month)

    suspend fun recordNow(
        activityName: String,
        remark: String,
        targetDateIso: String?,
        preferredTxtPath: String?
    ): RecordActionResult =
        recordDelegate.recordNow(activityName, remark, targetDateIso, preferredTxtPath)

    suspend fun syncLiveToDatabase(): NativeCallResult =
        recordDelegate.syncLiveToDatabase()

    suspend fun listTxtFiles(): TxtHistoryListResult =
        storageDelegate.listTxtFiles()

    suspend fun readTxtFile(relativePath: String): TxtFileContentResult =
        storageDelegate.readTxtFile(relativePath)

    suspend fun saveTxtFileAndSync(relativePath: String, content: String): RecordActionResult =
        recordDelegate.saveTxtFileAndSync(relativePath, content)

    suspend fun clearTxt(): ClearTxtResult = withContext(Dispatchers.IO) {
        try {
            clearTxtData()
        } catch (error: Exception) {
            ClearTxtResult(
                ok = false,
                message = formatNativeFailure("clear txt failed", error)
            )
        }
    }
}
