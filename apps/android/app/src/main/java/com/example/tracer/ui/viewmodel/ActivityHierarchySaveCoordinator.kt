package com.example.tracer

import com.example.tracer.data.ActivityCategoryColorPreferenceWriter
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch

internal class ActivityHierarchySaveCoordinator(
    private val configGateway: ConfigGateway,
    private val activityHierarchyGateway: ActivityHierarchyGateway,
    activityHierarchyMigrationGateway: ActivityHierarchyMigrationGateway,
    quickActivitiesPreferenceGateway: QuickActivitiesPreferenceGateway,
    private val activityCategoryColorPreferenceWriter: ActivityCategoryColorPreferenceWriter,
    private val configFileEditor: ActivityHierarchyFileEditor,
    private val scope: CoroutineScope,
    private val readState: () -> ActivityHierarchyEditorState,
    private val writeState: (ActivityHierarchyEditorState) -> Unit,
    private val refreshActivityCategories: () -> Unit
) {
    private var uiState: ActivityHierarchyEditorState
        get() = readState()
        set(value) = writeState(value)

    private val viewModelScope: CoroutineScope = scope

    private val activityHierarchyMigrationUseCase =
        ActivityHierarchyMigrationUseCase(
            gateway = activityHierarchyMigrationGateway,
            quickActivitiesPreferenceGateway = quickActivitiesPreferenceGateway
        )
    private val activityHierarchyEditCoordinator = ActivityHierarchyEditCoordinator(
        hierarchyGateway = activityHierarchyGateway,
        migrationUseCase = activityHierarchyMigrationUseCase
    )

    fun createAliasTomlFile(fileName: String) {
        viewModelScope.launch {
            uiState = configFileEditor.createAliasTomlFile(uiState, fileName)
        }
    }

    fun deleteCurrentAliasTomlFile() {
        val targetFilePath = uiState.selectedFilePath
        if (!isAliasConfigFilePath(targetFilePath)) {
            uiState = uiState.copy(statusText = "Select a canonical TOML file to delete.")
            return
        }
        viewModelScope.launch {
            val deleteResult = configGateway.deleteConfigTomlFile(targetFilePath)
            if (!deleteResult.ok) {
                uiState = uiState.copy(statusText = deleteResult.message)
                return@launch
            }
            activityCategoryColorPreferenceWriter.setInsightsActivityCategoryColor(
                targetFilePath,
                ""
            )
            reloadRuntimeAfterAliasConfigChange()?.let { message ->
                uiState = uiState.copy(statusText = message)
                return@launch
            }
            refreshActivityCategories()
            uiState = uiState.copy(statusText = "deleted canonical toml -> $targetFilePath")
        }
    }

    private suspend fun reloadRuntimeAfterAliasConfigChange(): String? {
        val reloadResult = (configGateway as? RuntimeInitializer)?.initializeRuntime() ?: return null
        return if (reloadResult.initialized) {
            null
        } else {
            "Canonical TOML was saved but runtime reload failed."
        }
    }

    /** Import path for canonical TOML; persistence still goes through Core + migration. */
    suspend fun applyImportedAliasToml(relativePath: String, updatedTomlContent: String): String? {
        if (!isAliasConfigFilePath(relativePath)) return "Not a canonical TOML path: $relativePath"
        val original = configGateway.readConfigTomlFile(relativePath)
        val result = activityHierarchyGateway.rewriteActivityHierarchyDocument(
            originalTomlContent = if (original.ok) original.content else updatedTomlContent,
            updatedTomlContent = updatedTomlContent
        )
        if (!result.ok) return result.message
        val duplicateMessage = validateAliasKeyUniqueness(
            configGateway = configGateway,
            activityHierarchyGateway = activityHierarchyGateway,
            aliasFiles = uiState.aliasFiles,
            currentFilePath = relativePath,
            currentTomlContent = result.updatedTomlContent
        )
        if (duplicateMessage != null) return duplicateMessage
        val document = result.hierarchy?.toActivityHierarchyDocument()
            ?: return "Activity hierarchy rewrite did not produce a document."
        return when (val outcome = activityHierarchyEditCoordinator.persistCoreResult(
            configRelativePath = relativePath,
            updatedTomlContent = result.updatedTomlContent,
            replacementPlan = result.replacementPlan,
            document = document,
            allowMissingConfig = !original.ok
        )) {
            is ActivityHierarchyEditOutcome.Applied -> null
            is ActivityHierarchyEditOutcome.Failed -> outcome.message
        }
    }

}
