package com.example.tracer

import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import com.example.tracer.data.ActivityCategoryColorPreferenceWriter

internal class ActivityHierarchyEditorViewModelFactory(
    private val configGateway: ConfigGateway,
    private val activityHierarchyGateway: ActivityHierarchyGateway,
    private val activityHierarchyMigrationGateway: ActivityHierarchyMigrationGateway,
    private val quickActivitiesPreferenceGateway: QuickActivitiesPreferenceGateway,
    private val activityCategoryColorPreferenceWriter: ActivityCategoryColorPreferenceWriter
) : ViewModelProvider.Factory {
    override fun <T : ViewModel> create(modelClass: Class<T>): T {
        if (modelClass.isAssignableFrom(ActivityHierarchyEditorViewModel::class.java)) {
            @Suppress("UNCHECKED_CAST")
            return ActivityHierarchyEditorViewModel(
                configGateway = configGateway,
                activityHierarchyGateway = activityHierarchyGateway,
                activityHierarchyMigrationGateway = activityHierarchyMigrationGateway,
                quickActivitiesPreferenceGateway = quickActivitiesPreferenceGateway,
                activityCategoryColorPreferenceWriter = activityCategoryColorPreferenceWriter
            ) as T
        }
        throw IllegalArgumentException("Unknown ViewModel class: ${modelClass.name}")
    }
}
