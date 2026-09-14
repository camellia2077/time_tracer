package com.example.tracer

internal sealed interface ActivityHierarchyEditStateOutcome {
    data class Applied(val state: ActivityHierarchyEditorState) : ActivityHierarchyEditStateOutcome
    data class Failed(val message: String) : ActivityHierarchyEditStateOutcome
}

/** Applies one Core hierarchy operation and translates its result into UI state. */
internal class ActivityHierarchyEditStateCoordinator(
    private val editCoordinator: ActivityHierarchyEditCoordinator,
    private val configGateway: ConfigGateway,
    private val activityHierarchyGateway: ActivityHierarchyGateway
) {
    suspend fun apply(
        state: ActivityHierarchyEditorState,
        operation: ActivityHierarchyOperation
    ): ActivityHierarchyEditStateOutcome {
        val selectedFile = state.selectedFilePath
        val outcome = editCoordinator.apply(
            ActivityHierarchyEditRequest(
                configRelativePath = selectedFile,
                tomlContent = state.selectedFileContent,
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
                selectedFileContent = migratedToml,
                aliasEntryMovePlan = null,
                aliasParentOptions = parentOptions,
                aliasEditorErrorMessage = "",
                txtReloadRequestVersion = state.txtReloadRequestVersion + 1,
                autoSaveStatus = ActivityHierarchySaveStatus.SAVED
            )
        )
    }
}
