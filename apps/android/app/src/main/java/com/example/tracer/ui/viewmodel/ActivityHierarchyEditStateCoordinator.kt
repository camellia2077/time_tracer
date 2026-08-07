package com.example.tracer

internal sealed interface ActivityHierarchyEditStateOutcome {
    data class Applied(val state: ConfigUiState) : ActivityHierarchyEditStateOutcome
    data class Failed(val message: String) : ActivityHierarchyEditStateOutcome
}

/** Applies one Core hierarchy operation and translates its result into UI state. */
internal class ActivityHierarchyEditStateCoordinator(
    private val editCoordinator: ActivityHierarchyEditCoordinator,
    private val configGateway: ConfigGateway,
    private val activityHierarchyGateway: ActivityHierarchyGateway
) {
    suspend fun apply(
        state: ConfigUiState,
        operation: ActivityHierarchyOperation
    ): ActivityHierarchyEditStateOutcome {
        val selectedFile = state.selectedFilePath
        val outcome = editCoordinator.apply(
            ActivityHierarchyEditRequest(
                configRelativePath = selectedFile,
                tomlContent = state.aliasAdvancedTomlDraft.ifBlank { state.selectedFileContent },
                operation = operation
            )
        )
        if (outcome is ActivityHierarchyEditOutcome.Failed) {
            return ActivityHierarchyEditStateOutcome.Failed(outcome.message)
        }

        val applied = outcome as ActivityHierarchyEditOutcome.Applied
        val migratedToml = applied.renderedToml
        val parentOptions = resolveAliasParentOptions(
            configGateway = configGateway,
            activityHierarchyGateway = activityHierarchyGateway,
            aliasFiles = state.aliasFiles,
            selectedFilePath = selectedFile,
            selectedFileContent = migratedToml
        )
        return ActivityHierarchyEditStateOutcome.Applied(
            state.copy(
                aliasDocumentDraft = applied.document,
                aliasBaselineDocument = applied.document,
                selectedFileContent = migratedToml,
                aliasAdvancedTomlDraft = migratedToml,
                aliasStructuredDraftsByFile = cacheStructuredDraft(
                    state,
                    selectedFile,
                    applied.document,
                    migratedToml
                ),
                aliasEditorModeByFile = cacheAliasMode(
                    state,
                    selectedFile,
                    AliasEditorMode.STRUCTURED
                ),
                aliasEntryMovePlan = null,
                aliasParentOptions = parentOptions,
                aliasEditorErrorMessage = "",
                txtReloadRequestVersion = state.txtReloadRequestVersion + 1,
                autoSaveStatus = ConfigAutoSaveStatus.SAVED
            )
        )
    }

    private fun cacheStructuredDraft(
        state: ConfigUiState,
        filePath: String,
        document: ActivityHierarchyDocument,
        tomlContent: String
    ): Map<String, ActivityHierarchyDocument> {
        if (filePath.isBlank()) return state.aliasStructuredDraftsByFile
        val nextDrafts = state.aliasStructuredDraftsByFile.toMutableMap()
        if (tomlContent == state.selectedFileContent) {
            nextDrafts.remove(filePath)
        } else {
            nextDrafts[filePath] = document
        }
        return nextDrafts
    }

    private fun cacheAliasMode(
        state: ConfigUiState,
        filePath: String,
        mode: AliasEditorMode
    ): Map<String, AliasEditorMode> {
        if (filePath.isBlank()) return state.aliasEditorModeByFile
        return state.aliasEditorModeByFile + (filePath to mode)
    }
}
