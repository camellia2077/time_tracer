package com.example.tracer

import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow

internal interface QuickActivitiesPreferenceGateway {
    suspend fun getQuickActivities(): List<String>

    suspend fun setQuickActivities(values: List<String>)

    fun clearCachedQuickActivities() = Unit
}

internal class RuntimeQuickActivitiesGateway(
    private val gateway: QuickAccessGateway
) : QuickActivitiesPreferenceGateway {
    private val _quickActivities = MutableStateFlow<List<String>>(emptyList())
    val quickActivities: StateFlow<List<String>> = _quickActivities.asStateFlow()

    override suspend fun getQuickActivities(): List<String> {
        val result = gateway.readQuickAccess()
        check(result.ok) { result.message.ifBlank { "Cannot read Quick Access." } }
        _quickActivities.value = result.aliases
        return result.aliases
    }

    override suspend fun setQuickActivities(values: List<String>) {
        val result = gateway.writeQuickAccess(values)
        check(result.ok) { result.message.ifBlank { "Cannot save Quick Access." } }
        _quickActivities.value = result.aliases
    }

    override fun clearCachedQuickActivities() {
        _quickActivities.value = emptyList()
    }
}
