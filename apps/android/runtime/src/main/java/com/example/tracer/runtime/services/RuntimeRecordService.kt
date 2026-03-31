package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

internal class RuntimeRecordService(
    private val recordDelegate: RuntimeRecordDelegate,
    private val storageDelegate: RuntimeStorageDelegate,
    private val clearTxtData: () -> ClearTxtResult,
    private val clearTxtIngestSyncStatus: () -> NativeCallResult
) {
    suspend fun createCurrentMonthTxt(): RecordActionResult =
        recordDelegate.createCurrentMonthTxt()

    suspend fun createMonthTxt(month: String): RecordActionResult =
        recordDelegate.createMonthTxt(month)

    suspend fun recordNow(
        activityName: String,
        remark: String,
        targetDateIso: String?,
        preferredTxtPath: String?,
        timeOrderMode: RecordTimeOrderMode
    ): RecordActionResult =
        recordDelegate.recordNow(activityName, remark, targetDateIso, preferredTxtPath, timeOrderMode)

    suspend fun syncLiveToDatabase(): NativeCallResult =
        recordDelegate.syncLiveToDatabase()

    suspend fun inspectTxtFiles(): TxtInspectionResult =
        storageDelegate.inspectTxtFiles()

    suspend fun listTxtFiles(): TxtHistoryListResult =
        storageDelegate.listTxtFiles()

    suspend fun readTxtFile(relativePath: String): TxtFileContentResult =
        storageDelegate.readTxtFile(relativePath)

    suspend fun saveTxtFileAndSync(relativePath: String, content: String): RecordActionResult =
        recordDelegate.saveTxtFileAndSync(relativePath, content)

    suspend fun clearTxt(): ClearTxtResult = withContext(Dispatchers.IO) {
        try {
            val txtResult = clearTxtData()
            val syncResult = clearTxtIngestSyncStatus()
            val syncPayload = NativeResponseCodec().parse(syncResult.rawResponse)
            val syncMessage = if (syncResult.initialized && syncResult.operationOk) {
                "clear ingest sync -> ok"
            } else {
                "clear ingest sync -> ${syncPayload.errorMessage.ifEmpty { syncResult.rawResponse }}"
            }

            ClearTxtResult(
                ok = txtResult.ok && syncResult.initialized && syncResult.operationOk,
                message = "${txtResult.message}\n$syncMessage"
            )
        } catch (error: Exception) {
            ClearTxtResult(
                ok = false,
                message = formatNativeFailure("clear txt failed", error)
            )
        }
    }
}
