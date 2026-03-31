package com.example.tracer

enum class RecordTimeOrderMode {
    STRICT_CALENDAR,
    LOGICAL_DAY_0600
}

interface RecordGateway {
    suspend fun createCurrentMonthTxt(): RecordActionResult
    suspend fun createMonthTxt(month: String): RecordActionResult
    suspend fun recordNow(
        activityName: String,
        remark: String,
        targetDateIso: String?,
        preferredTxtPath: String?,
        timeOrderMode: RecordTimeOrderMode
    ): RecordActionResult
    suspend fun syncLiveToDatabase(): NativeCallResult
    suspend fun clearTxt(): ClearTxtResult
}
