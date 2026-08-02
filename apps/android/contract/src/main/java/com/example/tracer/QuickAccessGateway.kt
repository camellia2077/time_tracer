package com.example.tracer

interface QuickAccessGateway {
    suspend fun readQuickAccess(): QuickAccessResult

    suspend fun writeQuickAccess(aliases: List<String>): QuickAccessResult
}
