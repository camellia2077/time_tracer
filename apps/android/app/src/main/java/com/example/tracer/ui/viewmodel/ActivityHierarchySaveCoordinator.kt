package com.example.tracer

import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch

internal class ActivityHierarchySaveCoordinator(
    private val configGateway: ConfigGateway,
    private val activityHierarchyGateway: ActivityHierarchyGateway,
    activityHierarchyMigrationGateway: ActivityHierarchyMigrationGateway,
    quickActivitiesPreferenceGateway: QuickActivitiesPreferenceGateway,
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

    fun saveCurrentFile() {
        val selectedFile = uiState.selectedFilePath
        if (selectedFile.isEmpty()) {
            uiState = uiState.copy(
                statusText = "No TOML file selected.",
                autoSaveStatus = ActivityHierarchySaveStatus.FAILED
            )
            return
        }
        uiState = uiState.copy(autoSaveStatus = ActivityHierarchySaveStatus.SAVING)
        viewModelScope.launch {
            saveAliasFile(selectedFile)
        }
    }

    fun discardUnsavedDraft() {
        val selectedFile = uiState.selectedFilePath
        val baseline = uiState.aliasBaselineDocument
        uiState = uiState.copy(
            aliasEditorMode = AliasEditorMode.STRUCTURED,
            aliasDocumentDraft = baseline,
            aliasAdvancedTomlDraft = uiState.selectedFileContent,
            aliasStructuredDraftsByFile = uiState.aliasStructuredDraftsByFile - selectedFile,
            aliasAdvancedDraftsByFile = uiState.aliasAdvancedDraftsByFile - selectedFile,
            aliasEditorModeByFile = uiState.aliasEditorModeByFile - selectedFile,
            aliasEditorErrorMessage = if (baseline == null) {
                "Activity hierarchy is unavailable for this file."
            } else {
                ""
            }
        )
    }

    private suspend fun saveAliasFile(selectedFile: String) {
        uiState = when (uiState.aliasEditorMode) {
            AliasEditorMode.STRUCTURED -> saveStructuredAliasFile(selectedFile)
            AliasEditorMode.ADVANCED -> saveAdvancedAliasFile(selectedFile)
        }
    }

    private suspend fun saveStructuredAliasFile(selectedFile: String): ActivityHierarchyEditorState {
        val document = uiState.aliasDocumentDraft
            ?: return uiState.copy(
                statusText = "Alias editor is unavailable for this file.",
                autoSaveStatus = ActivityHierarchySaveStatus.FAILED
            )
        // Structured operations are already persisted by
        // applyCoreActivityHierarchyOperation through the migration service.
        // This button only clears presentation drafts; it must never write
        // canonical TOML directly from Android.
        return uiState.copy(
            aliasEditorMode = AliasEditorMode.STRUCTURED,
            aliasStructuredDraftsByFile = uiState.aliasStructuredDraftsByFile - selectedFile,
            aliasAdvancedDraftsByFile = uiState.aliasAdvancedDraftsByFile - selectedFile,
            aliasEditorModeByFile = uiState.aliasEditorModeByFile + (selectedFile to AliasEditorMode.STRUCTURED),
            aliasEditorErrorMessage = "",
            autoSaveStatus = ActivityHierarchySaveStatus.SAVED,
            statusText = "canonical TOML already persisted by core"
        )
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

    private suspend fun saveAdvancedAliasFile(selectedFile: String): ActivityHierarchyEditorState {
        val rewritten = activityHierarchyGateway.rewriteActivityHierarchyDocument(
            originalTomlContent = uiState.selectedFileContent,
            updatedTomlContent = uiState.aliasAdvancedTomlDraft
        )
        val document = rewritten.hierarchy?.toActivityHierarchyDocument()
            ?: return uiState.copy(
                aliasEditorErrorMessage = rewritten.message,
                autoSaveStatus = ActivityHierarchySaveStatus.FAILED,
                statusText = rewritten.message
            )
        if (!rewritten.ok) {
            return uiState.copy(
                aliasEditorErrorMessage = rewritten.message,
                autoSaveStatus = ActivityHierarchySaveStatus.FAILED,
                statusText = rewritten.message
            )
        }
        val duplicateMessage = validateAliasKeyUniqueness(
            configGateway = configGateway,
            activityHierarchyGateway = activityHierarchyGateway,
            aliasFiles = uiState.aliasFiles,
            currentFilePath = selectedFile,
            currentTomlContent = rewritten.updatedTomlContent
        )
        if (duplicateMessage != null) {
            return uiState.copy(
                aliasEditorErrorMessage = duplicateMessage,
                autoSaveStatus = ActivityHierarchySaveStatus.FAILED,
                statusText = duplicateMessage
            )
        }
        uiState = uiState.copy(autoSaveStatus = ActivityHierarchySaveStatus.SAVING)
        val outcome = activityHierarchyEditCoordinator.persistCoreResult(
            configRelativePath = selectedFile,
            updatedTomlContent = rewritten.updatedTomlContent,
            replacementPlan = rewritten.replacementPlan,
            document = document
        )
        if (outcome is ActivityHierarchyEditOutcome.Failed) {
            return uiState.copy(
                statusText = outcome.message,
                aliasEditorErrorMessage = outcome.message,
                autoSaveStatus = ActivityHierarchySaveStatus.FAILED
            )
        }
        val applied = outcome as ActivityHierarchyEditOutcome.Applied
        val migratedToml = applied.renderedToml
        val aliasParentOptions = resolveAliasParentOptions(
            configGateway = configGateway,
            activityHierarchyGateway = activityHierarchyGateway,
            aliasFiles = uiState.aliasFiles,
            selectedFilePath = selectedFile,
            selectedFileContent = migratedToml
        )
        return uiState.copy(
            selectedFileContent = migratedToml,
            aliasDocumentDraft = document,
            aliasParentOptions = aliasParentOptions,
            aliasAdvancedTomlDraft = migratedToml,
            aliasStructuredDraftsByFile = uiState.aliasStructuredDraftsByFile - selectedFile,
            aliasAdvancedDraftsByFile = uiState.aliasAdvancedDraftsByFile - selectedFile,
            aliasEditorModeByFile = uiState.aliasEditorModeByFile + (selectedFile to AliasEditorMode.ADVANCED),
            aliasEditorErrorMessage = "",
            autoSaveStatus = ActivityHierarchySaveStatus.SAVED,
            txtReloadRequestVersion = uiState.txtReloadRequestVersion + 1,
            statusText = "save canonical TOML through core migration"
        )
    }


}
