package com.example.tracer

import com.example.tracer.data.UserPreferencesRepository
import kotlinx.coroutines.flow.first

internal interface QuickActivitiesPreferenceGateway {
    suspend fun getQuickActivities(): List<String>

    suspend fun setQuickActivities(values: List<String>)
}

internal class UserPreferencesQuickActivitiesGateway(
    private val repository: UserPreferencesRepository
) : QuickActivitiesPreferenceGateway {
    override suspend fun getQuickActivities(): List<String> {
        return repository.recordSuggestionPreferences.first().quickActivities
    }

    override suspend fun setQuickActivities(values: List<String>) {
        repository.setRecordQuickActivities(values)
    }
}
