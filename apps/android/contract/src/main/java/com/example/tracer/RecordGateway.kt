package com.example.tracer

interface RecordGateway {
    suspend fun createCurrentMonthTxt(): RecordActionResult
    suspend fun createMonthTxt(month: String): RecordActionResult
    suspend fun recordNow(
        activityName: String,
        remark: String,
        targetDateIso: String?,
        preferredTxtPath: String?
    ): RecordActionResult
    suspend fun syncLiveToDatabase(): NativeCallResult
    suspend fun clearTxt(): ClearTxtResult
}
