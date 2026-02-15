package com.example.tracer

interface RuntimeInitializer {
    suspend fun initializeRuntime(): NativeCallResult
    suspend fun querySmoke(): NativeCallResult
    suspend fun ingestSmoke(): NativeCallResult
    suspend fun ingestFull(): NativeCallResult
    suspend fun clearAndReinitialize(): ClearAndInitResult
}
