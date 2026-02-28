package com.example.tracer

interface RuntimeInitializer {
    suspend fun initializeRuntime(): NativeCallResult
    suspend fun ingestFull(): NativeCallResult
    suspend fun ingestSingleTxtReplaceMonth(inputPath: String): NativeCallResult
    suspend fun clearAndReinitialize(): ClearAndInitResult
}
