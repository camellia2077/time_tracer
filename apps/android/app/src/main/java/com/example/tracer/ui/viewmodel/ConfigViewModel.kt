package com.example.tracer

import android.util.Log
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch

internal enum class ConfigCategory {
    // Canonical files live under `user/activity_hierarchy/*.toml`.
    ALIAS,
    // Charts = `charts/*.toml`
    CHARTS,
    // Meta = `config.toml` plus `meta/*.toml`
    META,
    // Reports = `reports/**/*.toml`
    REPORTS
}

internal enum class ConfigAutoSaveStatus {
    IDLE,
    SAVING,
    SAVED,
    FAILED
}

internal data class ConfigUiState(
    val selectedCategory: ConfigCategory = ConfigCategory.ALIAS,
    val aliasFiles: List<ConfigTomlFileEntry> = emptyList(),
    val chartFiles: List<ConfigTomlFileEntry> = emptyList(),
    val metaFiles: List<ConfigTomlFileEntry> = emptyList(),
    val reportFiles: List<ConfigTomlFileEntry> = emptyList(),
    val selectedFilePath: String = "",
    val selectedFileDisplayName: String = "",
    val selectedFileContent: String = "",
    val editableContent: String = "",
    // Keep unsaved drafts in memory per file so switching tabs/files behaves like a normal text
    // editor: the user sees their in-session edits again without silently writing them to disk.
    val plainTomlDraftsByFile: Map<String, String> = emptyMap(),
    val aliasEditorMode: AliasEditorMode = AliasEditorMode.STRUCTURED,
    val aliasDocumentDraft: AliasTomlDocument? = null,
    val aliasBaselineDocument: AliasTomlDocument? = null,
    val aliasParentOptions: List<String> = emptyList(),
    val aliasAdvancedTomlDraft: String = "",
    val aliasStructuredDraftsByFile: Map<String, AliasTomlDocument> = emptyMap(),
    val aliasAdvancedDraftsByFile: Map<String, String> = emptyMap(),
    val aliasEditorModeByFile: Map<String, AliasEditorMode> = emptyMap(),
    val aliasEntryMovePlan: AliasEntryMovePlan? = null,
    val aliasEntryMoveDestinations: List<AliasEntryMoveDestinationDocument> = emptyList(),
    val aliasEntryMoveDestinationsLoading: Boolean = false,
    val aliasEditorErrorMessage: String = "",
    val txtReloadRequestVersion: Long = 0L,
    val autoSaveStatus: ConfigAutoSaveStatus = ConfigAutoSaveStatus.IDLE,
    val statusText: String = "Preparing config..."
)

private object UnavailableActivityHierarchyGateway : ActivityHierarchyGateway {
    private const val MESSAGE = "Activity hierarchy runtime is unavailable."

    override suspend fun describeActivityHierarchy(
        tomlContent: String
    ): ActivityHierarchyDescribeResult = ActivityHierarchyDescribeResult(false, message = MESSAGE)

    override suspend fun validateActivityHierarchyDocuments(
        documents: List<ActivityHierarchyDocumentInput>
    ): ActivityHierarchyValidationResult = ActivityHierarchyValidationResult(false, MESSAGE)

    override suspend fun applyActivityHierarchyOperation(
        tomlContent: String,
        operation: ActivityHierarchyOperation
    ): ActivityHierarchyOperationResult = ActivityHierarchyOperationResult(
        ok = false,
        updatedTomlContent = tomlContent,
        message = MESSAGE
    )

    override suspend fun moveActivityHierarchyNodeBetweenDocuments(
        documents: List<ActivityHierarchyDocumentInput>,
        sourceName: String,
        destinationName: String,
        operation: ActivityHierarchyOperation
    ): ActivityHierarchyCrossDocumentOperationResult =
        ActivityHierarchyCrossDocumentOperationResult(false, message = MESSAGE)

    override suspend fun rewriteActivityHierarchyDocument(
        originalTomlContent: String,
        updatedTomlContent: String
    ): ActivityHierarchyOperationResult = ActivityHierarchyOperationResult(
        ok = false,
        updatedTomlContent = updatedTomlContent,
        message = MESSAGE
    )
}

private object UnavailableActivityHierarchyMigrationGateway : ActivityHierarchyMigrationGateway {
    override suspend fun applyActivityHierarchyMigration(
        request: ActivityHierarchyMigrationRequest
    ): ActivityHierarchyMigrationResult = ActivityHierarchyMigrationResult(
        ok = false,
        message = "Activity hierarchy migration runtime is unavailable."
    )
}

internal class ConfigViewModel(
    private val configGateway: ConfigGateway,
    private val txtStorageGateway: TxtStorageGateway,
    private val quickActivitiesPreferenceGateway: QuickActivitiesPreferenceGateway,
    private val activityHierarchyGateway: ActivityHierarchyGateway,
    private val activityHierarchyMigrationGateway: ActivityHierarchyMigrationGateway
) : ViewModel() {
    private val activityHierarchyMigrationUseCase =
        ActivityHierarchyMigrationUseCase(
            gateway = activityHierarchyMigrationGateway,
            quickActivitiesPreferenceGateway = quickActivitiesPreferenceGateway
        )
    private val activityHierarchyEditCoordinator = ActivityHierarchyEditCoordinator(
        hierarchyGateway = activityHierarchyGateway,
        migrationUseCase = activityHierarchyMigrationUseCase
    )
    private val configFileEditor = ConfigFileEditor(
        configGateway = configGateway,
        activityHierarchyGateway = activityHierarchyGateway
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

    /** Compatibility constructor for local tests that use one runtime fake. */
    constructor(
        configGateway: ConfigGateway,
        txtStorageGateway: TxtStorageGateway,
        quickActivitiesPreferenceGateway: QuickActivitiesPreferenceGateway
    ) : this(
        configGateway = configGateway,
        txtStorageGateway = txtStorageGateway,
        quickActivitiesPreferenceGateway = quickActivitiesPreferenceGateway,
        activityHierarchyGateway = configGateway as? ActivityHierarchyGateway
            ?: UnavailableActivityHierarchyGateway,
        activityHierarchyMigrationGateway = configGateway as? ActivityHierarchyMigrationGateway
            ?: UnavailableActivityHierarchyMigrationGateway
    )
    companion object {
        private const val ALIAS_RENAME_LOG_TAG = "AliasRename"
    }

    var uiState by mutableStateOf(ConfigUiState())
        private set

    init {
        refreshConfigFiles()
    }

    fun refreshConfigFiles(showStatus: Boolean = true) {
        viewModelScope.launch {
            uiState = configFileEditor.refresh(uiState, showStatus)
        }
    }

    fun selectCategory(category: ConfigCategory) {
        viewModelScope.launch {
            uiState = configFileEditor.selectCategory(uiState, category)
        }
    }

    fun openFile(path: String) {
        val trimmedPath = path.trim()
        if (trimmedPath.isEmpty()) {
            return
        }
        viewModelScope.launch {
            uiState = configFileEditor.open(uiState, trimmedPath)
        }
    }

    fun onEditableContentChange(value: String) {
        uiState = configFileEditor.updateEditableContent(uiState, value)
    }

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
            val document = result.hierarchy?.toActivityAliasDocument()
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
                currentAliasDocument = uiState.aliasDocumentDraft,
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
            uiState = uiState.copy(autoSaveStatus = ConfigAutoSaveStatus.SAVING)
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
                    autoSaveStatus = ConfigAutoSaveStatus.FAILED
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
                autoSaveStatus = ConfigAutoSaveStatus.SAVED
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
            onlyCurrentDocument = true
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

    fun setStatusText(message: String) {
        uiState = uiState.copy(statusText = message)
    }

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
            refreshConfigFiles(showStatus = false)
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
                autoSaveStatus = ConfigAutoSaveStatus.FAILED
            )
            return
        }
        uiState = uiState.copy(autoSaveStatus = ConfigAutoSaveStatus.SAVING)
        viewModelScope.launch {
            if (isAliasConfigFilePath(selectedFile)) {
                saveAliasFile(selectedFile)
            } else {
                uiState = configFileEditor.savePlainTomlFile(uiState, selectedFile)
            }
        }
    }

    fun discardUnsavedDraft() {
        if (isAliasConfigFilePath(uiState.selectedFilePath)) {
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
            return
        }
        uiState = configFileEditor.discardPlainTomlDraft(uiState)
    }

    private suspend fun saveAliasFile(selectedFile: String) {
        uiState = when (uiState.aliasEditorMode) {
            AliasEditorMode.STRUCTURED -> saveStructuredAliasFile(selectedFile)
            AliasEditorMode.ADVANCED -> saveAdvancedAliasFile(selectedFile)
        }
    }

    private suspend fun saveStructuredAliasFile(selectedFile: String): ConfigUiState {
        val document = uiState.aliasDocumentDraft
            ?: return uiState.copy(
                statusText = "Alias editor is unavailable for this file.",
                autoSaveStatus = ConfigAutoSaveStatus.FAILED
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
            autoSaveStatus = ConfigAutoSaveStatus.SAVED,
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
        val document = result.hierarchy?.toActivityAliasDocument()
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

    private suspend fun saveAdvancedAliasFile(selectedFile: String): ConfigUiState {
        val rewritten = activityHierarchyGateway.rewriteActivityHierarchyDocument(
            originalTomlContent = uiState.selectedFileContent,
            updatedTomlContent = uiState.aliasAdvancedTomlDraft
        )
        val document = rewritten.hierarchy?.toActivityAliasDocument()
            ?: return uiState.copy(
                aliasEditorErrorMessage = rewritten.message,
                autoSaveStatus = ConfigAutoSaveStatus.FAILED,
                statusText = rewritten.message
            )
        if (!rewritten.ok) {
            return uiState.copy(
                aliasEditorErrorMessage = rewritten.message,
                autoSaveStatus = ConfigAutoSaveStatus.FAILED,
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
                autoSaveStatus = ConfigAutoSaveStatus.FAILED,
                statusText = duplicateMessage
            )
        }
        uiState = uiState.copy(autoSaveStatus = ConfigAutoSaveStatus.SAVING)
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
                autoSaveStatus = ConfigAutoSaveStatus.FAILED
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
            autoSaveStatus = ConfigAutoSaveStatus.SAVED,
            txtReloadRequestVersion = uiState.txtReloadRequestVersion + 1,
            statusText = "save canonical TOML through core migration"
        )
    }

    private fun cacheAliasAdvancedMode(state: ConfigUiState): ConfigUiState {
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

    private fun cacheAliasStructuredMode(state: ConfigUiState): ConfigUiState {
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

private fun logInfo(tag: String, message: String) {
    runCatching { Log.i(tag, message) }
}

internal class ConfigViewModelFactory(
    private val configGateway: ConfigGateway,
    private val activityHierarchyGateway: ActivityHierarchyGateway,
    private val activityHierarchyMigrationGateway: ActivityHierarchyMigrationGateway,
    private val txtStorageGateway: TxtStorageGateway,
    private val quickActivitiesPreferenceGateway: QuickActivitiesPreferenceGateway
) : ViewModelProvider.Factory {
    override fun <T : ViewModel> create(modelClass: Class<T>): T {
        if (modelClass.isAssignableFrom(ConfigViewModel::class.java)) {
            @Suppress("UNCHECKED_CAST")
            return ConfigViewModel(
                configGateway = configGateway,
                activityHierarchyGateway = activityHierarchyGateway,
                activityHierarchyMigrationGateway = activityHierarchyMigrationGateway,
                txtStorageGateway = txtStorageGateway,
                quickActivitiesPreferenceGateway = quickActivitiesPreferenceGateway
            ) as T
        }
        throw IllegalArgumentException("Unknown ViewModel class: ${modelClass.name}")
    }
}
