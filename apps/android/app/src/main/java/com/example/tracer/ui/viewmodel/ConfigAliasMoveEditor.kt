package com.example.tracer

import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch

internal class ConfigAliasMoveEditor(
    private val configGateway: ConfigGateway,
    private val activityHierarchyGateway: ActivityHierarchyGateway,
    activityHierarchyMigrationGateway: ActivityHierarchyMigrationGateway,
    quickActivitiesPreferenceGateway: QuickActivitiesPreferenceGateway,
    private val configFileEditor: ConfigFileEditor,
    private val scope: CoroutineScope,
    private val readState: () -> ConfigUiState,
    private val writeState: (ConfigUiState) -> Unit
) {
    private var uiState: ConfigUiState
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
    private val activityHierarchyMoveCoordinator = ActivityHierarchyMoveCoordinator(
        configGateway = configGateway,
        activityHierarchyGateway = activityHierarchyGateway
    )
    private val activityHierarchyEditStateCoordinator = ActivityHierarchyEditStateCoordinator(
        editCoordinator = activityHierarchyEditCoordinator,
        configGateway = configGateway,
        activityHierarchyGateway = activityHierarchyGateway
    )

    fun previewAliasEntryMove(entryId: String, targetGroupId: String) {
        viewModelScope.launch {
            uiState = applyMovePreviewOutcome(
                state = uiState,
                outcome = activityHierarchyMoveCoordinator.previewEntryMove(
                    state = uiState,
                    entryId = entryId,
                    targetGroupId = targetGroupId
                )
            )
        }
    }

    fun prepareAliasEntryMove(entryId: String) {
        val sourceDocument = uiState.aliasDocumentDraft ?: return
        val sourcePath = uiState.selectedFilePath
        if (sourcePath.isBlank() || sourceDocument.findAliasEntry(entryId) == null) return
        val sourceParentPath = sourceDocument.canonicalTargetPathForEntry(entryId)
            ?.split('.')
            ?.dropLast(1)
            .orEmpty()
        prepareAliasMoveDestinations(
            sourcePath,
            sourceParentPath,
            excludeDescendants = false,
            onlyCurrentDocument = false
        )
    }

    fun prepareAliasGroupMove(groupId: String) {
        val sourceDocument = uiState.aliasDocumentDraft ?: return
        val sourcePath = uiState.selectedFilePath
        val groupPath = sourceDocument.canonicalTargetPathForGroup(groupId)
            ?.split('.')
            ?.filter(String::isNotEmpty)
            ?: return
        if (sourcePath.isBlank() || sourceDocument.findAliasGroup(groupId) == null) return
        prepareAliasMoveDestinations(
            sourcePath,
            groupPath,
            excludeDescendants = true,
            onlyCurrentDocument = false
        )
    }

    private fun prepareAliasMoveDestinations(
        sourcePath: String,
        excludedGroupPath: List<String>,
        excludeDescendants: Boolean,
        onlyCurrentDocument: Boolean
    ) {
        uiState = uiState.copy(
            aliasEntryMoveDestinations = emptyList(),
            aliasEntryMoveDestinationsLoading = true,
            aliasEditorErrorMessage = ""
        )
        viewModelScope.launch {
            val outcome = activityHierarchyMoveCoordinator.prepareDestinations(
                state = uiState,
                sourcePath = sourcePath,
                excludedGroupPath = excludedGroupPath,
                excludeDescendants = excludeDescendants,
                onlyCurrentDocument = onlyCurrentDocument
            )
            uiState = when (outcome) {
                is ActivityHierarchyMoveDestinationsOutcome.Ready -> uiState.copy(
                    aliasEntryMoveDestinations = outcome.documents,
                    aliasEntryMoveDestinationsLoading = false
                )
                is ActivityHierarchyMoveDestinationsOutcome.Failed -> uiState.copy(
                    aliasEntryMoveDestinationsLoading = false,
                    aliasEditorErrorMessage = outcome.message
                )
            }
        }
    }

    fun previewAliasEntryMove(entryId: String, target: AliasEntryMoveTarget) {
        val sourceDocument = uiState.aliasDocumentDraft ?: return
        val sourcePath = uiState.selectedFilePath
        if (target.sourceName == sourcePath) {
            val targetGroupId = target.groupId ?: return
            previewAliasEntryMove(entryId, targetGroupId)
            return
        }
        viewModelScope.launch {
            uiState = applyMovePreviewOutcome(
                state = uiState,
                outcome = activityHierarchyMoveCoordinator.previewEntryMove(
                    state = uiState,
                    entryId = entryId,
                    target = target
                )
            )
        }
    }

    fun previewAliasGroupMove(groupId: String, target: AliasEntryMoveTarget) {
        if (uiState.aliasDocumentDraft?.findAliasGroup(groupId) == null) return
        viewModelScope.launch {
            uiState = applyMovePreviewOutcome(
                state = uiState,
                outcome = activityHierarchyMoveCoordinator.previewGroupMove(
                    state = uiState,
                    groupId = groupId,
                    target = target
                )
            )
        }
    }

    private fun applyMovePreviewOutcome(
        state: ConfigUiState,
        outcome: ActivityHierarchyMovePreviewOutcome
    ): ConfigUiState = when (outcome) {
        is ActivityHierarchyMovePreviewOutcome.Ready -> state.copy(
            aliasEntryMovePlan = outcome.plan,
            aliasEditorErrorMessage = ""
        )
        is ActivityHierarchyMovePreviewOutcome.Failed -> state.copy(
            aliasEntryMovePlan = null,
            aliasEditorErrorMessage = outcome.message,
            statusText = outcome.message
        )
    }

    fun discardAliasEntryMovePlan() {
        if (uiState.aliasEntryMovePlan == null) {
            return
        }
        uiState = uiState.copy(
            aliasEntryMovePlan = null,
            aliasEntryMoveDestinations = emptyList(),
            aliasEntryMoveDestinationsLoading = false,
            aliasEditorErrorMessage = "",
            statusText = "move plan discarded"
        )
    }

    fun confirmAliasEntryMovePlan() {
        val plan = uiState.aliasEntryMovePlan ?: return
        if (plan.updatedDocuments.isNotEmpty()) {
            confirmCrossDocumentAliasEntryMovePlan(plan)
            return
        }
        val document = uiState.aliasDocumentDraft ?: return
        val sourcePath = if (plan.nodeKind == AliasMoveNodeKind.GROUP) {
            document.canonicalTargetPathForGroup(plan.entryId)
        } else {
            document.canonicalTargetPathForEntry(plan.entryId)
        } ?: return
        val destinationPath = document.canonicalTargetPathForGroup(plan.targetGroupId) ?: return
        applyCoreActivityHierarchyOperation(
            ActivityHierarchyOperation(
                kind = if (plan.nodeKind == AliasMoveNodeKind.GROUP) {
                    ActivityHierarchyOperationKind.MOVE_GROUP
                } else {
                    ActivityHierarchyOperationKind.MOVE_LEAF
                },
                targetPath = sourcePath,
                destinationPath = destinationPath
            )
        )
    }

    private fun confirmCrossDocumentAliasEntryMovePlan(plan: AliasEntryMovePlan) {
        val sourceDocument = plan.updatedDocuments.firstOrNull { it.sourceName == plan.sourceFilePath }
            ?: return
        viewModelScope.launch {
            uiState = uiState.copy(autoSaveStatus = ConfigAutoSaveStatus.SAVING)
            val outcome = activityHierarchyEditCoordinator.persistMigration(
                ActivityHierarchyMigrationRequest(
                    configRelativePath = plan.sourceFilePath,
                    updatedTomlContent = sourceDocument.updatedTomlContent,
                    replacementPlan = plan.replacementPlan,
                    updatedDocuments = plan.updatedDocuments.map {
                        ActivityHierarchyDocumentInput(it.sourceName, it.updatedTomlContent)
                    }
                )
            )
            if (outcome is ActivityHierarchyMigrationOutcome.Invalid) {
                uiState = uiState.copy(
                    aliasEditorErrorMessage = outcome.message,
                    autoSaveStatus = ConfigAutoSaveStatus.FAILED
                )
                return@launch
            }
            val refreshed = configGateway.listConfigTomlFiles()
            val refreshedState = if (refreshed.ok) {
                uiState.copy(
                    aliasFiles = refreshed.aliasFiles,
                    chartFiles = refreshed.chartFiles,
                    metaFiles = refreshed.metaFiles,
                    reportFiles = refreshed.reportFiles,
                    aliasEntryMovePlan = null,
                    aliasEntryMoveDestinations = emptyList(),
                    aliasEntryMoveDestinationsLoading = false,
                    autoSaveStatus = ConfigAutoSaveStatus.SAVED,
                    txtReloadRequestVersion = uiState.txtReloadRequestVersion + 1,
                    statusText = moveCompletionStatus(plan)
                )
            } else {
                uiState.copy(
                    aliasEntryMovePlan = null,
                    aliasEntryMoveDestinations = emptyList(),
                    aliasEntryMoveDestinationsLoading = false,
                    autoSaveStatus = ConfigAutoSaveStatus.SAVED,
                    txtReloadRequestVersion = uiState.txtReloadRequestVersion + 1,
                    statusText = moveCompletionStatus(plan)
                )
            }
            uiState = configFileEditor.open(
                state = refreshedState,
                path = plan.sourceFilePath,
                statusText = refreshedState.statusText
            )
        }
    }

    private fun moveCompletionStatus(plan: AliasEntryMovePlan): String =
        if (plan.nodeKind == AliasMoveNodeKind.GROUP) {
            "moved group subtree across TOML and rebuilt database"
        } else {
            "moved activity name across TOML and rebuilt database"
        }

    private fun applyCoreActivityHierarchyOperation(operation: ActivityHierarchyOperation) {
        viewModelScope.launch {
            uiState = uiState.copy(autoSaveStatus = ConfigAutoSaveStatus.SAVING)
            val outcome = activityHierarchyEditStateCoordinator.apply(
                state = uiState,
                operation = operation
            )
            uiState = when (outcome) {
                is ActivityHierarchyEditStateOutcome.Failed -> uiState.copy(
                    aliasEditorErrorMessage = outcome.message,
                    autoSaveStatus = ConfigAutoSaveStatus.FAILED
                )
                is ActivityHierarchyEditStateOutcome.Applied -> outcome.state
            }
        }
    }


}
