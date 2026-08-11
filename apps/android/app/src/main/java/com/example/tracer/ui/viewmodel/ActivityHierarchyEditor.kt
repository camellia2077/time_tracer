package com.example.tracer

import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch

internal class ActivityHierarchyEditor(
    private val configGateway: ConfigGateway,
    private val activityHierarchyGateway: ActivityHierarchyGateway,
    activityHierarchyMigrationGateway: ActivityHierarchyMigrationGateway,
    quickActivitiesPreferenceGateway: QuickActivitiesPreferenceGateway,
    private val configFileEditor: ActivityHierarchyFileEditor,
    private val scope: CoroutineScope,
    private val readState: () -> ActivityHierarchyEditorState,
    private val writeState: (ActivityHierarchyEditorState) -> Unit
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
    private val activityHierarchyEditStateCoordinator = ActivityHierarchyEditStateCoordinator(
        editCoordinator = activityHierarchyEditCoordinator,
        configGateway = configGateway,
        activityHierarchyGateway = activityHierarchyGateway
    )

    fun onAliasAdvancedTomlChange(value: String) {
        val selectedFile = uiState.selectedFilePath
        val nextAdvancedDrafts = uiState.aliasAdvancedDraftsByFile.toMutableMap()
        val nextModeByFile = uiState.aliasEditorModeByFile.toMutableMap()
        if (selectedFile.isNotBlank()) {
            if (value == uiState.selectedFileContent) {
                nextAdvancedDrafts.remove(selectedFile)
            } else {
                nextAdvancedDrafts[selectedFile] = value
            }
            nextModeByFile[selectedFile] = AliasEditorMode.ADVANCED
        }
        uiState = uiState.copy(
            aliasAdvancedTomlDraft = value,
            aliasAdvancedDraftsByFile = nextAdvancedDrafts,
            aliasEditorModeByFile = nextModeByFile
        )
    }

    fun selectAliasEditorMode(mode: AliasEditorMode) {
        if (!isAliasConfigFilePath(uiState.selectedFilePath) || uiState.aliasEditorMode == mode) {
            return
        }
        if (mode == AliasEditorMode.ADVANCED) {
            uiState = cacheAliasAdvancedMode(switchAliasEditorToAdvanced(uiState))
            return
        }
        val rawToml = uiState.aliasAdvancedTomlDraft
        viewModelScope.launch {
            val result = activityHierarchyGateway.describeActivityHierarchy(rawToml)
            val document = result.hierarchy?.toActivityHierarchyDocument()
            if (!result.ok || document == null) {
                val message = result.message.ifBlank { "Activity hierarchy validation failed." }
                uiState = uiState.copy(aliasEditorErrorMessage = message, statusText = message)
                return@launch
            }
            uiState = cacheAliasStructuredMode(uiState.copy(
                aliasEditorMode = AliasEditorMode.STRUCTURED,
                aliasDocumentDraft = document,
                aliasParentOptions = normalizeAliasParentOptions(
                    uiState.aliasParentOptions + document.parent
                ),
                aliasEditorErrorMessage = ""
            ))
        }
    }

    fun updateAliasParent(value: String) {
        val normalizedValue = value.trim()
        if (normalizedValue.isEmpty() || !isAliasConfigFilePath(uiState.selectedFilePath)) {
            return
        }
        viewModelScope.launch {
            val currentFilePath = uiState.selectedFilePath
            val targetFilePath = resolveAliasFilePathForParent(
                configGateway = configGateway,
                activityHierarchyGateway = activityHierarchyGateway,
                aliasFiles = uiState.aliasFiles,
                currentFilePath = currentFilePath,
                currentActivityHierarchyDocument = uiState.aliasDocumentDraft,
                parent = normalizedValue
            )
            if (targetFilePath != null && targetFilePath != currentFilePath) {
                uiState = configFileEditor.open(
                    state = uiState,
                    path = targetFilePath,
                    statusText = "open toml -> $targetFilePath"
                )
                return@launch
            }

            applyCoreActivityHierarchyOperation(
                ActivityHierarchyOperation(
                    kind = ActivityHierarchyOperationKind.RENAME_PARENT,
                    oldParent = uiState.aliasDocumentDraft?.parent.orEmpty(),
                    newName = normalizedValue
                )
            )
        }
    }

    fun renameAliasCategory(newName: String) {
        val selectedFile = uiState.selectedFilePath
        val document = uiState.aliasDocumentDraft
        if (!isAliasConfigFilePath(selectedFile) || document == null) return
        val normalizedName = newName.trim()
        val newPath = newAliasTomlPath(normalizedName)
        if (newPath == null) {
            uiState = uiState.copy(aliasEditorErrorMessage = "Category name is invalid.")
            return
        }
        if (newPath == selectedFile) {
            uiState = uiState.copy(aliasEditorErrorMessage = "Category name is unchanged.")
            return
        }
        if (uiState.aliasFiles.any { it.relativePath == newPath }) {
            uiState = uiState.copy(aliasEditorErrorMessage = "An activity category with this name already exists.")
            return
        }
        val content = uiState.aliasAdvancedTomlDraft.ifBlank { uiState.selectedFileContent }
        viewModelScope.launch {
            uiState = uiState.copy(autoSaveStatus = ActivityHierarchySaveStatus.SAVING)
            val outcome = activityHierarchyEditCoordinator.apply(
                ActivityHierarchyEditRequest(
                    configRelativePath = selectedFile,
                    tomlContent = content,
                    operation = ActivityHierarchyOperation(
                    kind = ActivityHierarchyOperationKind.RENAME_PARENT,
                    oldParent = document.parent,
                    newName = normalizedName
                    ),
                    configFileRename = ActivityHierarchyDocumentRename(
                        oldSourceName = selectedFile,
                        newSourceName = newPath
                    )
                )
            )
            if (outcome is ActivityHierarchyEditOutcome.Failed) {
                uiState = uiState.copy(
                    aliasEditorErrorMessage = outcome.message,
                    autoSaveStatus = ActivityHierarchySaveStatus.FAILED
                )
                return@launch
            }

            val applied = outcome as ActivityHierarchyEditOutcome.Applied
            val committedPath = applied.migration.updatedConfigRelativePath.ifBlank { newPath }
            val refreshedAliasFiles = uiState.aliasFiles
                .filterNot { it.relativePath == selectedFile }
                .plus(ConfigTomlFileEntry(committedPath, committedPath.removePrefix("user/activity_hierarchy/")))
                .sortedBy { it.relativePath }
            val nextState = uiState.copy(
                aliasFiles = refreshedAliasFiles,
                selectedFilePath = committedPath,
                selectedFileDisplayName = committedPath.removePrefix("user/activity_hierarchy/"),
                selectedFileContent = applied.renderedToml,
                aliasDocumentDraft = applied.document,
                aliasBaselineDocument = applied.document,
                aliasAdvancedTomlDraft = applied.renderedToml,
                aliasStructuredDraftsByFile = uiState.aliasStructuredDraftsByFile
                    .minus(selectedFile)
                    .minus(committedPath),
                aliasAdvancedDraftsByFile = uiState.aliasAdvancedDraftsByFile
                    .minus(selectedFile)
                    .minus(committedPath),
                aliasEditorModeByFile = uiState.aliasEditorModeByFile
                    .minus(selectedFile)
                    .minus(committedPath)
                    .plus(committedPath to AliasEditorMode.STRUCTURED),
                aliasEntryMovePlan = null,
                aliasEditorErrorMessage = "",
                statusText = "Activity category renamed successfully.",
                txtReloadRequestVersion = uiState.txtReloadRequestVersion + 1,
                autoSaveStatus = ActivityHierarchySaveStatus.SAVED
            )
            uiState = configFileEditor.open(
                state = nextState,
                path = committedPath,
                statusText = nextState.statusText
            )
        }
    }

    fun addAliasGroup(parentGroupId: String?, name: String) {
        val document = uiState.aliasDocumentDraft ?: return
        val normalizedName = name.trim()
        if (normalizedName.isEmpty()) {
            uiState = uiState.copy(aliasEditorErrorMessage = "Alias group name must not be empty.")
            return
        }
        applyCoreActivityHierarchyOperation(
            ActivityHierarchyOperation(
                kind = ActivityHierarchyOperationKind.ADD_GROUP,
                targetPath = parentGroupId?.let(document::canonicalTargetPathForGroup) ?: "root",
                canonicalKey = normalizedName
            )
        )
    }

    fun deleteAliasGroup(groupId: String) {
        val document = uiState.aliasDocumentDraft ?: return
        val path = document.canonicalTargetPathForGroup(groupId) ?: return
        applyCoreActivityHierarchyOperation(ActivityHierarchyOperation(ActivityHierarchyOperationKind.DELETE_GROUP, targetPath = path))
    }

    fun addAliasEntry(parentGroupId: String?, canonicalLeaf: String, aliases: List<String>) {
        val document = uiState.aliasDocumentDraft ?: return
        val normalizedCanonicalLeaf = canonicalLeaf.trim()
        val normalizedAliases = aliases.map(String::trim).filter(String::isNotEmpty)
        if (normalizedCanonicalLeaf.isEmpty() || normalizedAliases.isEmpty()) {
            uiState = uiState.copy(
                aliasEditorErrorMessage = "Canonical and at least one alias must not be empty."
            )
            return
        }
        applyCoreActivityHierarchyOperation(ActivityHierarchyOperation(
            kind = ActivityHierarchyOperationKind.ADD_LEAF,
            targetPath = parentGroupId?.let(document::canonicalTargetPathForGroup) ?: "root",
            canonicalKey = normalizedCanonicalLeaf,
            aliases = normalizedAliases
        ))
    }

    fun updateAliasEntry(entryId: String, canonicalLeaf: String, aliases: List<String>) {
        val document = uiState.aliasDocumentDraft ?: return
        val normalizedCanonicalLeaf = canonicalLeaf.trim()
        val normalizedAliases = aliases.map(String::trim).filter(String::isNotEmpty)
        if (normalizedCanonicalLeaf.isEmpty() || normalizedAliases.isEmpty()) {
            uiState = uiState.copy(
                aliasEditorErrorMessage = "Canonical and at least one alias must not be empty."
            )
            return
        }
        val canonicalTargetPath = document.canonicalTargetPathForEntry(entryId)
        val oldLeaf = document.findAliasEntry(entryId)?.canonicalLeaf
        if (canonicalTargetPath == null || oldLeaf == null) return
        applyCoreActivityHierarchyOperation(ActivityHierarchyOperation(
            kind = if (oldLeaf == normalizedCanonicalLeaf) {
                ActivityHierarchyOperationKind.SET_LEAF_ALIASES
            } else {
                ActivityHierarchyOperationKind.RENAME_LEAF_CANONICAL
            },
            targetPath = canonicalTargetPath,
            newName = normalizedCanonicalLeaf,
            aliases = normalizedAliases
        ))
    }

    fun deleteAliasEntry(entryId: String) {
        val document = uiState.aliasDocumentDraft ?: return
        val path = document.canonicalTargetPathForEntry(entryId) ?: return
        applyCoreActivityHierarchyOperation(ActivityHierarchyOperation(ActivityHierarchyOperationKind.DELETE_LEAF, targetPath = path))
    }

    private fun applyCoreActivityHierarchyOperation(operation: ActivityHierarchyOperation) {
        viewModelScope.launch {
            uiState = uiState.copy(autoSaveStatus = ActivityHierarchySaveStatus.SAVING)
            val outcome = activityHierarchyEditStateCoordinator.apply(
                state = uiState,
                operation = operation
            )
            uiState = when (outcome) {
                is ActivityHierarchyEditStateOutcome.Failed -> uiState.copy(
                    aliasEditorErrorMessage = outcome.message,
                    autoSaveStatus = ActivityHierarchySaveStatus.FAILED
                )
                is ActivityHierarchyEditStateOutcome.Applied -> outcome.state
            }
        }
    }

    fun renameAliasGroup(groupId: String, name: String) {
        val document = uiState.aliasDocumentDraft ?: return
        val normalizedName = name.trim()
        if (normalizedName.isEmpty()) return
        val targetPath = document.canonicalTargetPathForGroup(groupId)
        if (targetPath == null) return
        applyCoreActivityHierarchyOperation(ActivityHierarchyOperation(
            kind = ActivityHierarchyOperationKind.RENAME_GROUP_CANONICAL,
            targetPath = targetPath,
            newName = normalizedName
        ))
    }

    fun addAliasToEntry(entryId: String, alias: String) {
        val document = uiState.aliasDocumentDraft ?: return
        val normalizedAlias = alias.trim()
        if (normalizedAlias.isEmpty()) return
        val path = document.canonicalTargetPathForEntry(entryId) ?: return
        val parentPath = path.substringBeforeLast('.', missingDelimiterValue = "root")
        val key = path.substringAfterLast('.')
        applyCoreActivityHierarchyOperation(ActivityHierarchyOperation(
            kind = ActivityHierarchyOperationKind.APPEND_LEAF_ALIAS,
            targetPath = parentPath,
            canonicalKey = key, aliases = listOf(normalizedAlias)
        ))
    }

    fun renameAliasOnEntry(entryId: String, oldAlias: String, newAlias: String) {
        val document = uiState.aliasDocumentDraft ?: return
        val normalizedAlias = newAlias.trim()
        if (normalizedAlias.isEmpty() || normalizedAlias == oldAlias) return
        val entry = document.findAliasEntry(entryId) ?: return
        val path = document.canonicalTargetPathForEntry(entryId) ?: return
        applyCoreActivityHierarchyOperation(ActivityHierarchyOperation(
            kind = ActivityHierarchyOperationKind.SET_LEAF_ALIASES,
            targetPath = path,
            aliases = entry.aliases.map { if (it == oldAlias) normalizedAlias else it }
        ))
    }

    fun deleteAliasFromEntry(entryId: String, alias: String) {
        val document = uiState.aliasDocumentDraft ?: return
        val entry = document.findAliasEntry(entryId) ?: return
        if (entry.aliases.size <= 1) {
            uiState = uiState.copy(aliasEditorErrorMessage = "A canonical must keep at least one alias.")
            return
        }
        val path = document.canonicalTargetPathForEntry(entryId) ?: return
        applyCoreActivityHierarchyOperation(ActivityHierarchyOperation(
            kind = ActivityHierarchyOperationKind.SET_LEAF_ALIASES,
            targetPath = path,
            aliases = entry.aliases - alias
        ))
    }

    fun promoteAliasEntryToGroup(entryId: String) {
        val document = uiState.aliasDocumentDraft ?: return
        val path = document.canonicalTargetPathForEntry(entryId) ?: return
        applyCoreActivityHierarchyOperation(ActivityHierarchyOperation(ActivityHierarchyOperationKind.PROMOTE_LEAF, targetPath = path))
    }

    fun mergeAliasEntry(sourceEntryId: String, destinationEntryId: String) {
        val document = uiState.aliasDocumentDraft ?: return
        val sourcePath = document.canonicalTargetPathForEntry(sourceEntryId) ?: return
        val destinationPath = document.canonicalTargetPathForEntry(destinationEntryId) ?: return
        applyCoreActivityHierarchyOperation(
            ActivityHierarchyOperation(
                kind = ActivityHierarchyOperationKind.MERGE_LEAF_CANONICAL,
                targetPath = sourcePath,
                destinationPath = destinationPath
            )
        )
    }

    fun renameGroupAlias(groupId: String, oldAlias: String, newAlias: String) {
        val trimmedAlias = newAlias.trim()
        val document = uiState.aliasDocumentDraft ?: return
        if (trimmedAlias.isEmpty() || trimmedAlias == oldAlias) return
        val path = document.canonicalTargetPathForGroup(groupId) ?: return
        applyCoreActivityHierarchyOperation(ActivityHierarchyOperation(
            kind = ActivityHierarchyOperationKind.RENAME_GROUP_ALIAS,
            targetPath = path,
            oldAlias = oldAlias, newName = trimmedAlias
        ))
    }

    fun addGroupAlias(groupId: String, alias: String) {
        val trimmedAlias = alias.trim()
        val document = uiState.aliasDocumentDraft ?: return
        if (trimmedAlias.isEmpty()) return
        val path = document.canonicalTargetPathForGroup(groupId) ?: return
        applyCoreActivityHierarchyOperation(ActivityHierarchyOperation(
            kind = ActivityHierarchyOperationKind.APPEND_GROUP_ALIAS,
            targetPath = path,
            aliases = listOf(trimmedAlias)
        ))
    }

    fun updateGroupAliases(groupId: String, aliases: List<String>) {
        val document = uiState.aliasDocumentDraft ?: return
        val normalizedAliases = aliases.map(String::trim)
        if (normalizedAliases.any(String::isEmpty) || normalizedAliases.distinct().size != normalizedAliases.size) {
            uiState = uiState.copy(aliasEditorErrorMessage = "Group aliases must be non-empty and unique.")
            return
        }
        val path = document.canonicalTargetPathForGroup(groupId) ?: return
        applyCoreActivityHierarchyOperation(ActivityHierarchyOperation(
            kind = ActivityHierarchyOperationKind.SET_GROUP_ALIASES,
            targetPath = path,
            aliases = normalizedAliases
        ))
    }


    private fun cacheAliasAdvancedMode(state: ActivityHierarchyEditorState): ActivityHierarchyEditorState {
        val selectedFile = state.selectedFilePath
        if (selectedFile.isBlank()) {
            return state
        }
        val nextAdvancedDrafts = state.aliasAdvancedDraftsByFile.toMutableMap()
        if (state.aliasAdvancedTomlDraft == state.selectedFileContent) {
            nextAdvancedDrafts.remove(selectedFile)
        } else {
            nextAdvancedDrafts[selectedFile] = state.aliasAdvancedTomlDraft
        }
        return state.copy(
            aliasAdvancedDraftsByFile = nextAdvancedDrafts,
            aliasEditorModeByFile = state.aliasEditorModeByFile + (selectedFile to AliasEditorMode.ADVANCED)
        )
    }

    private fun cacheAliasStructuredMode(state: ActivityHierarchyEditorState): ActivityHierarchyEditorState {
        val selectedFile = state.selectedFilePath
        val document = state.aliasDocumentDraft
        if (selectedFile.isBlank() || document == null) {
            return state
        }
        val nextStructuredDrafts = state.aliasStructuredDraftsByFile.toMutableMap()
        if (state.aliasAdvancedTomlDraft == state.selectedFileContent) {
            nextStructuredDrafts.remove(selectedFile)
        } else {
            nextStructuredDrafts[selectedFile] = document
        }
        return state.copy(
            aliasStructuredDraftsByFile = nextStructuredDrafts,
            aliasEditorModeByFile = state.aliasEditorModeByFile + (selectedFile to AliasEditorMode.STRUCTURED)
        )
    }

}

